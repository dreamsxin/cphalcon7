/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Martin Schröder <m.schroeder2007@gmail.com>                 |
  +----------------------------------------------------------------------+
*/

#include "async/core.h"
#include "async/async_helper.h"
#include "async/async_socket.h"

#ifdef PHP_WIN32
#include <svcguid.h>
#endif

#define ASYNC_DNS_STATUS_RESOLVED 0
#define ASYNC_DNS_STATUS_NO_RESULT 1
#define ASYNC_DNS_STATUS_FAILURE 2

ASYNC_API zend_class_entry *async_dns_config_ce;
ASYNC_API zend_class_entry *async_dns_query_ce;
ASYNC_API zend_class_entry *async_dns_resolver_ce;

static zend_object_handlers async_dns_query_handlers;

static zend_string *str_host;
static zend_string *str_type;
static zend_string *str_search;
static zend_string *str_class;
static zend_string *str_in;
static zend_string *str_ttl;
static zend_string *str_ip;
static zend_string *str_ipv6;
static zend_string *str_target;
static zend_string *str_pri;

static zend_function *orig_gethostbyname;
static zif_handler orig_gethostbyname_handler;

static zend_function *orig_gethostbynamel;
static zif_handler orig_gethostbynamel_handler;

static zend_function *orig_dns_check_record;
static zif_handler orig_dns_check_record_handler;

static zend_function *orig_dns_get_mx;
static zif_handler orig_dns_get_mx_handler;

static zend_function *orig_dns_get_record;
static zif_handler orig_dns_get_record_handler;

static zend_function *orig_checkdnsrr;
static zif_handler orig_checkdnsrr_handler;

static zend_function *orig_getmxrr;
static zif_handler orig_getmxrr_handler;

#define ASYNC_DNS_A 1
#define ASYNC_DNS_AAAA 28
#define ASYNC_DNS_A6 38
#define ASYNC_DNS_ANY 255
#define ASYNC_DNS_CAA 257
#define ASYNC_DNS_CNAME 5
#define ASYNC_DNS_HINFO 13
#define ASYNC_DNS_MX 15
#define ASYNC_DNS_NAPTR 35
#define ASYNC_DNS_NS 2
#define ASYNC_DNS_PTR 12
#define ASYNC_DNS_SOA 6
#define ASYNC_DNS_SRV 33
#define ASYNC_DNS_TXT 16

#define PHP_DNS_A      0x00000001
#define PHP_DNS_NS     0x00000002
#define PHP_DNS_CNAME  0x00000010
#define PHP_DNS_SOA    0x00000020
#define PHP_DNS_PTR    0x00000800
#define PHP_DNS_HINFO  0x00001000
#define PHP_DNS_CAA    0x00002000
#define PHP_DNS_MX     0x00004000
#define PHP_DNS_TXT    0x00008000
#define PHP_DNS_A6     0x01000000
#define PHP_DNS_SRV    0x02000000
#define PHP_DNS_NAPTR  0x04000000
#define PHP_DNS_AAAA   0x08000000

#define PHP_DNS_ANY 0x10000000
#define PHP_DNS_ALL (PHP_DNS_A|PHP_DNS_NS|PHP_DNS_CNAME|PHP_DNS_SOA|PHP_DNS_PTR|PHP_DNS_HINFO|PHP_DNS_CAA|PHP_DNS_MX|PHP_DNS_TXT|PHP_DNS_A6|PHP_DNS_SRV|PHP_DNS_NAPTR|PHP_DNS_AAAA)

#define ASYNC_DNS_QUERY_FLAG_ANY 1

typedef struct _async_dns_query {
	uint8_t flags;

	zval types;
	zval records;

	zend_object std;
} async_dns_query;

#define ASYNC_DNS_QUERY_CONST(name, value) \
	zend_declare_class_constant_long(async_dns_query_ce, name, sizeof(name)-1, (zend_long)value);

static async_dns_query *create_dns_query(char *host, zend_long types);

static zend_always_inline async_dns_query *async_dns_query_obj(zend_object *object)
{
	return (async_dns_query *)((char *) object - XtOffsetOf(async_dns_query, std));
}

static zend_always_inline uint32_t async_dns_query_prop_offset(zend_string *name)
{
	return zend_get_property_info(async_dns_query_ce, name, 1)->offset;
}

static zend_always_inline char *dns_type_name(zend_long type)
{
	switch (type) {
	case ASYNC_DNS_A: return "A";
	case ASYNC_DNS_NS: return "NS";
	case ASYNC_DNS_CNAME: return "CNAME";
	case ASYNC_DNS_SOA: return "SOA";
	case ASYNC_DNS_PTR: return "PTR";
	case ASYNC_DNS_HINFO: return "HINFO";
	case ASYNC_DNS_CAA: return "CAA";
	case ASYNC_DNS_MX: return "MX";
	case ASYNC_DNS_TXT: return "TXT";
	case ASYNC_DNS_A6: return "A6";
	case ASYNC_DNS_SRV: return "SRV";
	case ASYNC_DNS_NAPTR: return "NAPTR";
	case ASYNC_DNS_AAAA: return "AAAA";
	case ASYNC_DNS_ANY: return "ANY";
	}

	return "UNKNOWN";
}

static int query_dns(zval *resolver, async_dns_query *query)
{
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;

	zval retval;
	zval args[1];

	int code;

	ZVAL_OBJ(&args[0], &query->std);

	ZVAL_UNDEF(&fci.function_name);
	fci.size = sizeof(fci);
	fci.object = NULL;
	fci.retval = &retval;
	fci.param_count = 1;
	fci.params = args;

#if PHP_VERSION_ID < 80000
	fci.no_separation = 1;
#endif

	fcc = empty_fcall_info_cache;
	fcc.object = Z_OBJ_P(resolver);
	fcc.called_scope = Z_OBJCE_P(resolver);
	fcc.function_handler = zend_hash_find_ptr(&fcc.called_scope->function_table, str_search);

	code = zend_call_function(&fci, &fcc);

	zval_ptr_dtor(&retval);

	return code;
}

ASYNC_CALLBACK dns_gethostbyname_cb(uv_getaddrinfo_t *req, int status, struct addrinfo *addr)
{
	async_uv_op *op;
	
	op = (async_uv_op *) req->data;
	
	ZEND_ASSERT(op != NULL);
	
	if (UNEXPECTED(status == UV_ECANCELED)) {
		ASYNC_FREE_OP(op);
	} else {	
		op->code = status;
		
		ASYNC_FINISH_OP(op);
	}
}

static int dns_gethostbyname(uv_getaddrinfo_t *req, char *name, int proto)
{
	async_uv_op *op;
	
	struct addrinfo hints;

	int async;
	int code;
	
	async = ASYNC_G(cli) && !(async_task_scheduler_get()->flags & (ASYNC_TASK_SCHEDULER_FLAG_DISPOSED | ASYNC_TASK_SCHEDULER_FLAG_ERROR));
	op = NULL;

	if (async) {
		ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_uv_op));
		
		req->data = op;
	}
	
	memset(&hints, 0, sizeof(struct addrinfo));
	
	if (proto > 0) {
		hints.ai_protocol = proto;
	}

	code = uv_getaddrinfo(async_loop_get(), req, async ? dns_gethostbyname_cb : NULL, name, NULL, &hints);
	
	if (UNEXPECTED(code < 0)) {
		if (async) {
			ASYNC_FREE_OP(op);
		}
	
		uv_freeaddrinfo(req->addrinfo);
		
		return code;
	}
	
	if (async) {
		if (async_await_op((async_op *) op) == FAILURE) {
			ASYNC_FORWARD_OP_ERROR(op);
			
			if (0 == uv_cancel((uv_req_t *) req)) {
				ASYNC_FREE_OP(op);
			} else {
				op->base.status = ASYNC_STATUS_FAILED;
			}

			return FAILURE;
		}
		
		code = op->code;

		ASYNC_FREE_OP(op);
		
		if (UNEXPECTED(code < 0)) {
			uv_freeaddrinfo(req->addrinfo);
			
			return code;
		}
	}
	
	return 0;
}

static int lookup_ip_using_resolver(char *name, php_sockaddr_storage *dest, int types)
{
	async_dns_query *query;

	zval *resolver;
	zval *record;

	int code;

	resolver = async_get_component(async_task_scheduler_get(), async_dns_resolver_ce->name, EG(current_execute_data));

	if (UNEXPECTED(EG(exception))) {
		return ASYNC_DNS_STATUS_FAILURE;
	}

	query = create_dns_query(name, types);

	if (UNEXPECTED(FAILURE == query_dns(resolver, query) || EG(exception))) {
		ASYNC_DELREF(&query->std);
		return ASYNC_DNS_STATUS_FAILURE;
	}

	code = ASYNC_DNS_STATUS_NO_RESULT;

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(query->records), record) {
		if (Z_LVAL_P(zend_hash_find(Z_ARRVAL_P(record), str_type)) == ASYNC_DNS_A) {
			uv_ip4_addr(Z_STRVAL_P(zend_hash_find(Z_ARRVAL_P(record), str_ip)), 0, (struct sockaddr_in *) dest);
			code = ASYNC_DNS_STATUS_RESOLVED;

			break;
		}
	} ZEND_HASH_FOREACH_END();

#ifdef HAVE_IPV6
	if (code == ASYNC_DNS_STATUS_NO_RESULT && types & PHP_DNS_AAAA) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(query->records), record) {
			if (Z_LVAL_P(zend_hash_find(Z_ARRVAL_P(record), str_type)) == ASYNC_DNS_AAAA) {
				uv_ip6_addr(Z_STRVAL_P(zend_hash_find(Z_ARRVAL_P(record), str_ipv6)), 0, (struct sockaddr_in6 *) dest);
				code = ASYNC_DNS_STATUS_RESOLVED;

				break;
			}
		} ZEND_HASH_FOREACH_END();
	}
#endif

	ASYNC_DELREF(&query->std);

	return code;
}

ASYNC_API int async_dns_lookup_ip(char *name, php_sockaddr_storage *dest, int proto)
{
	uv_getaddrinfo_t req;
	struct addrinfo *info;
	int code;

	memset(dest, 0, sizeof(php_sockaddr_storage));
	
	if (SUCCESS == async_socket_parse_ip((const char *) name, 0, dest)) {
		return SUCCESS;
	}
	
	if (zend_hash_find_ptr(ASYNC_G(factories), async_dns_resolver_ce->name)) {
		switch (lookup_ip_using_resolver(name, dest, PHP_DNS_A | PHP_DNS_AAAA)) {
		case ASYNC_DNS_STATUS_FAILURE:
			return FAILURE;
		case ASYNC_DNS_STATUS_RESOLVED:
			return SUCCESS;
		}
	}

	code = dns_gethostbyname(&req, name, proto);
	
	if (UNEXPECTED(code != 0)) {
		return code;
	}
	
	info = req.addrinfo;
	
	while (info != NULL) {
		if (info->ai_family == AF_INET && (info->ai_protocol == proto || proto == 0)) {
			memcpy(dest, info->ai_addr, sizeof(struct sockaddr_in));
			
			uv_freeaddrinfo(req.addrinfo);
			
			return SUCCESS;
		}
		
		info = info->ai_next;
	}

#ifdef HAVE_IPV6
	info = req.addrinfo;

	while (info != NULL) {
		if (info->ai_family == AF_INET6 && (info->ai_protocol == proto || proto == 0)) {
			memcpy(dest, info->ai_addr, sizeof(struct sockaddr_in6));

			uv_freeaddrinfo(req.addrinfo);
			
			return SUCCESS;
		}

		info = info->ai_next;
	}
#endif
	
	uv_freeaddrinfo(req.addrinfo);
	
	return UV_EAI_NODATA;
}

static PHP_FUNCTION(async_gethostbyname)
{
	char *name;
	size_t len;
	
	uv_getaddrinfo_t req;
	struct sockaddr_in dest;
	struct addrinfo *info;
	char ip[64];
	int code;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STRING(name, len)
	ZEND_PARSE_PARAMETERS_END();

	if (UNEXPECTED(len > MAXFQDNLEN)) {
		php_error_docref(NULL, E_WARNING, "Host name is too long, the limit is %d characters", MAXFQDNLEN);
		RETURN_STRINGL(name, len);
	}

	if (SUCCESS == async_socket_parse_ipv4((const char *) name, 0, &dest)) {
		uv_ip4_name(&dest, ip, sizeof(ip));

		RETURN_STRING(ip);
	}

	if (zend_hash_find_ptr(ASYNC_G(factories), async_dns_resolver_ce->name)) {
		switch (lookup_ip_using_resolver(name, (php_sockaddr_storage *) &dest, PHP_DNS_A)) {
		case ASYNC_DNS_STATUS_FAILURE:
			return;
		case ASYNC_DNS_STATUS_RESOLVED:
			uv_ip4_name(&dest, ip, sizeof(ip));
			RETURN_STRING(ip);
		}
	}

	code = dns_gethostbyname(&req, name, 0);
	
	if (UNEXPECTED(code != 0)) {
		RETURN_STRINGL(name, len);
	}
	
	info = req.addrinfo;
	
	while (info != NULL) {
		if (info->ai_family == AF_INET) {
			uv_ip4_name((struct sockaddr_in *) info->ai_addr, ip, info->ai_addrlen);
			
			RETVAL_STRING(ip);
			break;
		}
		
		info = info->ai_next;
	}
	
	uv_freeaddrinfo(req.addrinfo);
}

static PHP_FUNCTION(async_gethostbynamel)
{
	async_dns_query *query;

	char *name;
	size_t len;
	
	zend_string *k;
	zval *resolver;
	zval *record;
	zval tmp;
	
	uv_getaddrinfo_t req;
	struct sockaddr_in dest;
	struct addrinfo *info;
	char ip[64];
	int code;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STRING(name, len)
	ZEND_PARSE_PARAMETERS_END();

	if (UNEXPECTED(len > MAXFQDNLEN)) {
		php_error_docref(NULL, E_WARNING, "Host name is too long, the limit is %d characters", MAXFQDNLEN);
		RETURN_STRINGL(name, len);
	}
	
	if (SUCCESS == async_socket_parse_ipv4((const char *) name, 0, &dest)) {
		uv_ip4_name(&dest, ip, sizeof(ip));
		
		array_init(return_value);
		add_next_index_str(return_value, zend_string_init(ip, strlen(ip), 0));
		
		return;
	}
	
	if (zend_hash_find_ptr(ASYNC_G(factories), async_dns_resolver_ce->name)) {
		resolver = async_get_component(async_task_scheduler_get(), async_dns_resolver_ce->name, execute_data);

		ASYNC_RETURN_ON_ERROR();

		query = create_dns_query(name, PHP_DNS_A);

		if (UNEXPECTED(FAILURE == query_dns(resolver, query) || EG(exception))) {
			ASYNC_ENSURE_ERROR("Failed to call DNS resolver");
			ASYNC_DELREF(&query->std);
			return;
		}

		if (zend_hash_num_elements(Z_ARRVAL(query->records))) {
			array_init(&tmp);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(query->records), record) {
				k = Z_STR_P(zend_hash_find(Z_ARRVAL_P(record), str_ip));

				add_assoc_bool_ex(&tmp, ZSTR_VAL(k), ZSTR_LEN(k), 1);
			} ZEND_HASH_FOREACH_END();

			ASYNC_DELREF(&query->std);

			array_init(return_value);

			ZEND_HASH_FOREACH_STR_KEY(Z_ARRVAL_P(&tmp), k) {
				add_next_index_str(return_value, zend_string_copy(k));
			} ZEND_HASH_FOREACH_END();

			zval_ptr_dtor(&tmp);

			return;
		}

		ASYNC_DELREF(&query->std);
	}

	code = dns_gethostbyname(&req, name, 0);
	
	if (UNEXPECTED(code != 0)) {
		RETURN_FALSE;
	}

	info = req.addrinfo;
	
	array_init(&tmp);
	
	while (info != NULL) {
		if (info->ai_family == AF_INET) {
			uv_ip4_name((struct sockaddr_in *) info->ai_addr, ip, info->ai_addrlen);
			
			add_assoc_bool_ex(&tmp, ip, strlen(ip), 1);
		}
		
		info = info->ai_next;
	}
	
	uv_freeaddrinfo(req.addrinfo);
			
	array_init(return_value);
	
	ZEND_HASH_FOREACH_STR_KEY(Z_ARRVAL_P(&tmp), k) {
		add_next_index_str(return_value, zend_string_copy(k));
	} ZEND_HASH_FOREACH_END();
	
	zval_ptr_dtor(&tmp);
}

static PHP_FUNCTION(async_dns_get_record)
{
	async_dns_query *query;

	zend_string *host;
	zend_long type;
	zend_bool raw;

	zval *auth;
	zval *add;

	zval *resolver;
	zval *record;
	zval str;

	if (!zend_hash_find_ptr(ASYNC_G(factories), async_dns_resolver_ce->name)) {
		orig_dns_get_record_handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		return;
	}

	type = PHP_DNS_ANY;
	auth = NULL;
	add = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 5)
		Z_PARAM_STR(host)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(type)
		Z_PARAM_ZVAL(auth)
		Z_PARAM_ZVAL(add)
		Z_PARAM_BOOL(raw)
	ZEND_PARSE_PARAMETERS_END();

	if (type & PHP_DNS_ANY) {
		type = PHP_DNS_ANY;
	}

	ASYNC_CHECK_ERROR(type & ~PHP_DNS_ALL && type != PHP_DNS_ANY, "Invalid DNS record type requested");

	resolver = async_get_component(async_task_scheduler_get(), async_dns_resolver_ce->name, execute_data);

	ASYNC_RETURN_ON_ERROR();

	query = create_dns_query(ZSTR_VAL(host), type);

	if (UNEXPECTED(FAILURE == query_dns(resolver, query) || EG(exception))) {
		ASYNC_ENSURE_ERROR("Failed to call DNS resolver");
		ASYNC_DELREF(&query->std);
		return;
	}

	if (zend_hash_num_elements(Z_ARRVAL(query->records))) {
		RETVAL_ZVAL(&query->records, 1, 0);
		ASYNC_DELREF(&query->std);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(return_value), record) {
			ZVAL_STRING(&str, dns_type_name(Z_LVAL_P(zend_hash_find(Z_ARRVAL_P(record), str_type))));

			zend_hash_update(Z_ARRVAL_P(record), str_type, &str);
		} ZEND_HASH_FOREACH_END();

		return;
	}

	ASYNC_DELREF(&query->std);

	orig_dns_get_record_handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static void handle_get_mx(INTERNAL_FUNCTION_PARAMETERS)
{
	async_dns_query *query;

	zend_string *host;
	zval *mx;
	zval *weight;
	zval tmp;

	zval *resolver;
	zval *record;

	if (!zend_hash_find_ptr(ASYNC_G(factories), async_dns_resolver_ce->name)) {
		orig_dns_get_mx_handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		return;
	}

	weight = NULL;

	ZEND_PARSE_PARAMETERS_START(2, 3)
		Z_PARAM_STR(host)
		Z_PARAM_ZVAL(mx)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(weight)
	ZEND_PARSE_PARAMETERS_END();

	zval_ptr_dtor(mx);
	array_init(mx);

	if (weight) {
		zval_ptr_dtor(weight);
		array_init(weight);
	}

	resolver = async_get_component(async_task_scheduler_get(), async_dns_resolver_ce->name, execute_data);

	ASYNC_RETURN_ON_ERROR();

	query = create_dns_query(ZSTR_VAL(host), PHP_DNS_MX);

	if (UNEXPECTED(FAILURE == query_dns(resolver, query) || EG(exception))) {
		ASYNC_ENSURE_ERROR("Failed to call DNS resolver");
		ASYNC_DELREF(&query->std);
		return;
	}

	if (zend_hash_num_elements(Z_ARRVAL(query->records))) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(query->records), record) {
			ZVAL_COPY(&tmp, zend_hash_find(Z_ARRVAL_P(record), str_target));
			zend_hash_next_index_insert(Z_ARRVAL_P(mx), &tmp);

			if (weight) {
				ZVAL_COPY(&tmp, zend_hash_find(Z_ARRVAL_P(record), str_pri));
				zend_hash_next_index_insert(Z_ARRVAL_P(weight), &tmp);
			}
		} ZEND_HASH_FOREACH_END();

		ASYNC_DELREF(&query->std);

		RETURN_TRUE;
	}

	ASYNC_DELREF(&query->std);

	orig_dns_get_mx_handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static PHP_FUNCTION(async_dns_get_mx)
{
	handle_get_mx(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static PHP_FUNCTION(async_getmxrr)
{
	handle_get_mx(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static void handle_check_record(INTERNAL_FUNCTION_PARAMETERS)
{
	async_dns_query *query;

	zend_string *host;
	zend_string *name;

	zend_long type;
	zval *resolver;

	if (!zend_hash_find_ptr(ASYNC_G(factories), async_dns_resolver_ce->name)) {
		orig_dns_check_record_handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		return;
	}

	name = NULL;
	type = PHP_DNS_MX;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_STR(host)
		Z_PARAM_OPTIONAL
		Z_PARAM_STR(name)
	ZEND_PARSE_PARAMETERS_END();

	if (!ZSTR_LEN(host)) {
		php_error_docref(NULL, E_WARNING, "Host cannot be empty");
		RETURN_FALSE;
	}

	if (name) {
		if (!strcasecmp("A", ZSTR_VAL(name))) type = PHP_DNS_A;
		else if (!strcasecmp("NS", ZSTR_VAL(name))) type = PHP_DNS_NS;
		else if (!strcasecmp("MX", ZSTR_VAL(name))) type = PHP_DNS_MX;
		else if (!strcasecmp("PTR", ZSTR_VAL(name))) type = PHP_DNS_PTR;
		else if (!strcasecmp("ANY", ZSTR_VAL(name))) type = PHP_DNS_ANY;
		else if (!strcasecmp("SOA", ZSTR_VAL(name))) type = PHP_DNS_SOA;
		else if (!strcasecmp("CAA", ZSTR_VAL(name))) type = PHP_DNS_CAA;
		else if (!strcasecmp("TXT", ZSTR_VAL(name))) type = PHP_DNS_TXT;
		else if (!strcasecmp("CNAME", ZSTR_VAL(name))) type = PHP_DNS_CNAME;
		else if (!strcasecmp("AAAA", ZSTR_VAL(name))) type = PHP_DNS_AAAA;
		else if (!strcasecmp("SRV", ZSTR_VAL(name))) type = PHP_DNS_SRV;
		else if (!strcasecmp("NAPTR", ZSTR_VAL(name))) type = PHP_DNS_NAPTR;
		else if (!strcasecmp("A6", ZSTR_VAL(name))) type = PHP_DNS_AAAA;
		else {
			php_error_docref(NULL, E_WARNING, "Type '%s' not supported", ZSTR_VAL(name));
			RETURN_FALSE;
		}
	}

	resolver = async_get_component(async_task_scheduler_get(), async_dns_resolver_ce->name, execute_data);

	ASYNC_RETURN_ON_ERROR();

	query = create_dns_query(ZSTR_VAL(host), type);

	if (UNEXPECTED(FAILURE == query_dns(resolver, query) || EG(exception))) {
		ASYNC_ENSURE_ERROR("Failed to call DNS resolver");
		ASYNC_DELREF(&query->std);
		return;
	}

	if (zend_hash_num_elements(Z_ARRVAL(query->records))) {
		ASYNC_DELREF(&query->std);
		RETURN_TRUE;
	}

	ASYNC_DELREF(&query->std);

	orig_dns_check_record_handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static PHP_FUNCTION(async_dns_check_record)
{
	handle_check_record(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static PHP_FUNCTION(async_checkdnsrr)
{
	handle_check_record(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static zend_string *get_hosts_file()
{
	zend_string *str;

#ifdef PHP_WIN32
	TCHAR buf[128];
	UINT size = sizeof(buf);

	size = GetSystemWindowsDirectoryA(buf, size);

	if (UNEXPECTED(size == 0)) {
		return NULL;
	}

	ASYNC_STRF(str, "%.*s\\system32\\drivers\\etc\\hosts", size, buf);
#elif defined(__APPLE__)
	str = zend_string_init(ZEND_STRL("/private/etc/hosts"), 0);
#else
	str = zend_string_init(ZEND_STRL("/etc/hosts"), 0);
#endif

	return str;
}

static zend_always_inline int skip_line(const char a)
{
	switch (a) {
	case '#':
	case ';':
	case '\n':
	case '\r':
		return 1;
	}

	return 0;
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dns_config_get_hosts_file, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Config, getHostsFile)
{
	zend_string *str;

	ZEND_PARSE_PARAMETERS_NONE();

	str = get_hosts_file();
	ASYNC_CHECK_ERROR(!str, "Failed to determine system hosts file location");

	RETURN_STR(str);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dns_config_get_hosts, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Config, getHosts)
{
	php_stream *fp;

	zend_string *str;
	zval *entry;
	zval addr;
	zval tmp;

	const char *delim = " \t\n\r";
	char buf[1024];
	char *ptr;

	ZEND_PARSE_PARAMETERS_NONE();

	str = get_hosts_file();
	ASYNC_CHECK_ERROR(!str, "Failed to determine system hosts file location");

	fp = php_stream_open_wrapper(ZSTR_VAL(str), "rb", STREAM_DISABLE_OPEN_BASEDIR, NULL);

	if (UNEXPECTED(!fp)) {
		zend_throw_error(NULL, "Failed to open system hosts file: %s", ZSTR_VAL(str));
		zend_string_release(str);
		return;
	}

	zend_string_release(str);

	array_init(return_value);

	while (NULL != php_stream_gets(fp, buf, sizeof(buf))) {
		if (skip_line(*buf)) {
			continue;
		}

		ptr = strtok(buf, delim);

		if (ptr) {
			ZVAL_STRING(&addr, ptr);

			ptr = strtok(NULL, delim);

			while (ptr) {
				if (skip_line(*ptr)) {
					break;
				}

				str = zend_string_init(ptr, strlen(ptr), 0);
				entry = zend_hash_find(Z_ARRVAL_P(return_value), str);

				if (EXPECTED(!entry)) {
					array_init(&tmp);
					entry = zend_hash_add(Z_ARRVAL_P(return_value), str, &tmp);
				}

				zend_string_release(str);

				Z_ADDREF(addr);
				zend_hash_next_index_insert(Z_ARRVAL_P(entry), &addr);

				ptr = strtok(NULL, delim);
			}

			zval_ptr_dtor(&addr);
		}
	}

	php_stream_close(fp);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dns_config_get_resolve_conf, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(Config, getResolveConf)
{
	ZEND_PARSE_PARAMETERS_NONE();

#ifndef PHP_WIN32
	RETURN_STRING("/etc/resolv.conf");
#endif
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dns_config_get_nameservers, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Config, getNameservers)
{
	zval p;

	char *port;
	int tmp;

	ZEND_PARSE_PARAMETERS_NONE();

	array_init(return_value);

#ifdef PHP_WIN32
	int code;
	GUID guid = SVCID_NAMESERVER_UDP;

	WSAQUERYSET qs = { 0 };
	qs.dwSize = sizeof(WSAQUERYSET);
	qs.dwNameSpace = NS_DNS;
	qs.lpServiceClassId = &guid;

	HANDLE lookup;
	code = WSALookupServiceBegin(&qs, LUP_RETURN_NAME | LUP_RETURN_ADDR, &lookup);

	if (UNEXPECTED(code == SOCKET_ERROR)) {
		code = uv_translate_sys_error(WSAGetLastError());
		zend_throw_error(NULL, "Failed to read DNS server entries", uv_strerror(code));
		return;
	}

	DWORD size = 1024;
	unsigned char *buf;

	buf = emalloc(size);

	do {
		code = WSALookupServiceNext(lookup, LUP_RETURN_NAME | LUP_RETURN_ADDR, &size, (WSAQUERYSET *) buf);

		if (UNEXPECTED(code == SOCKET_ERROR)) {
			code = WSAGetLastError();

			if (code == WSAEFAULT) {
				buf = erealloc(buf, size);
				continue;
			}

			break;
		}

		WSAQUERYSET *set = (LPWSAQUERYSET) buf;
		CSADDR_INFO *info = set->lpcsaBuffer;
		DWORD i;

		char str[256];
		DWORD len = sizeof(str);

		for (i = 0; i < set->dwNumberOfCsAddrs; i++) {
			code = WSAAddressToString(info->RemoteAddr.lpSockaddr, info->RemoteAddr.iSockaddrLength, NULL, str, &len);

			if (EXPECTED(code != SOCKET_ERROR)) {
				port = strrchr(str, ':');

				if (EXPECTED(port)) {
					tmp = atoi(port + 1);

					ZVAL_LONG(&p, tmp);
					zend_hash_str_add(Z_ARRVAL_P(return_value), str, port - str, &p);
				}
			}
		}

		break;
	} while (1);

	WSALookupServiceEnd(lookup);
	efree(buf);
#else
	php_stream *fp;

	const char *delim = " \t\n\r";
	char buf[1024];
	char *ptr;

	fp = php_stream_open_wrapper("/etc/resolv.conf", "rb", STREAM_DISABLE_OPEN_BASEDIR, NULL);

	ASYNC_CHECK_ERROR(!fp, "Failed to open DNS config file: /etc/resolv.conf");

	while (NULL != php_stream_gets(fp, buf, sizeof(buf))) {
		if (skip_line(*buf)) {
			continue;
		}

		ptr = strtok(buf, delim);

		if (ptr) {
			if (strcmp(ptr, "nameserver") != 0) {
				continue;
			}

			ptr = strtok(NULL, delim);

			while (ptr) {
				if (skip_line(*ptr)) {
					break;
				}

				port = strrchr(ptr, ':');

				if (port) {
					tmp = atoi(port + 1);

					ZVAL_LONG(&p, tmp);
					zend_hash_str_add(Z_ARRVAL_P(return_value), ptr, port - ptr, &p);
				} else {
					ZVAL_LONG(&p, 53);
					zend_hash_str_add(Z_ARRVAL_P(return_value), ptr, strlen(ptr), &p);
				}

				ptr = strtok(NULL, delim);
			}
		}
	}

	php_stream_close(fp);
#endif
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(Config, async_dns_config_ce)
ASYNC_METHOD_NO_WAKEUP(Config, async_dns_config_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_dns_config_functions[] = {
	PHP_ME(Config, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(Config, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(Config, getHostsFile, arginfo_dns_config_get_hosts_file, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Config, getHosts, arginfo_dns_config_get_hosts, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Config, getResolveConf, arginfo_dns_config_get_resolve_conf, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Config, getNameservers, arginfo_dns_config_get_nameservers, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};


static zend_object *async_dns_query_object_create(zend_class_entry *ce)
{
	async_dns_query *query;

	query = ecalloc(1, sizeof(async_dns_query) + zend_object_properties_size(ce));

	zend_object_std_init(&query->std, ce);
	query->std.handlers = &async_dns_query_handlers;

	object_properties_init(&query->std, ce);

	array_init(&query->types);
	array_init(&query->records);

	return &query->std;
}

static async_dns_query *create_dns_query(char *host, zend_long types)
{
	async_dns_query *query;

	zval val;

	zend_long i;
	zend_long j;

	query = async_dns_query_obj(async_dns_query_object_create(async_dns_query_ce));

	ZVAL_TRUE(&val);

	for (i = 1; i <= PHP_DNS_ANY; i <<= 1) {
		if (types & i) {
			switch (i) {
			case PHP_DNS_A: j = ASYNC_DNS_A; break;
			case PHP_DNS_A6: j = ASYNC_DNS_A6; break;
			case PHP_DNS_AAAA: j = ASYNC_DNS_AAAA; break;
			case PHP_DNS_CAA: j = ASYNC_DNS_CAA; break;
			case PHP_DNS_CNAME: j = ASYNC_DNS_CNAME; break;
			case PHP_DNS_HINFO: j = ASYNC_DNS_HINFO; break;
			case PHP_DNS_MX: j = ASYNC_DNS_MX; break;
			case PHP_DNS_NAPTR: j = ASYNC_DNS_NAPTR; break;
			case PHP_DNS_NS: j = ASYNC_DNS_NS; break;
			case PHP_DNS_PTR: j = ASYNC_DNS_PTR; break;
			case PHP_DNS_SOA: j = ASYNC_DNS_SOA; break;
			case PHP_DNS_SRV: j = ASYNC_DNS_SRV; break;
			case PHP_DNS_TXT: j = ASYNC_DNS_TXT; break;
			case PHP_DNS_ANY:
				j = ASYNC_DNS_ANY;
				query->flags |= ASYNC_DNS_QUERY_FLAG_ANY;
				break;
			default:
				j = 0;
			}

			if (j) {
				zend_hash_index_add(Z_ARRVAL(query->types), j, &val);
			}
		}
	}

	ZVAL_STRING(OBJ_PROP(&query->std, async_dns_query_prop_offset(str_host)), host);

	return query;
}

static void async_dns_query_object_destroy(zend_object *object)
{
	async_dns_query *query;

	query = async_dns_query_obj(object);

	zval_ptr_dtor(&query->types);
	zval_ptr_dtor(&query->records);

	zend_object_std_dtor(&query->std);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_dns_query_ctor, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, types, IS_LONG, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Query, __construct)
{
	async_dns_query *query;

	zend_string *host;
	zend_ulong type;

	zval *types;
	zval val;

	uint32_t count;
	uint32_t i;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, -1)
		Z_PARAM_STR(host)
		Z_PARAM_VARIADIC('+', types, count)
	ZEND_PARSE_PARAMETERS_END();

	query = async_dns_query_obj(Z_OBJ_P(getThis()));

	ZVAL_STR_COPY(OBJ_PROP(&query->std, async_dns_query_prop_offset(str_host)), host);
	ZVAL_TRUE(&val);

	for (i = 0; i < count; i++) {
		type = (zend_ulong) Z_LVAL_P(&types[i]);

		if (type == ASYNC_DNS_ANY) {
			query->flags |= ASYNC_DNS_QUERY_FLAG_ANY;
		}

		zend_hash_index_add(Z_ARRVAL(query->types), type, &val);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dns_query_get_types, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Query, getTypes)
{
	async_dns_query *query;

	zval val;
	zend_ulong i;

	ZEND_PARSE_PARAMETERS_NONE();

	query = async_dns_query_obj(Z_OBJ_P(getThis()));

	array_init(return_value);

	ZEND_HASH_FOREACH_NUM_KEY(Z_ARRVAL(query->types), i) {
		ZVAL_LONG(&val, i);
		zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &val);
	} ZEND_HASH_FOREACH_END();
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dns_query_get_records, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Query, getRecords)
{
	async_dns_query *query;

	ZEND_PARSE_PARAMETERS_NONE();

	query = async_dns_query_obj(Z_OBJ_P(getThis()));

	RETURN_ZVAL(&query->records, 1, 0);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dns_query_add_record, 0, 3, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, ttl, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Query, addRecord)
{
	async_dns_query *query;

	zend_long type;
	zend_long ttl;

	HashTable *data;
	zend_string *k;

	zval record;
	zval tmp;
	zval *v;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 3, 3)
		Z_PARAM_LONG(type)
		Z_PARAM_LONG(ttl)
		Z_PARAM_ARRAY_HT(data)
	ZEND_PARSE_PARAMETERS_END();

	query = async_dns_query_obj(Z_OBJ_P(getThis()));

	if (!(query->flags & ASYNC_DNS_QUERY_FLAG_ANY)) {
		ASYNC_CHECK_ERROR(!zend_hash_index_find(Z_ARRVAL(query->types), (zend_ulong) type), "Unrequested DNS record type: %s (%ld)", dns_type_name(type), type);
	}

	switch (type) {
	case ASYNC_DNS_A:
		ASYNC_CHECK_ERROR(!zend_hash_find(data, str_ip), "DNS A record is missing data key \"%s\"", ZSTR_VAL(str_ip));
		break;
	case ASYNC_DNS_AAAA:
		ASYNC_CHECK_ERROR(!zend_hash_find(data, str_ipv6), "DNS AAAA record is missing data key \"%s\"", ZSTR_VAL(str_ipv6));
		break;
	}

	array_init(&record);

	ZVAL_COPY(&tmp, OBJ_PROP(&query->std, async_dns_query_prop_offset(str_host)));
	zend_hash_add(Z_ARRVAL(record), str_host, &tmp);

	ZVAL_STR_COPY(&tmp, str_in);
	zend_hash_add(Z_ARRVAL(record), str_class, &tmp);

	ZVAL_LONG(&tmp, ttl);
	zend_hash_add(Z_ARRVAL(record), str_ttl, &tmp);

	ZVAL_LONG(&tmp, type);
	zend_hash_add(Z_ARRVAL(record), str_type, &tmp);

	ZEND_HASH_FOREACH_STR_KEY_VAL_IND(data, k, v) {
		if (EXPECTED(!zend_hash_find(Z_ARRVAL(record), k))) {
			Z_TRY_ADDREF_P(v);
			zend_hash_add(Z_ARRVAL(record), k, v);
		}
	} ZEND_HASH_FOREACH_END();

	zend_hash_next_index_insert(Z_ARRVAL(query->records), &record);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_WAKEUP(Query, async_dns_query_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_dns_query_functions[] = {
	PHP_ME(Query, __construct, arginfo_dns_query_ctor, ZEND_ACC_PUBLIC)
	PHP_ME(Query, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(Query, getTypes, arginfo_dns_query_get_types, ZEND_ACC_PUBLIC)
	PHP_ME(Query, getRecords, arginfo_dns_query_get_records, ZEND_ACC_PUBLIC)
	PHP_ME(Query, addRecord, arginfo_dns_query_add_record, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dns_resolver_search, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, query, Phalcon\\Async\\DNS\\Query, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Resolver, search) { }

static const zend_function_entry async_dns_resolver_functions[] = {
	PHP_ME(Resolver, search, arginfo_dns_resolver_search, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_FE_END
};

void async_dns_ce_register()
{
	zend_class_entry ce;

#if PHP_VERSION_ID >= 70400
	zval tmp;
#endif

	str_host = zend_new_interned_string(zend_string_init(ZEND_STRL("host"), 1));
	str_type = zend_new_interned_string(zend_string_init(ZEND_STRL("type"), 1));
	str_search = zend_new_interned_string(zend_string_init(ZEND_STRL("search"), 1));
	str_class = zend_new_interned_string(zend_string_init(ZEND_STRL("class"), 1));
	str_in = zend_new_interned_string(zend_string_init(ZEND_STRL("IN"), 1));
	str_ttl = zend_new_interned_string(zend_string_init(ZEND_STRL("ttl"), 1));
	str_ip = zend_new_interned_string(zend_string_init(ZEND_STRL("ip"), 1));
	str_ipv6 = zend_new_interned_string(zend_string_init(ZEND_STRL("ipv6"), 1));
	str_target = zend_new_interned_string(zend_string_init(ZEND_STRL("target"), 1));
	str_pri = zend_new_interned_string(zend_string_init(ZEND_STRL("pri"), 1));

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\DNS", "Config", async_dns_config_functions);
	async_dns_query_ce = zend_register_internal_class(&ce);
	async_dns_query_ce->ce_flags |= ZEND_ACC_FINAL;

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\DNS", "Query", async_dns_query_functions);
	async_dns_query_ce = zend_register_internal_class(&ce);
	async_dns_query_ce->ce_flags |= ZEND_ACC_FINAL;
	async_dns_query_ce->create_object = async_dns_query_object_create;
	async_dns_query_ce->serialize = zend_class_serialize_deny;
	async_dns_query_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_dns_query_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_dns_query_handlers.offset = XtOffsetOf(async_dns_query, std);
	async_dns_query_handlers.free_obj = async_dns_query_object_destroy;
	async_dns_query_handlers.write_property = async_prop_write_handler_readonly;

	ASYNC_DNS_QUERY_CONST("A", ASYNC_DNS_A);
	ASYNC_DNS_QUERY_CONST("A6", ASYNC_DNS_A6);
	ASYNC_DNS_QUERY_CONST("AAAA", ASYNC_DNS_AAAA);
	ASYNC_DNS_QUERY_CONST("ANY", ASYNC_DNS_ANY);
	ASYNC_DNS_QUERY_CONST("CAA", ASYNC_DNS_CAA);
	ASYNC_DNS_QUERY_CONST("CNAME", ASYNC_DNS_CNAME);
	ASYNC_DNS_QUERY_CONST("HINFO", ASYNC_DNS_HINFO);
	ASYNC_DNS_QUERY_CONST("MX", ASYNC_DNS_MX);
	ASYNC_DNS_QUERY_CONST("NAPTR", ASYNC_DNS_NAPTR);
	ASYNC_DNS_QUERY_CONST("NS", ASYNC_DNS_NS);
	ASYNC_DNS_QUERY_CONST("PTR", ASYNC_DNS_PTR);
	ASYNC_DNS_QUERY_CONST("SOA", ASYNC_DNS_SOA);
	ASYNC_DNS_QUERY_CONST("SRV", ASYNC_DNS_SRV);
	ASYNC_DNS_QUERY_CONST("TXT", ASYNC_DNS_TXT);

#if PHP_VERSION_ID >= 80000
	ZVAL_STRING(&tmp, "");
	zend_declare_typed_property(async_dns_query_ce, str_host, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_STRING, 0, 0));
	zval_ptr_dtor(&tmp);
#elif PHP_VERSION_ID < 70400
	zend_declare_property_null(async_dns_query_ce, ZEND_STRL("host"), ZEND_ACC_PUBLIC);
#else
	ZVAL_STRING(&tmp, "");
	zend_declare_typed_property(async_dns_query_ce, str_host, &tmp, ZEND_ACC_PUBLIC, NULL, IS_STRING);
	zval_ptr_dtor(&tmp);
#endif

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\DNS", "Resolver", async_dns_resolver_functions);
	async_dns_resolver_ce = zend_register_internal_interface(&ce);
}

void async_dns_init()
{
	orig_gethostbyname = (zend_function *) zend_hash_str_find_ptr(EG(function_table), ZEND_STRL("gethostbyname"));
	orig_gethostbyname_handler = orig_gethostbyname->internal_function.handler;
	
	orig_gethostbyname->internal_function.handler = PHP_FN(async_gethostbyname);
	
	orig_gethostbynamel = (zend_function *) zend_hash_str_find_ptr(EG(function_table), ZEND_STRL("gethostbynamel"));
	orig_gethostbynamel_handler = orig_gethostbynamel->internal_function.handler;
	
	orig_gethostbynamel->internal_function.handler = PHP_FN(async_gethostbynamel);

	orig_dns_check_record = (zend_function *) zend_hash_str_find_ptr(EG(function_table), ZEND_STRL("dns_check_record"));
	orig_dns_check_record_handler = orig_dns_check_record->internal_function.handler;

	orig_dns_check_record->internal_function.handler = PHP_FN(async_dns_check_record);

	orig_dns_get_mx = (zend_function *) zend_hash_str_find_ptr(EG(function_table), ZEND_STRL("dns_get_mx"));
	orig_dns_get_mx_handler = orig_dns_get_mx->internal_function.handler;

	orig_dns_get_mx->internal_function.handler = PHP_FN(async_dns_get_mx);

	orig_dns_get_record = (zend_function *) zend_hash_str_find_ptr(EG(function_table), ZEND_STRL("dns_get_record"));
	orig_dns_get_record_handler = orig_dns_get_record->internal_function.handler;

	orig_dns_get_record->internal_function.handler = PHP_FN(async_dns_get_record);

	orig_checkdnsrr = (zend_function *) zend_hash_str_find_ptr(EG(function_table), ZEND_STRL("checkdnsrr"));
	orig_checkdnsrr_handler = orig_checkdnsrr->internal_function.handler;

	orig_checkdnsrr->internal_function.handler = PHP_FN(async_checkdnsrr);

	orig_getmxrr = (zend_function *) zend_hash_str_find_ptr(EG(function_table), ZEND_STRL("getmxrr"));
	orig_getmxrr_handler = orig_getmxrr->internal_function.handler;

	orig_getmxrr->internal_function.handler = PHP_FN(async_getmxrr);
}

void async_dns_shutdown()
{
	orig_gethostbyname->internal_function.handler = orig_gethostbyname_handler;
	orig_gethostbynamel->internal_function.handler = orig_gethostbynamel_handler;

	orig_dns_check_record->internal_function.handler = orig_dns_check_record_handler;
	orig_dns_get_mx->internal_function.handler = orig_dns_get_mx_handler;
	orig_dns_get_record->internal_function.handler = orig_dns_get_record_handler;

	orig_checkdnsrr->internal_function.handler = orig_checkdnsrr_handler;
	orig_getmxrr->internal_function.handler = orig_getmxrr_handler;
}

void async_dns_ce_unregister()
{
	zend_string_release(str_host);
	zend_string_release(str_type);
	zend_string_release(str_search);
	zend_string_release(str_class);
	zend_string_release(str_in);
	zend_string_release(str_ttl);
	zend_string_release(str_ip);
	zend_string_release(str_ipv6);
	zend_string_release(str_target);
	zend_string_release(str_pri);
}
