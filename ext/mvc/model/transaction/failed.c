
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
  +------------------------------------------------------------------------+
*/

#include "mvc/model/transaction/failed.h"
#include "mvc/model/transaction/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/fcall.h"

/**
 * Phalcon\Mvc\Model\Transaction\Failed
 *
 * This class will be thrown to exit a try/catch block for isolated transactions
 */
zend_class_entry *phalcon_mvc_model_transaction_failed_ce;

PHP_METHOD(Phalcon_Mvc_Model_Transaction_Failed, __construct);
PHP_METHOD(Phalcon_Mvc_Model_Transaction_Failed, getRecordMessages);
PHP_METHOD(Phalcon_Mvc_Model_Transaction_Failed, getRecord);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_transaction_failed___construct, 0, 0, 2)
	ZEND_ARG_INFO(0, message)
	ZEND_ARG_INFO(0, record)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_transaction_failed_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Transaction_Failed, __construct, arginfo_phalcon_mvc_model_transaction_failed___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_Transaction_Failed, getRecordMessages, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Transaction_Failed, getRecord, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Transaction\Failed initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Transaction_Failed){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Transaction, Failed, mvc_model_transaction_failed, phalcon_mvc_model_transaction_exception_ce, phalcon_mvc_model_transaction_failed_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_model_transaction_failed_ce, SL("_record"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\Transaction\Failed constructor
 *
 * @param string $message
 * @param Phalcon\Mvc\ModelInterface $record
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction_Failed, __construct){

	zval *message, *record, *code = NULL;

	phalcon_fetch_params(0, 2, 1, &message, &record, &code);

	if (!code) {
		code = &PHALCON_GLOBAL(z_zero);
	}

	phalcon_update_property(getThis(), SL("_record"), record);

	PHALCON_CALL_PARENT(NULL, phalcon_mvc_model_transaction_failed_ce, getThis(), "__construct", message, code);
}

/**
 * Returns validation record messages which stop the transaction
 *
 * @return Phalcon\Mvc\Model\MessageInterface[]
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction_Failed, getRecordMessages)
{
	zval record = {};

	phalcon_read_property(&record, getThis(), SL("_record"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(record) != IS_NULL) {
		PHALCON_RETURN_CALL_METHOD(&record, "getmessages");
	}

	PHALCON_RETURN_CALL_METHOD(getThis(), "getmessage");
}

/**
 * Returns validation record messages which stop the transaction
 *
 * @return Phalcon\Mvc\ModelInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction_Failed, getRecord){


	RETURN_MEMBER(getThis(), "_record");
}
