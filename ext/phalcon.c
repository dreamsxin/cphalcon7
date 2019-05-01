
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "php_phalcon.h"

#include <main/php_ini.h>
#include <ext/standard/info.h>
#include <Zend/zend_extensions.h>
#include <main/SAPI.h>

#include "cache/yac/storage.h"
#include "cache/yac/allocator.h"

#include "py.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/mbstring.h"
#include "kernel/time.h"

#include "interned-strings.h"

#include "phalcon.h"

ZEND_API void (*original_zend_execute_internal)(zend_execute_data *execute_data, zval *return_value);
ZEND_API void (*original_zend_execute_ex)(zend_execute_data *execute_data);

ZEND_API void (*xhprof_orig_zend_execute_internal)(zend_execute_data*, zval*);
ZEND_API void (*xhprof_orig_zend_execute_ex)(zend_execute_data*);

ZEND_API void (*async_orig_execute_ex)(zend_execute_data *exec);

ZEND_DECLARE_MODULE_GLOBALS(phalcon)

#ifdef PHALCON_CACHE_YAC
static PHP_INI_MH(OnChangeKeysMemoryLimit) {
	if (new_value) {
		PHALCON_GLOBAL(cache).yac_keys_size = zend_atol(ZSTR_VAL(new_value), ZSTR_LEN(new_value));
	}
	return SUCCESS;
}

static PHP_INI_MH(OnChangeValsMemoryLimit) {
	if (new_value) {
		PHALCON_GLOBAL(cache).yac_values_size = zend_atol(ZSTR_VAL(new_value), ZSTR_LEN(new_value));
	}
	return SUCCESS;
}
#endif

#if PHALCON_USE_UV
static PHP_INI_MH(OnUpdateFiberStackSize)
{
	OnUpdateLong(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);

	if (ASYNC_G(stack_size) < 0) {
		ASYNC_G(stack_size) = 0;
	}

	return SUCCESS;
}

static PHP_INI_MH(OnUpdateThreadCount)
{
	OnUpdateLong(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);

	if (ASYNC_G(threads) < 4) {
		ASYNC_G(threads) = 4;
	}

	if (ASYNC_G(threads) > 128) {
		ASYNC_G(threads) = 128;
	}

	return SUCCESS;
}
#endif

PHP_INI_BEGIN()
	/* Enables/Disables debug */
	STD_PHP_INI_BOOLEAN("phalcon.debug.enable_debug",           "0",    PHP_INI_ALL,    OnUpdateBool, debug.enable_debug,           zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables globally the internal events */
	STD_PHP_INI_BOOLEAN("phalcon.orm.events",                   "1",    PHP_INI_ALL,    OnUpdateBool, orm.events,                   zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables virtual foreign keys */
	STD_PHP_INI_BOOLEAN("phalcon.orm.virtual_foreign_keys",     "1",    PHP_INI_ALL,    OnUpdateBool, orm.virtual_foreign_keys,     zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables automatic NOT NULL validation */
	STD_PHP_INI_BOOLEAN("phalcon.orm.not_null_validations",     "1",    PHP_INI_ALL,    OnUpdateBool, orm.not_null_validations,     zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables automatic length validation */
	STD_PHP_INI_BOOLEAN("phalcon.orm.length_validations",       "1",    PHP_INI_ALL,    OnUpdateBool, orm.length_validations,       zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_BOOLEAN("phalcon.orm.use_mb_strlen",            "1",    PHP_INI_ALL,    OnUpdateBool, orm.use_mb_strlen,            zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables throwing an exception if save fails */
	STD_PHP_INI_BOOLEAN("phalcon.orm.exception_on_failed_save", "0",    PHP_INI_ALL,    OnUpdateBool, orm.exception_on_failed_save, zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables literals in PHQL */
	STD_PHP_INI_BOOLEAN("phalcon.orm.enable_literals",          "1",    PHP_INI_ALL,    OnUpdateBool, orm.enable_literals,          zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables property method */
	STD_PHP_INI_BOOLEAN("phalcon.orm.enable_property_method",   "1",    PHP_INI_ALL,    OnUpdateBool, orm.enable_property_method,   zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables auto convert column value follow database data type */
	STD_PHP_INI_BOOLEAN("phalcon.orm.enable_auto_convert",      "1",    PHP_INI_ALL,    OnUpdateBool, orm.enable_auto_convert,      zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_BOOLEAN("phalcon.orm.allow_update_primary",     "0",    PHP_INI_ALL,    OnUpdateBool, orm.allow_update_primary,     zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_BOOLEAN("phalcon.orm.enable_strict",            "0",    PHP_INI_ALL,    OnUpdateBool, orm.enable_strict,            zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_BOOLEAN("phalcon.orm.must_column",              "1",    PHP_INI_ALL,    OnUpdateBool, orm.must_column,              zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables allow empty */
	STD_PHP_INI_BOOLEAN("phalcon.validation.allow_empty",       "0",    PHP_INI_ALL,    OnUpdateBool, validation.allow_empty,       zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables auttomatic escape */
	STD_PHP_INI_BOOLEAN("phalcon.db.escape_identifiers",        "1",    PHP_INI_ALL,    OnUpdateBool, db.escape_identifiers,        zend_phalcon_globals, phalcon_globals)
#ifdef PHALCON_CACHE_YAC
	/* Enables/Disables cache memory */
	STD_PHP_INI_BOOLEAN("phalcon.cache.enable_yac",             "1",   PHP_INI_ALL,    OnUpdateBool, cache.enable_yac,          zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_BOOLEAN("phalcon.cache.enable_yac_cli",         "0",   PHP_INI_ALL,    OnUpdateBool, cache.enable_yac_cli,      zend_phalcon_globals, phalcon_globals)
    STD_PHP_INI_ENTRY("phalcon.cache.yac_keys_size",            "4M",  PHP_INI_SYSTEM, OnChangeKeysMemoryLimit, cache.yac_keys_size,       zend_phalcon_globals, phalcon_globals)
    STD_PHP_INI_ENTRY("phalcon.cache.yac_values_size",          "64M", PHP_INI_SYSTEM, OnChangeValsMemoryLimit, cache.yac_values_size,     zend_phalcon_globals, phalcon_globals)
#endif
	/* Enables/Disables xhprof */
	STD_PHP_INI_ENTRY("phalcon.xhprof.nesting_max_level", "0",  PHP_INI_ALL, OnUpdateLong, xhprof.nesting_maximum_level,	zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_BOOLEAN("phalcon.xhprof.enable_xhprof",   "0",  PHP_INI_SYSTEM, OnUpdateBool, xhprof.enable_xhprof,	zend_phalcon_globals, phalcon_globals)
    STD_PHP_INI_ENTRY("phalcon.xhprof.clock_use_rdtsc",   "0",  PHP_INI_ALL, OnUpdateBool, xhprof.clock_use_rdtsc,	zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_ENTRY("phalcon.snowflake.node", "0", PHP_INI_SYSTEM, OnUpdateLong, snowflake.node,  zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_BOOLEAN("phalcon.aop.enable_aop",   "0", PHP_INI_SYSTEM, OnUpdateBool, aop.enable_aop, zend_phalcon_globals, phalcon_globals)
#if PHALCON_USE_UV
	STD_PHP_INI_ENTRY("phalcon.async.dns",        "0", PHP_INI_SYSTEM | PHP_INI_PERDIR, OnUpdateBool, async.dns_enabled, zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_ENTRY("phalcon.async.fs_enabled", "0", PHP_INI_SYSTEM | PHP_INI_PERDIR, OnUpdateBool, async.fs_enabled, zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_ENTRY("phalcon.async.forked",     "0", PHP_INI_SYSTEM | PHP_INI_PERDIR, OnUpdateBool, async.forked, zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_ENTRY("phalcon.async.stack_size", "0", PHP_INI_SYSTEM | PHP_INI_PERDIR, OnUpdateFiberStackSize, async.stack_size, zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_ENTRY("phalcon.async.tcp",        "0", PHP_INI_SYSTEM | PHP_INI_PERDIR, OnUpdateBool, async.tcp_enabled, zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_ENTRY("phalcon.async.threads",    "4", PHP_INI_SYSTEM | PHP_INI_PERDIR, OnUpdateThreadCount, async.threads, zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_ENTRY("phalcon.async.timer",      "0", PHP_INI_SYSTEM | PHP_INI_PERDIR, OnUpdateBool, async.timer_enabled, zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_ENTRY("phalcon.async.udp",        "0", PHP_INI_SYSTEM | PHP_INI_PERDIR, OnUpdateBool, async.udp_enabled, zend_phalcon_globals, phalcon_globals)
#endif
PHP_INI_END()

#if PHALCON_USE_UV
ASYNC_CALLBACK init_threads(uv_work_t *req) { }

ASYNC_CALLBACK after_init_threads(uv_work_t *req, int status)
{
	ASYNC_G(threads) = 0;

	efree(req);
}
#endif

static PHP_MINIT_FUNCTION(phalcon)
{
	REGISTER_INI_ENTRIES();

#ifdef PHALCON_CACHE_YAC
	if (!PHALCON_GLOBAL(cache).enable_yac_cli && !strcmp(sapi_module.name, "cli")) {
		PHALCON_GLOBAL(cache).enable_yac = 0;
	}

	if (PHALCON_GLOBAL(cache).enable_yac) {
		char *msg;
		if (PHALCON_GLOBAL(cache).yac_values_size < PHALCON_CACHE_YAC_SMM_SEGMENT_MIN_SIZE) {
			php_error(E_ERROR, "Shared memory values(values_memory_size) must be at least '%d'", PHALCON_CACHE_YAC_SMM_SEGMENT_MIN_SIZE);
			return FAILURE;
		}
		if (!phalcon_cache_yac_storage_startup(PHALCON_GLOBAL(cache).yac_keys_size, PHALCON_GLOBAL(cache).yac_values_size, &msg)) {
			php_error(E_ERROR, "Shared memory allocator startup failed at '%s': %s", msg, strerror(errno));
			return FAILURE;
		}
	}
#else
	PHALCON_GLOBAL(cache).enable_yac = 0;
#endif

    if (PHALCON_GLOBAL(snowflake).node < 0) {
		php_error_docref(NULL, E_WARNING, "snowflake.node must greater than 0");
		PHALCON_GLOBAL(snowflake).node = 0;
    }

    if (PHALCON_GLOBAL(snowflake).node > 0x3FF) {
		php_error_docref(NULL, E_WARNING, "snowflake.node must less than %d", 0x3FF);
		PHALCON_GLOBAL(snowflake).node = 0;
    }

	if (PHALCON_GLOBAL(aop).enable_aop) {
#if PHP_VERSION_ID >= 70300
		zend_object_handlers *handlers = (zend_object_handlers *)zend_get_std_object_handlers();
#endif
		// overload zend_execute_ex and zend_execute_internal
		original_zend_execute_internal = zend_execute_internal;
		zend_execute_internal = phalcon_aop_execute_internal;

		original_zend_execute_ex = zend_execute_ex;
		zend_execute_ex = phalcon_aop_execute_ex;

#if PHP_VERSION_ID >= 70300
		// overload zend_std_read_property and zend_std_write_property
		original_zend_std_read_property = handlers->read_property;
		handlers->read_property = phalcon_aop_read_property;

		original_zend_std_write_property = handlers->write_property;
		handlers->write_property = phalcon_aop_write_property;

		original_zend_std_get_property_ptr_ptr = handlers->get_property_ptr_ptr;
		handlers->get_property_ptr_ptr = phalcon_aop_get_property_ptr_ptr;
#else
		// overload zend_std_read_property and zend_std_write_property
		original_zend_std_read_property = std_object_handlers.read_property;
		std_object_handlers.read_property = phalcon_aop_read_property;

		original_zend_std_write_property = std_object_handlers.write_property;
		std_object_handlers.write_property = phalcon_aop_write_property;

		original_zend_std_get_property_ptr_ptr = std_object_handlers.get_property_ptr_ptr;
		std_object_handlers.get_property_ptr_ptr = phalcon_aop_get_property_ptr_ptr;
#endif
	}

	if (PHALCON_GLOBAL(xhprof).enable_xhprof) {
		xhprof_orig_zend_execute_internal = zend_execute_internal;
		zend_execute_internal = phalcon_xhprof_execute_internal;

		xhprof_orig_zend_execute_ex = zend_execute_ex;
		zend_execute_ex = phalcon_xhprof_execute_ex;
	}

#if PHALCON_USE_MONGOC
	mongoc_init();
#endif

#if PHALCON_USE_PYTHON
	Py_SetProgramName("Phalcon7");
	Py_InitializeEx(0); // Py_Initialize();
	PyEval_InitThreads();
	python_streams_init();
	PHALCON_GLOBAL(python).mtstate = PyEval_SaveThread();
	PHALCON_GLOBAL(python).isInitialized = Py_IsInitialized();
	PyEval_ReleaseLock();
#endif

#ifdef PHALCON_USE_UV
	ASYNC_G(cli) = !strcmp(sapi_module.name, "cli");
	if (ASYNC_G(cli)) {
		uv_work_t *req = emalloc(sizeof(uv_work_t));
		char entry[4];

		sprintf(entry, "%d", (int) MAX(4, MIN(128, ASYNC_G(threads))));
		uv_os_setenv("UV_THREADPOOL_SIZE", (const char *) entry);

		uv_queue_work(uv_default_loop(), req, init_threads, after_init_threads);
		uv_cancel((uv_req_t *) req);
		uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	}

#ifdef HAVE_ASYNC_SSL
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined (LIBRESSL_VERSION_NUMBER)
	SSL_library_init();
	OPENSSL_config(NULL);
	SSL_load_error_strings();
#else
	OPENSSL_init_ssl(OPENSSL_INIT_LOAD_CONFIG, NULL);
#endif
#endif
	async_awaitable_ce_register();

	async_stream_ce_register();
	async_socket_ce_register();

	async_channel_ce_register();
	async_console_ce_register();
	async_context_ce_register();
	async_deferred_ce_register();
	async_dns_ce_register();
	async_pipe_ce_register();
	async_process_ce_register();
	async_signal_watcher_ce_register();
	async_ssl_ce_register();
	async_stream_watcher_ce_register();
	async_task_ce_register();
	async_tcp_ce_register();
	async_timer_ce_register();
	async_udp_socket_ce_register();
	
	async_orig_execute_ex = zend_execute_ex;
	zend_execute_ex = async_execute_ex;
#endif

	/* 1. Register exceptions */
	PHALCON_INIT(Phalcon_Exception);
	PHALCON_INIT(Phalcon_ContinueException);
	PHALCON_INIT(Phalcon_ExitException);
	PHALCON_INIT(Phalcon_Profiler_Exception);
	PHALCON_INIT(Phalcon_Debug_Exception);
	PHALCON_INIT(Phalcon_Acl_Exception);
	PHALCON_INIT(Phalcon_Annotations_Exception);
	PHALCON_INIT(Phalcon_Assets_Exception);
	PHALCON_INIT(Phalcon_Cache_Exception);
	PHALCON_INIT(Phalcon_Crypt_Exception);
	PHALCON_INIT(Phalcon_Db_Exception);
	PHALCON_INIT(Phalcon_Db_Builder_Exception);
	PHALCON_INIT(Phalcon_Di_Exception);
	PHALCON_INIT(Phalcon_Escaper_Exception);
	PHALCON_INIT(Phalcon_Events_Exception);
	PHALCON_INIT(Phalcon_Filter_Exception);
	PHALCON_INIT(Phalcon_Flash_Exception);
	PHALCON_INIT(Phalcon_Forms_Exception);
	PHALCON_INIT(Phalcon_Http_Cookie_Exception);
	PHALCON_INIT(Phalcon_Http_Request_Exception);
	PHALCON_INIT(Phalcon_Http_Response_Exception);
	PHALCON_INIT(Phalcon_Http_Client_Exception);
	PHALCON_INIT(Phalcon_Image_Exception);

	PHALCON_INIT(Phalcon_Router_Exception);
	PHALCON_INIT(Phalcon_Application_Exception);
	PHALCON_INIT(Phalcon_Cli_Console_Exception);
	PHALCON_INIT(Phalcon_Cli_Dispatcher_Exception);
	PHALCON_INIT(Phalcon_Cli_Router_Exception);
	PHALCON_INIT(Phalcon_Cli_Options_Exception);
	PHALCON_INIT(Phalcon_Mvc_Application_Exception);
	PHALCON_INIT(Phalcon_Mvc_Dispatcher_Exception);
	PHALCON_INIT(Phalcon_Mvc_Micro_Exception);
	PHALCON_INIT(Phalcon_Mvc_Model_Exception);
	PHALCON_INIT(Phalcon_Mvc_Model_Query_Exception);
	PHALCON_INIT(Phalcon_Mvc_Model_Transaction_Exception);
	PHALCON_INIT(Phalcon_Mvc_Router_Exception);
	PHALCON_INIT(Phalcon_Mvc_Url_Exception);
	PHALCON_INIT(Phalcon_Mvc_View_Exception);
	PHALCON_INIT(Phalcon_Paginator_Exception);
	PHALCON_INIT(Phalcon_Tag_Exception);
	PHALCON_INIT(Phalcon_Validation_Exception);
	PHALCON_INIT(Phalcon_Security_Exception);
	PHALCON_INIT(Phalcon_Session_Exception);
	PHALCON_INIT(Phalcon_Config_Exception);
	PHALCON_INIT(Phalcon_Loader_Exception);
	PHALCON_INIT(Phalcon_Logger_Exception);
	PHALCON_INIT(Phalcon_Translate_Exception);
	PHALCON_INIT(Phalcon_Mvc_Micro_Exception);
	PHALCON_INIT(Phalcon_Chart_Exception);
	PHALCON_INIT(Phalcon_Binary_Exception);
	PHALCON_INIT(Phalcon_Socket_Exception);
	PHALCON_INIT(Phalcon_Process_Exception);
	PHALCON_INIT(Phalcon_Storage_Exception);
	PHALCON_INIT(Phalcon_Server_Exception);
#if PHALCON_USE_SHM_OPEN
	PHALCON_INIT(Phalcon_Sync_Exception);
#endif
#if PHALCON_USE_PYTHON
	PHALCON_INIT(Phalcon_Py_Exception);
#endif
	PHALCON_INIT(Phalcon_Thread_Exception);
	PHALCON_INIT(Phalcon_Aop_Exception);

	/* 2. Register interfaces */
	PHALCON_INIT(Phalcon_DiInterface);
	PHALCON_INIT(Phalcon_Di_InjectionAwareInterface);
	PHALCON_INIT(Phalcon_Di_ServiceInterface);
	PHALCON_INIT(Phalcon_Events_EventInterface);
	PHALCON_INIT(Phalcon_Events_EventsAwareInterface);

	PHALCON_INIT(Phalcon_ProfilerInterface);
	PHALCON_INIT(Phalcon_Profiler_ItemInterface);

	PHALCON_INIT(Phalcon_Acl_AdapterInterface);
	PHALCON_INIT(Phalcon_Acl_ResourceInterface);
	PHALCON_INIT(Phalcon_Acl_ResourceAware);
	PHALCON_INIT(Phalcon_Acl_RoleInterface);
	PHALCON_INIT(Phalcon_Acl_RoleAware);
	PHALCON_INIT(Phalcon_Annotations_AdapterInterface);
	PHALCON_INIT(Phalcon_Annotations_ReaderInterface);
	PHALCON_INIT(Phalcon_Assets_FilterInterface);
	PHALCON_INIT(Phalcon_Cache_BackendInterface);
	PHALCON_INIT(Phalcon_Cache_FrontendInterface);
	PHALCON_INIT(Phalcon_CryptInterface);
	PHALCON_INIT(Phalcon_Db_AdapterInterface);
	PHALCON_INIT(Phalcon_Db_ColumnInterface);
	PHALCON_INIT(Phalcon_Db_DialectInterface);
	PHALCON_INIT(Phalcon_Db_IndexInterface);
	PHALCON_INIT(Phalcon_Db_ReferenceInterface);
	PHALCON_INIT(Phalcon_Db_ResultInterface);
	PHALCON_INIT(Phalcon_DispatcherInterface);
	PHALCON_INIT(Phalcon_Config_AdapterInterface);
	PHALCON_INIT(Phalcon_EscaperInterface);
	PHALCON_INIT(Phalcon_Events_ManagerInterface);
	PHALCON_INIT(Phalcon_FlashInterface);
	PHALCON_INIT(Phalcon_FilterInterface);
	PHALCON_INIT(Phalcon_Filter_UserFilterInterface);
	PHALCON_INIT(Phalcon_Forms_ElementInterface);
	PHALCON_INIT(Phalcon_Http_RequestInterface);
	PHALCON_INIT(Phalcon_Http_Request_FileInterface);
	PHALCON_INIT(Phalcon_Http_ResponseInterface);
	PHALCON_INIT(Phalcon_Http_Response_CookiesInterface);
	PHALCON_INIT(Phalcon_Http_Response_HeadersInterface);
	PHALCON_INIT(Phalcon_Http_Client_AdapterInterface);
	PHALCON_INIT(Phalcon_Image_AdapterInterface);
	PHALCON_INIT(Phalcon_Logger_AdapterInterface);
	PHALCON_INIT(Phalcon_Logger_FormatterInterface);

	PHALCON_INIT(Phalcon_RouterInterface);
	PHALCON_INIT(Phalcon_Db_BuilderInterface);
	PHALCON_INIT(Phalcon_Mvc_ControllerInterface);
	PHALCON_INIT(Phalcon_Mvc_DispatcherInterface);
	PHALCON_INIT(Phalcon_Mvc_Micro_CollectionInterface);
	PHALCON_INIT(Phalcon_Mvc_Micro_MiddlewareInterface);
	PHALCON_INIT(Phalcon_Mvc_ModelInterface);
	PHALCON_INIT(Phalcon_Mvc_Model_BehaviorInterface);
	PHALCON_INIT(Phalcon_Mvc_Model_CriteriaInterface);
	PHALCON_INIT(Phalcon_Mvc_Model_ManagerInterface);
	PHALCON_INIT(Phalcon_Mvc_Model_MetaDataInterface);
	PHALCON_INIT(Phalcon_Mvc_Model_QueryInterface);
	PHALCON_INIT(Phalcon_Mvc_Model_Query_BuilderInterface);
	PHALCON_INIT(Phalcon_Mvc_Model_Query_StatusInterface);
	PHALCON_INIT(Phalcon_Mvc_Model_RelationInterface);
	PHALCON_INIT(Phalcon_Mvc_Model_ResultInterface);
	PHALCON_INIT(Phalcon_Mvc_Model_ResultsetInterface);
	PHALCON_INIT(Phalcon_Mvc_Model_TransactionInterface);
	PHALCON_INIT(Phalcon_Mvc_Model_Transaction_ManagerInterface);
	PHALCON_INIT(Phalcon_Mvc_ModuleDefinitionInterface);
	PHALCON_INIT(Phalcon_Mvc_RouterInterface);
	PHALCON_INIT(Phalcon_Mvc_Router_RouteInterface);
	PHALCON_INIT(Phalcon_Mvc_UrlInterface);
	PHALCON_INIT(Phalcon_Mvc_ViewInterface);
	PHALCON_INIT(Phalcon_Mvc_View_EngineInterface);
	PHALCON_INIT(Phalcon_Mvc_View_ModelInterface);
	PHALCON_INIT(Phalcon_Paginator_AdapterInterface);
	PHALCON_INIT(Phalcon_Session_AdapterInterface);
	PHALCON_INIT(Phalcon_Session_BagInterface);
	PHALCON_INIT(Phalcon_Translate_AdapterInterface);
	PHALCON_INIT(Phalcon_ValidationInterface);
	PHALCON_INIT(Phalcon_Validation_ValidatorInterface);
	PHALCON_INIT(Phalcon_Validation_MessageInterface);

#ifdef PHALCON_USE_WEBSOCKET
	PHALCON_INIT(Phalcon_Websocket_EventloopInterface);
#endif

	/* 4. Register everything else */
	PHALCON_INIT(Phalcon_Xhprof);
	PHALCON_INIT(Phalcon_Di);
	PHALCON_INIT(Phalcon_Di_Injectable);
	PHALCON_INIT(Phalcon_Di_FactoryDefault);
	PHALCON_INIT(Phalcon_Di_FactoryDefault_Cli);
	PHALCON_INIT(Phalcon_Di_Service);
	PHALCON_INIT(Phalcon_Di_Service_Builder);

	PHALCON_INIT(Phalcon_Profiler);
	PHALCON_INIT(Phalcon_Profiler_Item);

	PHALCON_INIT(Phalcon_Forms_Element);
	PHALCON_INIT(Phalcon_Annotations_Adapter);
	PHALCON_INIT(Phalcon_Logger_Adapter);
	PHALCON_INIT(Phalcon_Logger_Formatter);
	PHALCON_INIT(Phalcon_Assets_Resource);
	PHALCON_INIT(Phalcon_Flash);
	PHALCON_INIT(Phalcon_Dispatcher);
	PHALCON_INIT(Phalcon_Translate_Adapter);
	PHALCON_INIT(Phalcon_Config);
	PHALCON_INIT(Phalcon_Config_Adapter);
	PHALCON_INIT(Phalcon_Config_Adapter_Ini);
	PHALCON_INIT(Phalcon_Config_Adapter_Json);
	PHALCON_INIT(Phalcon_Config_Adapter_Php);
	PHALCON_INIT(Phalcon_Config_Adapter_Yaml);
	PHALCON_INIT(Phalcon_Acl);
	PHALCON_INIT(Phalcon_Acl_Adapter);
	PHALCON_INIT(Phalcon_Acl_Role);
	PHALCON_INIT(Phalcon_Acl_Resource);
	PHALCON_INIT(Phalcon_Acl_Adapter_Memory);
	PHALCON_INIT(Phalcon_Session_Adapter);
#ifdef PHALCON_CACHE_YAC
	PHALCON_INIT(Phalcon_Cache_Yac);
#endif
	PHALCON_INIT(Phalcon_Cache_Backend);
	PHALCON_INIT(Phalcon_Cache_Frontend_Data);
	PHALCON_INIT(Phalcon_Cache_Multiple);
	PHALCON_INIT(Phalcon_Cache_Backend_Apc);
	PHALCON_INIT(Phalcon_Cache_Backend_File);
	PHALCON_INIT(Phalcon_Cache_Backend_Memory);
#ifdef PHALCON_USE_MONGOC
	PHALCON_INIT(Phalcon_Cache_Backend_Mongo);
#endif
	PHALCON_INIT(Phalcon_Cache_Backend_Memcached);
	PHALCON_INIT(Phalcon_Cache_Backend_Redis);
	PHALCON_INIT(Phalcon_Cache_Backend_Yac);
#if PHALCON_USE_WIREDTIGER
	PHALCON_INIT(Phalcon_Cache_Backend_Wiredtiger);
#endif
#if PHALCON_USE_LMDB
	PHALCON_INIT(Phalcon_Cache_Backend_Lmdb);
#endif

	PHALCON_INIT(Phalcon_Cache_Frontend_Json);
	PHALCON_INIT(Phalcon_Cache_Frontend_Output);
	PHALCON_INIT(Phalcon_Cache_Frontend_None);
	PHALCON_INIT(Phalcon_Cache_Frontend_Base64);
	PHALCON_INIT(Phalcon_Cache_Frontend_Igbinary);
	PHALCON_INIT(Phalcon_Tag);
	PHALCON_INIT(Phalcon_Tag_Select);
	PHALCON_INIT(Phalcon_Paginator_Adapter);
	PHALCON_INIT(Phalcon_Paginator_Adapter_Model);
	PHALCON_INIT(Phalcon_Paginator_Adapter_NativeArray);
	PHALCON_INIT(Phalcon_Paginator_Adapter_QueryBuilder);
	PHALCON_INIT(Phalcon_Paginator_Adapter_DbBuilder);
	PHALCON_INIT(Phalcon_Paginator_Adapter_Sql);
	PHALCON_INIT(Phalcon_Validation);
	PHALCON_INIT(Phalcon_Validation_Validator);
	PHALCON_INIT(Phalcon_Validation_Message);
	PHALCON_INIT(Phalcon_Validation_Message_Group);
	PHALCON_INIT(Phalcon_Validation_Validator_Regex);
	PHALCON_INIT(Phalcon_Validation_Validator_Email);
	PHALCON_INIT(Phalcon_Validation_Validator_Between);
	PHALCON_INIT(Phalcon_Validation_Validator_Identical);
	PHALCON_INIT(Phalcon_Validation_Validator_PresenceOf);
	PHALCON_INIT(Phalcon_Validation_Validator_InclusionIn);
	PHALCON_INIT(Phalcon_Validation_Validator_StringLength);
	PHALCON_INIT(Phalcon_Validation_Validator_ExclusionIn);
	PHALCON_INIT(Phalcon_Validation_Validator_Confirmation);
	PHALCON_INIT(Phalcon_Validation_Validator_Url);
	PHALCON_INIT(Phalcon_Validation_Validator_File);
	PHALCON_INIT(Phalcon_Validation_Validator_Numericality);
	PHALCON_INIT(Phalcon_Validation_Validator_Json);
	PHALCON_INIT(Phalcon_Validation_Validator_Uniqueness);
	PHALCON_INIT(Phalcon_Validation_Validator_Alnum);
	PHALCON_INIT(Phalcon_Validation_Validator_Alpha);
	PHALCON_INIT(Phalcon_Validation_Validator_Digit);
	PHALCON_INIT(Phalcon_Validation_Validator_Date);
	PHALCON_INIT(Phalcon_Db);
	PHALCON_INIT(Phalcon_Db_Adapter);
	PHALCON_INIT(Phalcon_Db_Adapter_Pdo);
	PHALCON_INIT(Phalcon_Db_Adapter_Pdo_Sqlite);
	PHALCON_INIT(Phalcon_Db_Adapter_Pdo_Mysql);
	PHALCON_INIT(Phalcon_Db_Adapter_Pdo_Postgresql);
	PHALCON_INIT(Phalcon_Db_Index);
	PHALCON_INIT(Phalcon_Db_Column);
	PHALCON_INIT(Phalcon_Db_Dialect);
	PHALCON_INIT(Phalcon_Db_Dialect_Sqlite);
	PHALCON_INIT(Phalcon_Db_Dialect_Mysql);
	PHALCON_INIT(Phalcon_Db_Dialect_Postgresql);
	PHALCON_INIT(Phalcon_Db_Profiler);
	PHALCON_INIT(Phalcon_Db_Profiler_Item);
	PHALCON_INIT(Phalcon_Db_RawValue);
	PHALCON_INIT(Phalcon_Db_Reference);
	PHALCON_INIT(Phalcon_Db_Result_Pdo);
	PHALCON_INIT(Phalcon_Db_Builder);
	PHALCON_INIT(Phalcon_Db_Builder_Where);
	PHALCON_INIT(Phalcon_Db_Builder_Join);
	PHALCON_INIT(Phalcon_Db_Builder_Select);
	PHALCON_INIT(Phalcon_Db_Builder_Update);
	PHALCON_INIT(Phalcon_Db_Builder_Insert);
	PHALCON_INIT(Phalcon_Db_Builder_Delete);
	PHALCON_INIT(Phalcon_Kernel);
	PHALCON_INIT(Phalcon_Debug);
	PHALCON_INIT(Phalcon_Debug_Dump);
	PHALCON_INIT(Phalcon_Text);
	PHALCON_INIT(Phalcon_Date);
	PHALCON_INIT(Phalcon_Date_DateTime);
	PHALCON_INIT(Phalcon_Random);
	PHALCON_INIT(Phalcon_Security);
	PHALCON_INIT(Phalcon_Security_Random);
	PHALCON_INIT(Phalcon_Version);
	PHALCON_INIT(Phalcon_Session_Bag);
	PHALCON_INIT(Phalcon_Session_Adapter_Files);
	PHALCON_INIT(Phalcon_Session_Adapter_Memcached);
	PHALCON_INIT(Phalcon_Session_Adapter_Cache);
	PHALCON_INIT(Phalcon_Filter);
	PHALCON_INIT(Phalcon_Flash_Direct);
	PHALCON_INIT(Phalcon_Flash_Session);
	PHALCON_INIT(Phalcon_Annotations_Reader);
	PHALCON_INIT(Phalcon_Annotations_Annotation);
	PHALCON_INIT(Phalcon_Annotations_Adapter_Apc);
	PHALCON_INIT(Phalcon_Annotations_Collection);
	PHALCON_INIT(Phalcon_Annotations_Reflection);
	PHALCON_INIT(Phalcon_Annotations_Adapter_Files);
	PHALCON_INIT(Phalcon_Annotations_Adapter_Memory);
	PHALCON_INIT(Phalcon_Annotations_Adapter_Cache);
	PHALCON_INIT(Phalcon_Loader);
	PHALCON_INIT(Phalcon_Logger);
	PHALCON_INIT(Phalcon_Logger_Item);
	PHALCON_INIT(Phalcon_Logger_Multiple);
	PHALCON_INIT(Phalcon_Logger_Formatter_Json);
	PHALCON_INIT(Phalcon_Logger_Formatter_Line);
	PHALCON_INIT(Phalcon_Logger_Formatter_Firephp);
	PHALCON_INIT(Phalcon_Logger_Formatter_Syslog);
	PHALCON_INIT(Phalcon_Logger_Adapter_Stream);
	PHALCON_INIT(Phalcon_Logger_Adapter_File);
	PHALCON_INIT(Phalcon_Logger_Adapter_Firephp);
	PHALCON_INIT(Phalcon_Logger_Adapter_Syslog);
	PHALCON_INIT(Phalcon_Logger_Adapter_Direct);
	PHALCON_INIT(Phalcon_Forms_Form);
	PHALCON_INIT(Phalcon_Forms_Manager);
	PHALCON_INIT(Phalcon_Forms_Element_Text);
	PHALCON_INIT(Phalcon_Forms_Element_Date);
	PHALCON_INIT(Phalcon_Forms_Element_File);
	PHALCON_INIT(Phalcon_Forms_Element_Hidden);
	PHALCON_INIT(Phalcon_Forms_Element_Select);
	PHALCON_INIT(Phalcon_Forms_Element_Check);
	PHALCON_INIT(Phalcon_Forms_Element_Numeric);
	PHALCON_INIT(Phalcon_Forms_Element_Email);
	PHALCON_INIT(Phalcon_Forms_Element_Submit);
	PHALCON_INIT(Phalcon_Forms_Element_Password);
	PHALCON_INIT(Phalcon_Forms_Element_TextArea);
	PHALCON_INIT(Phalcon_Forms_Element_Radio);
	PHALCON_INIT(Phalcon_Crypt);
	PHALCON_INIT(Phalcon_Translate_Adapter_NativeArray);
	PHALCON_INIT(Phalcon_Translate_Adapter_Gettext);
	PHALCON_INIT(Phalcon_Translate_Adapter_Php);
	PHALCON_INIT(Phalcon_Escaper);
	PHALCON_INIT(Phalcon_Assets_Manager);
	PHALCON_INIT(Phalcon_Assets_Resource_Js);
	PHALCON_INIT(Phalcon_Assets_Collection);
	PHALCON_INIT(Phalcon_Assets_Filters_None);
	PHALCON_INIT(Phalcon_Assets_Filters_Cssmin);
	PHALCON_INIT(Phalcon_Assets_Filters_Jsmin);
	PHALCON_INIT(Phalcon_Assets_Resource_Css);
	PHALCON_INIT(Phalcon_Http_Parser);
	PHALCON_INIT(Phalcon_Http_Request);
	PHALCON_INIT(Phalcon_Http_Cookie);
	PHALCON_INIT(Phalcon_Http_Response);
	PHALCON_INIT(Phalcon_Http_Request_File);
	PHALCON_INIT(Phalcon_Http_Response_Cookies);
	PHALCON_INIT(Phalcon_Http_Response_Headers);
	PHALCON_INIT(Phalcon_Http_Uri);
	PHALCON_INIT(Phalcon_Http_Client);
	PHALCON_INIT(Phalcon_Http_Client_Header);
	PHALCON_INIT(Phalcon_Http_Client_Response);
	PHALCON_INIT(Phalcon_Http_Client_Adapter);
	PHALCON_INIT(Phalcon_Http_Client_Adapter_Curl);
	PHALCON_INIT(Phalcon_Http_Client_Adapter_Stream);
	PHALCON_INIT(Phalcon_Queue_Beanstalk);
	PHALCON_INIT(Phalcon_Queue_Beanstalk_Job);

	PHALCON_INIT(Phalcon_User_Module);
	PHALCON_INIT(Phalcon_User_Plugin);
	PHALCON_INIT(Phalcon_User_Component);
	PHALCON_INIT(Phalcon_User_Logic);

	PHALCON_INIT(Phalcon_Application);
	PHALCON_INIT(Phalcon_Router);
	PHALCON_INIT(Phalcon_Cli_Task);
	PHALCON_INIT(Phalcon_Cli_Router);
	PHALCON_INIT(Phalcon_Cli_Console);
	PHALCON_INIT(Phalcon_Cli_Dispatcher);
	PHALCON_INIT(Phalcon_Cli_Options);
	PHALCON_INIT(Phalcon_Cli_Color);
	PHALCON_INIT(Phalcon_Mvc_Router);
	PHALCON_INIT(Phalcon_Mvc_View_Engine);
	PHALCON_INIT(Phalcon_Mvc_View_Model);
	PHALCON_INIT(Phalcon_Mvc_View);
	PHALCON_INIT(Phalcon_Mvc_Url);
	PHALCON_INIT(Phalcon_Mvc_Micro);
	PHALCON_INIT(Phalcon_Mvc_Application);
	PHALCON_INIT(Phalcon_Mvc_Controller);
	PHALCON_INIT(Phalcon_Mvc_Dispatcher);
	PHALCON_INIT(Phalcon_Mvc_Model);
	PHALCON_INIT(Phalcon_Mvc_Model_Resultset);
	PHALCON_INIT(Phalcon_Mvc_Model_Behavior);
	PHALCON_INIT(Phalcon_Mvc_Model_Row);
	PHALCON_INIT(Phalcon_Mvc_Model_Query);
	PHALCON_INIT(Phalcon_Mvc_Micro_Collection);
	PHALCON_INIT(Phalcon_Mvc_Micro_LazyLoader);
	PHALCON_INIT(Phalcon_Mvc_Model_Criteria);
	PHALCON_INIT(Phalcon_Mvc_Model_Manager);
	PHALCON_INIT(Phalcon_Mvc_Model_Relation);
	PHALCON_INIT(Phalcon_Mvc_Model_Query_Lang);
	PHALCON_INIT(Phalcon_Mvc_Model_Query_Status);
	PHALCON_INIT(Phalcon_Mvc_Model_Query_Builder);
	PHALCON_INIT(Phalcon_Mvc_Model_Query_Builder_Where);
	PHALCON_INIT(Phalcon_Mvc_Model_Query_Builder_Join);
	PHALCON_INIT(Phalcon_Mvc_Model_Query_Builder_Select);
	PHALCON_INIT(Phalcon_Mvc_Model_Query_Builder_Update);
	PHALCON_INIT(Phalcon_Mvc_Model_Query_Builder_Insert);
	PHALCON_INIT(Phalcon_Mvc_Model_Query_Builder_Delete);
	PHALCON_INIT(Phalcon_Mvc_Model_ValidationFailed);
	PHALCON_INIT(Phalcon_Mvc_Model_Resultset_Simple);
	PHALCON_INIT(Phalcon_Mvc_Model_Resultset_Complex);
	PHALCON_INIT(Phalcon_Mvc_Model_MetaData);
	PHALCON_INIT(Phalcon_Mvc_Model_MetaData_Apc);
	PHALCON_INIT(Phalcon_Mvc_Model_MetaData_Files);
	PHALCON_INIT(Phalcon_Mvc_Model_MetaData_Session);
	PHALCON_INIT(Phalcon_Mvc_Model_MetaData_Memcached);
	PHALCON_INIT(Phalcon_Mvc_Model_MetaData_Redis);
#if PHALCON_USE_MONGOC
	PHALCON_INIT(Phalcon_Mvc_Model_MetaData_Mongo);
#endif
	PHALCON_INIT(Phalcon_Mvc_Model_MetaData_Cache);
	PHALCON_INIT(Phalcon_Mvc_Model_MetaData_Memory);
	PHALCON_INIT(Phalcon_Mvc_Model_MetaData_Strategy_Annotations);
	PHALCON_INIT(Phalcon_Mvc_Model_MetaData_Strategy_Introspection);
	PHALCON_INIT(Phalcon_Mvc_Model_Transaction);
	PHALCON_INIT(Phalcon_Mvc_Model_Transaction_Manager);
	PHALCON_INIT(Phalcon_Mvc_Model_Transaction_Failed);
	PHALCON_INIT(Phalcon_Mvc_Model_Behavior_SoftDelete);
	PHALCON_INIT(Phalcon_Mvc_Model_Behavior_Timestampable);
	PHALCON_INIT(Phalcon_Mvc_Router_Route);
	PHALCON_INIT(Phalcon_Mvc_Router_Group);
	PHALCON_INIT(Phalcon_Mvc_Router_Annotations);
	PHALCON_INIT(Phalcon_Mvc_User_Module);
	PHALCON_INIT(Phalcon_Mvc_User_Plugin);
	PHALCON_INIT(Phalcon_Mvc_User_Component);
	PHALCON_INIT(Phalcon_Mvc_User_Logic);
	PHALCON_INIT(Phalcon_Mvc_User_Logic_Model);
	PHALCON_INIT(Phalcon_Mvc_View_Simple);
	PHALCON_INIT(Phalcon_Mvc_View_Engine_Php);
	PHALCON_INIT(Phalcon_Events_Event);
	PHALCON_INIT(Phalcon_Events_Manager);
	PHALCON_INIT(Phalcon_Events_Listener);
	PHALCON_INIT(Phalcon_Image);
	PHALCON_INIT(Phalcon_Image_Adapter);
	PHALCON_INIT(Phalcon_Image_Adapter_GD);
	PHALCON_INIT(Phalcon_Image_Adapter_Imagick);
	PHALCON_INIT(Phalcon_Registry);
	PHALCON_INIT(Phalcon_Arr);
	PHALCON_INIT(Phalcon_Matrix);

#ifdef PHALCON_CHART
	PHALCON_INIT(Phalcon_Chart_Captcha);
#endif

	PHALCON_INIT(Phalcon_Chart_Captcha_Tiny);

#if PHALCON_USE_QRENCODE
	PHALCON_INIT(Phalcon_Chart_QRcode);
#endif

	PHALCON_INIT(Phalcon_Async);

	PHALCON_INIT(Phalcon_Thread_Pool);

#if PHALCON_USE_SHM_OPEN
	PHALCON_INIT(Phalcon_Sync_Mutex);
	PHALCON_INIT(Phalcon_Sync_Readerwriter);
	PHALCON_INIT(Phalcon_Sync_Event);
	PHALCON_INIT(Phalcon_Sync_Semaphore);
	PHALCON_INIT(Phalcon_Sync_Sharedmemory);
#endif

	PHALCON_INIT(Phalcon_Binary);
	PHALCON_INIT(Phalcon_Binary_Reader);
	PHALCON_INIT(Phalcon_Binary_Writer);

#ifdef PHALCON_PROCESS
	PHALCON_INIT(Phalcon_Process_Proc);
	PHALCON_INIT(Phalcon_Process_Sharedmemory);
#endif

	PHALCON_INIT(Phalcon_Socket);
	PHALCON_INIT(Phalcon_Socket_Client);
	PHALCON_INIT(Phalcon_Socket_Server);

#ifdef PHALCON_USE_WEBSOCKET
	PHALCON_INIT(Phalcon_Websocket_Connection);
	PHALCON_INIT(Phalcon_Websocket_Client);
	PHALCON_INIT(Phalcon_Websocket_Server);
#endif

#ifdef PHALCON_INTRUSIVE
	PHALCON_INIT(Phalcon_Intrusive_Avltree);
	PHALCON_INIT(Phalcon_Intrusive_Avltree_Node);
	PHALCON_INIT(Phalcon_Intrusive_Rbtree);
	PHALCON_INIT(Phalcon_Intrusive_Rbtree_Node);
#endif

#ifdef PHALCON_STORAGE_BTREE
	PHALCON_INIT(Phalcon_Storage_Btree);
#endif

#if PHALCON_USE_WIREDTIGER
	PHALCON_INIT(Phalcon_Storage_Wiredtiger);
	PHALCON_INIT(Phalcon_Storage_Wiredtiger_Cursor);
#endif

#if PHALCON_USE_LMDB
	PHALCON_INIT(Phalcon_Storage_Lmdb);
	PHALCON_INIT(Phalcon_Storage_Lmdb_Cursor);
#endif
#if PHALCON_USE_LIBMDBX
	PHALCON_INIT(Phalcon_Storage_Libmdbx);
	PHALCON_INIT(Phalcon_Storage_Libmdbx_Cursor);
#endif

#if PHALCON_USE_LEVELDB
	PHALCON_INIT(Phalcon_Storage_Leveldb);
	PHALCON_INIT(Phalcon_Storage_Leveldb_Iterator);
	PHALCON_INIT(Phalcon_Storage_Leveldb_Writebatch);
#endif

#if PHALCON_USE_BLOOMFILTER
	PHALCON_INIT(Phalcon_Storage_Bloomfilter);
# ifdef ZEND_ENABLE_ZVAL_LONG64
	PHALCON_INIT(Phalcon_Storage_Bloomfilter_Counting);
# endif
#endif

#ifdef PHALCON_USE_DATRIE
	PHALCON_INIT(Phalcon_Storage_Datrie);
#endif

	PHALCON_INIT(Phalcon_Snowflake);

#if PHALCON_USE_SERVER
	PHALCON_INIT(Phalcon_Server);
	PHALCON_INIT(Phalcon_Server_Http);
#endif
	PHALCON_INIT(Phalcon_Server_Simple);

#if PHALCON_USE_PYTHON
	PHALCON_INIT(Phalcon_Py);
	PHALCON_INIT(Phalcon_Py_Object);
	PHALCON_INIT(Phalcon_Py_Matplot);
#endif

	PHALCON_INIT(Phalcon_Aop);
	PHALCON_INIT(Phalcon_Aop_Joinpoint);

	return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(phalcon){

	phalcon_deinitialize_memory();

	assert(PHALCON_GLOBAL(orm).ast_cache == NULL);
#ifdef PHALCON_CACHE_YAC
	if (PHALCON_GLOBAL(cache).enable_yac) {
		phalcon_cache_yac_storage_shutdown();
	}
#endif

#ifdef PHALCON_USE_MONGOC
	mongoc_cleanup();
#endif

#if PHALCON_USE_PYTHON
	PyEval_RestoreThread(PHALCON_GLOBAL(python).mtstate);
	Py_Finalize();
#endif

#if PHALCON_USE_UV
	async_task_ce_unregister();
#endif

	UNREGISTER_INI_ENTRIES();

#if PHALCON_USE_UV
	if (ASYNC_G(cli)) {
		uv_tty_reset_mode();
	}

	zend_execute_ex = async_orig_execute_ex;
#endif

	return SUCCESS;
}

static PHP_RINIT_FUNCTION(phalcon){

	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;

	php_phalcon_init_globals(phalcon_globals_ptr);
	phalcon_init_interned_strings();

	phalcon_initialize_memory(phalcon_globals_ptr);

	if (PHALCON_GLOBAL(aop).enable_aop) {
		PHALCON_GLOBAL(aop).overloaded = 0;
		PHALCON_GLOBAL(aop).pointcut_version = 0;

		PHALCON_GLOBAL(aop).object_cache_size = 1024;
		PHALCON_GLOBAL(aop).object_cache = ecalloc(1024, sizeof(phalcon_aop_object_cache*));

		PHALCON_GLOBAL(aop).property_value = NULL;

		PHALCON_GLOBAL(aop).lock_read_property = 0;
		PHALCON_GLOBAL(aop).lock_write_property = 0;

		ALLOC_HASHTABLE(PHALCON_GLOBAL(aop).pointcuts_table);
		zend_hash_init(PHALCON_GLOBAL(aop).pointcuts_table, 16, NULL, phalcon_aop_free_pointcut, 0);

		ALLOC_HASHTABLE(PHALCON_GLOBAL(aop).function_cache);
		zend_hash_init(PHALCON_GLOBAL(aop).function_cache, 16, NULL, phalcon_aop_free_pointcut_cache, 0);
	}

	if (PHALCON_GLOBAL(xhprof).enable_xhprof) {
		tracing_request_init();
		tracing_determine_clock_source();
	}

#if PHALCON_USE_PYTHON
	PHALCON_GLOBAL(python).tstate = interpreter_python_init_thread();
#endif

#if PHALCON_USE_UV
	async_context_init();
	async_task_scheduler_init();
	
	if (ASYNC_G(timer_enabled)) {
		async_timer_init();
	}
	
	async_filesystem_init();
	async_tcp_socket_init();
	async_udp_socket_init();
#endif
	return SUCCESS;
}

static PHP_RSHUTDOWN_FUNCTION(phalcon){

	if (PHALCON_GLOBAL(aop).enable_aop) {
		int i;
		zend_array_destroy(PHALCON_GLOBAL(aop).pointcuts_table);
		zend_array_destroy(PHALCON_GLOBAL(aop).function_cache);
		for (i = 0; i < PHALCON_GLOBAL(aop).object_cache_size; i++) {
			if (PHALCON_GLOBAL(aop).object_cache[i] != NULL) {
				phalcon_aop_object_cache *_cache = PHALCON_GLOBAL(aop).object_cache[i];
				if (_cache->write!=NULL) {
					zend_hash_destroy(_cache->write);
					FREE_HASHTABLE(_cache->write);
				}
				if (_cache->read!=NULL) {
					zend_hash_destroy(_cache->read);
					FREE_HASHTABLE(_cache->read);
				}
				if (_cache->func!=NULL) {
					zend_hash_destroy(_cache->func);
					FREE_HASHTABLE(_cache->func);
				}
				efree(_cache);
			}
		}
		efree(PHALCON_GLOBAL(aop).object_cache);

		if (PHALCON_GLOBAL(aop).property_value != NULL) {
			zval_ptr_dtor(PHALCON_GLOBAL(aop).property_value);
			efree(PHALCON_GLOBAL(aop).property_value);
		}
	}

	if (PHALCON_GLOBAL(xhprof).enable_xhprof) {
		int i = 0;
		xhprof_callgraph_bucket *bucket;

		tracing_end();

		for (i = 0; i < PHALCON_XHPROF_CALLGRAPH_SLOTS; i++) {
			bucket = PHALCON_GLOBAL(xhprof).callgraph_buckets[i];

			while (bucket) {
				PHALCON_GLOBAL(xhprof).callgraph_buckets[i] = bucket->next;
				tracing_callgraph_bucket_free(bucket);
				bucket = PHALCON_GLOBAL(xhprof).callgraph_buckets[i];
			}
		}

		tracing_request_shutdown();
	}

	phalcon_deinitialize_memory();
	phalcon_release_interned_strings();

#if PHALCON_USE_PYTHON
	interpreter_python_shutdown_thread(PHALCON_GLOBAL(python).tstate);
#endif

#if PHALCON_USE_UV
	if (ASYNC_G(dns_enabled)) {
		async_dns_shutdown();
	}

	if (ASYNC_G(timer_enabled)) {
		async_timer_shutdown();
	}

	async_filesystem_shutdown();
	async_tcp_socket_shutdown();
	async_udp_socket_shutdown();

	async_task_scheduler_shutdown();
	async_context_shutdown();
#endif
	return SUCCESS;
}

static PHP_MINFO_FUNCTION(phalcon)
{
#ifdef PHALCON_USE_UV
	char uv_version[20];
#endif
	php_info_print_table_start();
	php_info_print_table_row(2, "Phalcon7 Framework", "enabled");
	php_info_print_table_row(2, "Phalcon7 Version", PHP_PHALCON_VERSION);
	php_info_print_table_row(2, "Build Date", __DATE__ " " __TIME__ );
#ifdef PHALCON_USE_UV
	sprintf(uv_version, "%d.%d", UV_VERSION_MAJOR, UV_VERSION_MINOR);
	php_info_print_table_row(2, "Libuv version", uv_version);
#endif
	if (PHALCON_GLOBAL(aop).enable_aop) {
		php_info_print_table_header(2, "AOP support", "enabled");
	}

	if (PHALCON_GLOBAL(xhprof).enable_xhprof) {
		php_info_print_table_row(2, "Xhprof", "enabled");
		php_info_print_table_row(2, "Function Nesting Limit", "enabled");
		switch (PHALCON_GLOBAL(xhprof).clock_source) {
			case PHALCON_CLOCK_TSC:
				php_info_print_table_row(2, "Clock Source", "tsc");
				break;
			case PHALCON_CLOCK_CGT:
				php_info_print_table_row(2, "Clock Source", "clock_gettime");
				break;
			case PHALCON_CLOCK_GTOD:
				php_info_print_table_row(2, "Clock Source", "gettimeofday");
				break;
			case PHALCON_CLOCK_MACH:
				php_info_print_table_row(2, "Clock Source", "mach");
				break;
			case PHALCON_CLOCK_QPC:
				php_info_print_table_row(2, "Clock Source", "Query Performance Counter");
				break;
			case PHALCON_CLOCK_NONE:
				php_info_print_table_row(2, "Clock Source", "none");
				break;
		}
	}

#ifdef PHALCON_CACHE_YAC
	php_info_print_table_row(2, "Cache Yac", "enabled");
#endif

#ifdef PHALCON_USE_MONGOC
	php_info_print_table_row(2, "Cache Backend Mongo", "enabled");
#endif

#ifdef PHALCON_CHART
	php_info_print_table_row(2, "Chart Captcha", "enabled");
#endif

#if PHALCON_USE_QRENCODE
	php_info_print_table_row(2, "Chart QRcode", "enabled");
#endif

#ifdef PHALCON_PROCESS
	php_info_print_table_row(2, "Process", "enabled");
#endif

	php_info_print_table_row(2, "Socket", "enabled");

#if PHALCON_USE_WEBSOCKET
	php_info_print_table_row(2, "Websocket", "enabled");
#endif

#if PHALCON_USE_SHM_OPEN
	php_info_print_table_row(2, "Sync", "enabled");
#endif

#ifdef PHALCON_INTRUSIVE
	php_info_print_table_row(2, "Intrusive", "enabled");
#endif

#ifdef PHALCON_STORAGE_BTREE
	php_info_print_table_row(2, "Storage B+Tree", "enabled");
#endif

#if PHALCON_USE_WIREDTIGER
	php_info_print_table_row(2, "Storage Wiredtiger", "enabled");
#endif

#if PHALCON_USE_LEVELDB
	php_info_print_table_row(2, "Storage Leveldb", "enabled");
#endif

#if PHALCON_USE_LMDB
	php_info_print_table_row(2, "Storage Lmdb", "enabled");
#endif

#if PHALCON_USE_LIBMDBX
	php_info_print_table_row(2, "Storage Libmdbx", "enabled");
#endif

#if PHALCON_USE_BLOOMFILTER
	php_info_print_table_row(2, "Storage Bloomfilter", "enabled");
#endif

#if PHALCON_USE_DATRIE
	php_info_print_table_row(2, "Storage Datrie", "enabled");
#endif

#if PHALCON_USE_SERVER
	php_info_print_table_row(2, "Server", "enabled");
#endif

#if PHALCON_USE_PYTHON
	php_info_print_table_row(2, "Python", "enabled");
#endif

	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}

static PHP_GINIT_FUNCTION(phalcon)
{
	memset(phalcon_globals, 0, sizeof(zend_phalcon_globals));
	php_phalcon_init_globals(phalcon_globals);

#ifdef PHALCON_CACHE_YAC
	/* Cache options */
	phalcon_globals->cache.enable_yac = 1;
	phalcon_globals->cache.enable_yac_cli = 0;
	phalcon_globals->cache.yac_keys_size = (4 * 1024 * 1024);
	phalcon_globals->cache.yac_values_size = (64 * 1024 * 1024);
#endif
	phalcon_globals->xhprof.root = NULL;
	phalcon_globals->xhprof.callgraph_frames = NULL;
	phalcon_globals->xhprof.frame_free_list = NULL;
	phalcon_globals->xhprof.nesting_current_level = -1;
	phalcon_globals->xhprof.nesting_maximum_level =  0;

	phalcon_globals->snowflake.epoch = 1420864633000ULL;
}

static PHP_GSHUTDOWN_FUNCTION(phalcon)
{
	phalcon_deinitialize_memory();
}

static const zend_module_dep phalcon_deps[] = {
	ZEND_MOD_REQUIRED("spl")
	ZEND_MOD_REQUIRED("date")
#if PHALCON_USE_PHP_JSON
	ZEND_MOD_REQUIRED("json")
#else
	ZEND_MOD_OPTIONAL("json")
#endif
#if PHALCON_USE_PHP_SESSION
	ZEND_MOD_REQUIRED("session")
#else
	ZEND_MOD_OPTIONAL("session")
#endif
#if PHALCON_USE_PHP_PCRE
	ZEND_MOD_REQUIRED("pcre")
#else
	ZEND_MOD_OPTIONAL("pcre")
#endif
#if PHALCON_USE_PHP_HASH
	ZEND_MOD_REQUIRED("hash")
#else
	ZEND_MOD_OPTIONAL("hash")
#endif

	ZEND_MOD_OPTIONAL("sockets")

#if PHALCON_USE_PHP_MBSTRING
	ZEND_MOD_REQUIRED("mbstring")
#else
	ZEND_MOD_OPTIONAL("mbstring")
#endif
	ZEND_MOD_OPTIONAL("apc")
	ZEND_MOD_OPTIONAL("apcu")
	ZEND_MOD_OPTIONAL("memcached")
	ZEND_MOD_OPTIONAL("filter")
	ZEND_MOD_OPTIONAL("iconv")
	ZEND_MOD_OPTIONAL("libxml")
	ZEND_MOD_OPTIONAL("mbstring")
	ZEND_MOD_OPTIONAL("openssl")
	ZEND_MOD_OPTIONAL("pdo")
	ZEND_MOD_OPTIONAL("gd")
	ZEND_MOD_OPTIONAL("imagick")
	ZEND_MOD_END
};

zend_module_entry phalcon_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	phalcon_deps,
	PHP_PHALCON_EXTNAME,
	NULL,
	PHP_MINIT(phalcon),
	PHP_MSHUTDOWN(phalcon),
	PHP_RINIT(phalcon),
	PHP_RSHUTDOWN(phalcon),
	PHP_MINFO(phalcon),
	PHP_PHALCON_VERSION,
	ZEND_MODULE_GLOBALS(phalcon),
	PHP_GINIT(phalcon),
	PHP_GSHUTDOWN(phalcon),
	NULL, /* ZEND_MODULE_POST_ZEND_DEACTIVATE_N(phalcon), */
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_PHALCON
ZEND_GET_MODULE(phalcon)
#endif
