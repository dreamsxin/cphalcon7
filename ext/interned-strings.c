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
  |          Vladimir Kolesnikov <vladimir@extrememember.com>              |
  +------------------------------------------------------------------------+
*/

#include "php_phalcon.h"
#include "interned-strings.h"
#include "kernel/main.h"
#include "kernel/string.h"

zend_string *phalcon_interned_DELETE					= NULL;
zend_string *phalcon_interned_GET					= NULL;
zend_string *phalcon_interned_HEAD					= NULL;
zend_string *phalcon_interned_OPTIONS				= NULL;
zend_string *phalcon_interned_PATCH					= NULL;
zend_string *phalcon_interned_POST					= NULL;
zend_string *phalcon_interned_PUT					= NULL;
zend_string *phalcon_interned_action					= NULL;
zend_string *phalcon_interned_alias					= NULL;
zend_string *phalcon_interned_all					= NULL;
zend_string *phalcon_interned_allowEmpty				= NULL;
zend_string *phalcon_interned_arguments				= NULL;
zend_string *phalcon_interned_balias					= NULL;
zend_string *phalcon_interned_binary_op				= NULL;
zend_string *phalcon_interned_code					= NULL;
zend_string *phalcon_interned_column					= NULL;
zend_string *phalcon_interned_columns				= NULL;
zend_string *phalcon_interned_conditions				= NULL;
zend_string *phalcon_interned_controller				= NULL;
zend_string *phalcon_interned_delete					= NULL;
zend_string *phalcon_interned_dispatcher				= NULL;
zend_string *phalcon_interned_distinct				= NULL;
zend_string *phalcon_interned_domain					= NULL;
zend_string *phalcon_interned_escaper				= NULL;
zend_string *phalcon_interned_expr					= NULL;
zend_string *phalcon_interned_fields					= NULL;
zend_string *phalcon_interned_file					= NULL;
zend_string *phalcon_interned_filter					= NULL;
zend_string *phalcon_interned_functionCall			= NULL;
zend_string *phalcon_interned_group					= NULL;
zend_string *phalcon_interned_groupBy				= NULL;
zend_string *phalcon_interned_having					= NULL;
zend_string *phalcon_interned_items					= NULL;
zend_string *phalcon_interned_joins					= NULL;
zend_string *phalcon_interned_label					= NULL;
zend_string *phalcon_interned_left					= NULL;
zend_string *phalcon_interned_limit					= NULL;
zend_string *phalcon_interned_forupdate				= NULL;
zend_string *phalcon_interned_line					= NULL;
zend_string *phalcon_interned_message				= NULL;
zend_string *phalcon_interned_model					= NULL;
zend_string *phalcon_interned_models					= NULL;
zend_string *phalcon_interned_modelsCache			= NULL;
zend_string *phalcon_interned_modelsManager			= NULL;
zend_string *phalcon_interned_modelsMetadata			= NULL;
zend_string *phalcon_interned_modelsQuery			= NULL;
zend_string *phalcon_interned_modelsQueryBuilder		= NULL;
zend_string *phalcon_interned_modelsCriteria			= NULL;
zend_string *phalcon_interned_modelsResultsetSimple	= NULL;
zend_string *phalcon_interned_modelsResultsetComplex	= NULL;
zend_string *phalcon_interned_module					= NULL;
zend_string *phalcon_interned_name					= NULL;
zend_string *phalcon_interned_namespace				= NULL;
zend_string *phalcon_interned_ns_alias				= NULL;
zend_string *phalcon_interned_number					= NULL;
zend_string *phalcon_interned_offset					= NULL;
zend_string *phalcon_interned_op						= NULL;
zend_string *phalcon_interned_order					= NULL;
zend_string *phalcon_interned_orderBy				= NULL;
zend_string *phalcon_interned_params					= NULL;
zend_string *phalcon_interned_parent					= NULL;
zend_string *phalcon_interned_paths					= NULL;
zend_string *phalcon_interned_qualified				= NULL;
zend_string *phalcon_interned_qualifiedName			= NULL;
zend_string *phalcon_interned_request				= NULL;
zend_string *phalcon_interned_response				= NULL;
zend_string *phalcon_interned_right					= NULL;
zend_string *phalcon_interned_router					= NULL;
zend_string *phalcon_interned_select					= NULL;
zend_string *phalcon_interned_self					= NULL;
zend_string *phalcon_interned_session				= NULL;
zend_string *phalcon_interned_sort					= NULL;
zend_string *phalcon_interned_source					= NULL;
zend_string *phalcon_interned_static					= NULL;
zend_string *phalcon_interned_sqlAlias				= NULL;
zend_string *phalcon_interned_table					= NULL;
zend_string *phalcon_interned_tables					= NULL;
zend_string *phalcon_interned_type					= NULL;
zend_string *phalcon_interned_update					= NULL;
zend_string *phalcon_interned_url					= NULL;
zend_string *phalcon_interned_value					= NULL;
zend_string *phalcon_interned_values					= NULL;
zend_string *phalcon_interned_where					= NULL;
zend_string *phalcon_interned_cookies				= NULL;
zend_string *phalcon_interned_annotations			= NULL;
zend_string *phalcon_interned_security				= NULL;
zend_string *phalcon_interned_crypt					= NULL;
zend_string *phalcon_interned_flash					= NULL;
zend_string *phalcon_interned_flashSession			= NULL;
zend_string *phalcon_interned_tag					= NULL;
zend_string *phalcon_interned_sessionBag				= NULL;
zend_string *phalcon_interned_eventsManager			= NULL;
zend_string *phalcon_interned_transactionManager		= NULL;
zend_string *phalcon_interned_assets					= NULL;
zend_string *phalcon_interned_rows					= NULL;
zend_string *phalcon_interned_view					= NULL;

PHALCON_STATIC void phalcon_init_interned_strings()
{
	phalcon_interned_DELETE						= zend_new_interned_string(SSL("DELETE"));
	phalcon_interned_GET						= zend_new_interned_string(SSL("GET"));
	phalcon_interned_HEAD						= zend_new_interned_string(SSL("HEAD"));
	phalcon_interned_OPTIONS					= zend_new_interned_string(SSL("OPTIONS"));
	phalcon_interned_PATCH						= zend_new_interned_string(SSL("PATCH"));
	phalcon_interned_POST						= zend_new_interned_string(SSL("POST"));
	phalcon_interned_PUT						= zend_new_interned_string(SSL("PUT"));
	phalcon_interned_action						= zend_new_interned_string(SSL("action"));
	phalcon_interned_alias						= zend_new_interned_string(SSL("alias"));
	phalcon_interned_all						= zend_new_interned_string(SSL("all"));
	phalcon_interned_allowEmpty					= zend_new_interned_string(SSL("allowEmpty"));
	phalcon_interned_arguments					= zend_new_interned_string(SSL("arguments"));
	phalcon_interned_balias						= zend_new_interned_string(SSL("balias"));
	phalcon_interned_binary_op					= zend_new_interned_string(SSL("binary-op"));
	phalcon_interned_code						= zend_new_interned_string(SSL("code"));
	phalcon_interned_column						= zend_new_interned_string(SSL("column"));
	phalcon_interned_columns					= zend_new_interned_string(SSL("columns"));
	phalcon_interned_conditions					= zend_new_interned_string(SSL("conditions"));
	phalcon_interned_controller					= zend_new_interned_string(SSL("controller"));
	phalcon_interned_delete						= zend_new_interned_string(SSL("delete"));
	phalcon_interned_dispatcher					= zend_new_interned_string(SSL("dispatcher"));
	phalcon_interned_distinct					= zend_new_interned_string(SSL("distinct"));
	phalcon_interned_domain						= zend_new_interned_string(SSL("domain"));
	phalcon_interned_escaper					= zend_new_interned_string(SSL("escaper"));
	phalcon_interned_expr						= zend_new_interned_string(SSL("expr"));
	phalcon_interned_fields						= zend_new_interned_string(SSL("fields"));
	phalcon_interned_file						= zend_new_interned_string(SSL("file"));
	phalcon_interned_filter						= zend_new_interned_string(SSL("filter"));
	phalcon_interned_functionCall				= zend_new_interned_string(SSL("functionCall"));
	phalcon_interned_group						= zend_new_interned_string(SSL("group"));
	phalcon_interned_groupBy					= zend_new_interned_string(SSL("groupBy"));
	phalcon_interned_having						= zend_new_interned_string(SSL("having"));
	phalcon_interned_items						= zend_new_interned_string(SSL("items"));
	phalcon_interned_joins						= zend_new_interned_string(SSL("joins"));
	phalcon_interned_label						= zend_new_interned_string(SSL("label"));
	phalcon_interned_left						= zend_new_interned_string(SSL("left"));
	phalcon_interned_limit						= zend_new_interned_string(SSL("limit"));
	phalcon_interned_forupdate					= zend_new_interned_string(SSL("forupdate"));
	phalcon_interned_line						= zend_new_interned_string(SSL("line"));
	phalcon_interned_message					= zend_new_interned_string(SSL("message"));
	phalcon_interned_model						= zend_new_interned_string(SSL("model"));
	phalcon_interned_models						= zend_new_interned_string(SSL("models"));
	phalcon_interned_modelsCache				= zend_new_interned_string(SSL("modelsCache"));
	phalcon_interned_modelsManager				= zend_new_interned_string(SSL("modelsManager"));
	phalcon_interned_modelsMetadata				= zend_new_interned_string(SSL("modelsMetadata"));
	phalcon_interned_modelsQuery				= zend_new_interned_string(SSL("modelsQuery"));
	phalcon_interned_modelsQueryBuilder			= zend_new_interned_string(SSL("modelsQueryBuilder"));
	phalcon_interned_modelsCriteria				= zend_new_interned_string(SSL("modelsCriteria"));
	phalcon_interned_modelsResultsetSimple		= zend_new_interned_string(SSL("modelsResultsetSimple"));
	phalcon_interned_modelsResultsetComplex		= zend_new_interned_string(SSL("modelsResultsetComplex"));
	phalcon_interned_module						= zend_new_interned_string(SSL("module"));
	phalcon_interned_name						= zend_new_interned_string(SSL("name"));
	phalcon_interned_namespace					= zend_new_interned_string(SSL("namespace"));
	phalcon_interned_ns_alias					= zend_new_interned_string(SSL("ns-alias"));
	phalcon_interned_number						= zend_new_interned_string(SSL("number"));
	phalcon_interned_offset						= zend_new_interned_string(SSL("offset"));
	phalcon_interned_op							= zend_new_interned_string(SSL("op"));
	phalcon_interned_order						= zend_new_interned_string(SSL("order"));
	phalcon_interned_orderBy					= zend_new_interned_string(SSL("orderBy"));
	phalcon_interned_params						= zend_new_interned_string(SSL("params"));
	phalcon_interned_parent						= zend_new_interned_string(SSL("parent"));
	phalcon_interned_paths						= zend_new_interned_string(SSL("paths"));
	phalcon_interned_qualified					= zend_new_interned_string(SSL("qualified"));
	phalcon_interned_qualifiedName				= zend_new_interned_string(SSL("qualifiedName"));
	phalcon_interned_request					= zend_new_interned_string(SSL("request"));
	phalcon_interned_response					= zend_new_interned_string(SSL("response"));
	phalcon_interned_right						= zend_new_interned_string(SSL("right"));
	phalcon_interned_router						= zend_new_interned_string(SSL("router"));
	phalcon_interned_select						= zend_new_interned_string(SSL("select"));
	phalcon_interned_self						= zend_new_interned_string(SSL("self"));
	phalcon_interned_session					= zend_new_interned_string(SSL("session"));
	phalcon_interned_sort						= zend_new_interned_string(SSL("sort"));
	phalcon_interned_source						= zend_new_interned_string(SSL("source"));
	phalcon_interned_static						= zend_new_interned_string(SSL("static"));
	phalcon_interned_sqlAlias					= zend_new_interned_string(SSL("sqlAlias"));
	phalcon_interned_table						= zend_new_interned_string(SSL("table"));
	phalcon_interned_tables						= zend_new_interned_string(SSL("tables"));
	phalcon_interned_type						= zend_new_interned_string(SSL("type"));
	phalcon_interned_update						= zend_new_interned_string(SSL("update"));
	phalcon_interned_url						= zend_new_interned_string(SSL("url"));
	phalcon_interned_value						= zend_new_interned_string(SSL("value"));
	phalcon_interned_values						= zend_new_interned_string(SSL("values"));
	phalcon_interned_where						= zend_new_interned_string(SSL("where"));
	phalcon_interned_rows						= zend_new_interned_string(SSL("rows"));
	phalcon_interned_cookies					= zend_new_interned_string(SSL("cookies"));
	phalcon_interned_annotations				= zend_new_interned_string(SSL("annotations"));
	phalcon_interned_security					= zend_new_interned_string(SSL("security"));
	phalcon_interned_crypt						= zend_new_interned_string(SSL("crypt"));
	phalcon_interned_flash						= zend_new_interned_string(SSL("flash"));
	phalcon_interned_flashSession				= zend_new_interned_string(SSL("flashSession"));
	phalcon_interned_tag						= zend_new_interned_string(SSL("tag"));
	phalcon_interned_sessionBag					= zend_new_interned_string(SSL("sessionBag"));
	phalcon_interned_eventsManager				= zend_new_interned_string(SSL("eventsManager"));
	phalcon_interned_transactionManager			= zend_new_interned_string(SSL("transactions"));
	phalcon_interned_assets						= zend_new_interned_string(SSL("assets"));
	phalcon_interned_view						= zend_new_interned_string(SSL("view"));
}
