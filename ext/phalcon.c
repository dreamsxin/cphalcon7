
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

#include "cache/shmemory/storage.h"
#include "cache/shmemory/allocator.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/mbstring.h"
#include "interned-strings.h"

#include "phalcon.h"

ZEND_DECLARE_MODULE_GLOBALS(phalcon)


static PHP_INI_MH(OnChangeKeysMemoryLimit) {
	if (new_value) {
		PHALCON_GLOBAL(cache).shmemory_keys_size = zend_atol(ZSTR_VAL(new_value), ZSTR_LEN(new_value));
	}
	return SUCCESS;
}

static PHP_INI_MH(OnChangeValsMemoryLimit) {
	if (new_value) {
		PHALCON_GLOBAL(cache).shmemory_values_size = zend_atol(ZSTR_VAL(new_value), ZSTR_LEN(new_value));
	}
	return SUCCESS;
}

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
	/* Enables/Disables throwing an exception if save fails */
	STD_PHP_INI_BOOLEAN("phalcon.orm.exception_on_failed_save", "0",    PHP_INI_ALL,    OnUpdateBool, orm.exception_on_failed_save, zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables literals in PHQL */
	STD_PHP_INI_BOOLEAN("phalcon.orm.enable_literals",          "1",    PHP_INI_ALL,    OnUpdateBool, orm.enable_literals,          zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables AST cache */
	STD_PHP_INI_BOOLEAN("phalcon.orm.enable_ast_cache",         "1",    PHP_INI_ALL,    OnUpdateBool, orm.enable_ast_cache,         zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables property method */
	STD_PHP_INI_BOOLEAN("phalcon.orm.enable_property_method",   "1",    PHP_INI_ALL,    OnUpdateBool, orm.enable_property_method,   zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables auto convert column value follow database data type */
	STD_PHP_INI_BOOLEAN("phalcon.orm.enable_auto_convert",      "1",    PHP_INI_ALL,    OnUpdateBool, orm.enable_auto_convert,      zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_BOOLEAN("phalcon.orm.allow_update_primary",     "0",    PHP_INI_ALL,    OnUpdateBool, orm.allow_update_primary,     zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_BOOLEAN("phalcon.orm.enable_strict",            "0",    PHP_INI_ALL,    OnUpdateBool, orm.enable_strict,            zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables auttomatic escape */
	STD_PHP_INI_BOOLEAN("phalcon.db.escape_identifiers",        "1",    PHP_INI_ALL,    OnUpdateBool, db.escape_identifiers,        zend_phalcon_globals, phalcon_globals)
	/* Enables/Disables cache memory */
	STD_PHP_INI_BOOLEAN("phalcon.cache.enable_shmemory",        "1",   PHP_INI_ALL,    OnUpdateBool, cache.enable_shmemory,          zend_phalcon_globals, phalcon_globals)
	STD_PHP_INI_BOOLEAN("phalcon.cache.enable_shmemory_cli",    "0",   PHP_INI_ALL,    OnUpdateBool, cache.enable_shmemory_cli,      zend_phalcon_globals, phalcon_globals)
    STD_PHP_INI_ENTRY("phalcon.cache.shmemory_keys_size",       "4M",  PHP_INI_SYSTEM, OnChangeKeysMemoryLimit, cache.shmemory_keys_size,       zend_phalcon_globals, phalcon_globals)
    STD_PHP_INI_ENTRY("phalcon.cache.shmemory_values_size",     "64M", PHP_INI_SYSTEM, OnChangeValsMemoryLimit, cache.shmemory_values_size,     zend_phalcon_globals, phalcon_globals)
PHP_INI_END()

static PHP_MINIT_FUNCTION(phalcon)
{
	REGISTER_INI_ENTRIES();

#ifdef PHALCON_CACHE_SHMEMORY
	if (!PHALCON_GLOBAL(cache).enable_shmemory_cli && !strcmp(sapi_module.name, "cli")) {
		PHALCON_GLOBAL(cache).enable_shmemory = 0;
	}

	if (PHALCON_GLOBAL(cache).enable_shmemory) {
		char *msg;
		if (PHALCON_GLOBAL(cache).shmemory_values_size < PHALCON_CACHE_SHMEMORY_SMM_SEGMENT_MIN_SIZE) {
			php_error(E_ERROR, "Shared memory values(values_memory_size) must be at least '%d'", PHALCON_CACHE_SHMEMORY_SMM_SEGMENT_MIN_SIZE);
			return FAILURE;
		}
		if (!phalcon_cache_shmemory_storage_startup(PHALCON_GLOBAL(cache).shmemory_keys_size, PHALCON_GLOBAL(cache).shmemory_values_size, &msg)) {
			php_error(E_ERROR, "Shared memory allocator startup failed at '%s': %s", msg, strerror(errno));
			return FAILURE;
		}
	}
#else
	PHALCON_GLOBAL(cache).enable_shmemory = 0;
#endif

#if PHALCON_USE_MONGOC
	mongoc_init();
#endif

	/* 1. Register exceptions */
	PHALCON_INIT(Phalcon_Exception);
	PHALCON_INIT(Phalcon_Debug_Exception);
	PHALCON_INIT(Phalcon_Acl_Exception);
	PHALCON_INIT(Phalcon_Annotations_Exception);
	PHALCON_INIT(Phalcon_Assets_Exception);
	PHALCON_INIT(Phalcon_Cache_Exception);
	PHALCON_INIT(Phalcon_Crypt_Exception);
	PHALCON_INIT(Phalcon_Db_Exception);
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
	PHALCON_INIT(Phalcon_Application_Exception);
	PHALCON_INIT(Phalcon_Cli_Console_Exception);
	PHALCON_INIT(Phalcon_Cli_Dispatcher_Exception);
	PHALCON_INIT(Phalcon_Cli_Router_Exception);
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
	PHALCON_INIT(Phalcon_Mvc_JsonRpc_Exception);
	PHALCON_INIT(Phalcon_JsonRpc_Client_Exception);
#ifdef PHALCON_CHART
	PHALCON_INIT(Phalcon_Chart_Exception);
#endif
	PHALCON_INIT(Phalcon_Binary_Exception);
#if PHALCON_USE_PHP_SOCKET
	PHALCON_INIT(Phalcon_Socket_Exception);
#endif
#ifdef PHALCON_PROCESS
	PHALCON_INIT(Phalcon_Process_Exception);
#endif
#ifdef PHALCON_STORAGE_BTREE
	PHALCON_INIT(Phalcon_Storage_Exception);
#endif

	/* 2. Register interfaces */
	PHALCON_INIT(Phalcon_DiInterface);
	PHALCON_INIT(Phalcon_Di_InjectionAwareInterface);
	PHALCON_INIT(Phalcon_Di_ServiceInterface);
	PHALCON_INIT(Phalcon_Events_EventInterface);
	PHALCON_INIT(Phalcon_Events_EventsAwareInterface);

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

	/* 4. Register everything else */
	PHALCON_INIT(Phalcon_Di);
	PHALCON_INIT(Phalcon_Di_Injectable);
	PHALCON_INIT(Phalcon_Di_FactoryDefault);
	PHALCON_INIT(Phalcon_Di_FactoryDefault_Cli);
	PHALCON_INIT(Phalcon_Di_Service);
	PHALCON_INIT(Phalcon_Di_Service_Builder);
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
#ifdef PHALCON_CACHE_SHMEMORY
	PHALCON_INIT(Phalcon_Cache_SHMemory);
#endif
	PHALCON_INIT(Phalcon_Cache_Backend);
	PHALCON_INIT(Phalcon_Cache_Frontend_Data);
	PHALCON_INIT(Phalcon_Cache_Multiple);
	PHALCON_INIT(Phalcon_Cache_Backend_Apc);
	PHALCON_INIT(Phalcon_Cache_Backend_File);
	PHALCON_INIT(Phalcon_Cache_Backend_Memory);
	PHALCON_INIT(Phalcon_Cache_Backend_Xcache);
#ifdef PHALCON_USE_MONGOC
	PHALCON_INIT(Phalcon_Cache_Backend_Mongo);
#endif
	PHALCON_INIT(Phalcon_Cache_Backend_Memcached);
	PHALCON_INIT(Phalcon_Cache_Backend_Redis);
	PHALCON_INIT(Phalcon_Cache_Frontend_Json);
	PHALCON_INIT(Phalcon_Cache_Frontend_Output);
	PHALCON_INIT(Phalcon_Cache_Frontend_None);
	PHALCON_INIT(Phalcon_Cache_Frontend_Base64);
	PHALCON_INIT(Phalcon_Cache_Frontend_Igbinary);
	PHALCON_INIT(Phalcon_Tag);
	PHALCON_INIT(Phalcon_Tag_Select);
	PHALCON_INIT(Phalcon_Paginator_Adapter_Model);
	PHALCON_INIT(Phalcon_Paginator_Adapter_NativeArray);
	PHALCON_INIT(Phalcon_Paginator_Adapter_QueryBuilder);
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
	PHALCON_INIT(Phalcon_Annotations_Adapter_Xcache);
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
	PHALCON_INIT(Phalcon_Logger_Adapter_Stream);
	PHALCON_INIT(Phalcon_Logger_Adapter_Syslog);
	PHALCON_INIT(Phalcon_Logger_Adapter_File);
	PHALCON_INIT(Phalcon_Logger_Adapter_Firephp);
	PHALCON_INIT(Phalcon_Logger_Formatter_Syslog);
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
	PHALCON_INIT(Phalcon_JsonRpc_Client);
	PHALCON_INIT(Phalcon_JsonRpc_Client_Response);
	PHALCON_INIT(Phalcon_Queue_Beanstalk);
	PHALCON_INIT(Phalcon_Queue_Beanstalk_Job);
	PHALCON_INIT(Phalcon_Application);
	PHALCON_INIT(Phalcon_Cli_Task);
	PHALCON_INIT(Phalcon_Cli_Router);
	PHALCON_INIT(Phalcon_Cli_Console);
	PHALCON_INIT(Phalcon_Cli_Dispatcher);
	PHALCON_INIT(Phalcon_Mvc_JsonRpc);
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
	PHALCON_INIT(Phalcon_Mvc_Model_MetaData_Xcache);
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

#ifdef PHALCON_CHART
# if PHALCON_USE_QRENCODE
	PHALCON_INIT(Phalcon_Chart_QRcode);
# endif
	PHALCON_INIT(Phalcon_Chart_Captcha);
#endif

	PHALCON_INIT(Phalcon_Async);
	PHALCON_INIT(Phalcon_Binary);
	PHALCON_INIT(Phalcon_Binary_Reader);
	PHALCON_INIT(Phalcon_Binary_Writer);

#ifdef PHALCON_PROCESS
	PHALCON_INIT(Phalcon_Process_Proc);
	PHALCON_INIT(Phalcon_Process_Sharedmemory);
#endif

#ifdef PHALCON_SOCKET
	#if PHALCON_USE_PHP_SOCKET
	PHALCON_INIT(Phalcon_Socket);
	PHALCON_INIT(Phalcon_Socket_Client);
	PHALCON_INIT(Phalcon_Socket_Server);
	#endif

	#ifdef PHALCON_USE_WEBSOCKET
		PHALCON_INIT(Phalcon_Websocket_Connection);
		PHALCON_INIT(Phalcon_Websocket_Client);
		PHALCON_INIT(Phalcon_Websocket_Server);
	#endif
#endif

#ifdef PHALCON_INTRUSIVE
	PHALCON_INIT(Phalcon_Intrusive_Avltree);
	PHALCON_INIT(Phalcon_Intrusive_Avltree_Node);
#endif

#ifdef PHALCON_STORAGE_BTREE
	PHALCON_INIT(Phalcon_Storage_Btree);
#endif
	return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(phalcon){

	phalcon_deinitialize_memory();

	assert(PHALCON_GLOBAL(orm).ast_cache == NULL);
#ifdef PHALCON_CACHE_SHMEMORY
	if (PHALCON_GLOBAL(cache).enable_shmemory) {
		phalcon_cache_shmemory_storage_shutdown();
	}
#endif

#ifdef PHALCON_USE_MONGOC
	mongoc_cleanup();
#endif

	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}

static PHP_RINIT_FUNCTION(phalcon){

	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;

	php_phalcon_init_globals(phalcon_globals_ptr);
	phalcon_init_interned_strings();

	phalcon_initialize_memory(phalcon_globals_ptr);

	return SUCCESS;
}

static PHP_RSHUTDOWN_FUNCTION(phalcon){
	phalcon_deinitialize_memory();
	phalcon_release_interned_strings();

	return SUCCESS;
}

static PHP_MINFO_FUNCTION(phalcon)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Phalcon7 Framework", "enabled");
	php_info_print_table_row(2, "Phalcon7 Version", PHP_PHALCON_VERSION);
	php_info_print_table_row(2, "Build Date", __DATE__ " " __TIME__ );
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}

static PHP_GINIT_FUNCTION(phalcon)
{
	php_phalcon_init_globals(phalcon_globals);
#ifdef PHALCON_CACHE_SHMEMORY
	/* Cache options */
	phalcon_globals->cache.enable_shmemory = 1;
	phalcon_globals->cache.enable_shmemory_cli = 0;
	phalcon_globals->cache.shmemory_keys_size = (4 * 1024 * 1024);
	phalcon_globals->cache.shmemory_values_size = (64 * 1024 * 1024);
#endif
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
#if PHALCON_USE_PHP_SOCKET
	ZEND_MOD_REQUIRED("sockets")
#else
	ZEND_MOD_OPTIONAL("sockets")
#endif
	ZEND_MOD_OPTIONAL("apc")
	ZEND_MOD_OPTIONAL("apcu")
	ZEND_MOD_OPTIONAL("XCache")
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
