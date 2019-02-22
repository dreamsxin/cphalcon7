
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

#include "mvc/model/transaction.h"
#include "mvc/model/transactioninterface.h"
#include "mvc/model/transaction/exception.h"
#include "mvc/model/transaction/failed.h"
#include "mvc/model/transaction/managerinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/array.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Model\Transaction
 *
 * Transactions are protective blocks where SQL statements are only permanent if they can
 * all succeed as one atomic action. Phalcon\Transaction is intended to be used with Phalcon_Model_Base.
 * Phalcon Transactions should be created using Phalcon\Transaction\Manager.
 *
 *<code>
 *try {
 *
 *  $manager = new Phalcon\Mvc\Model\Transaction\Manager();
 *
 *  $transaction = $manager->get();
 *
 *  $robot = new Robots();
 *  $robot->setTransaction($transaction);
 *  $robot->name = 'WALL·E';
 *  $robot->created_at = date('Y-m-d');
 *  if ($robot->save() == false) {
 *    $transaction->rollback("Can't save robot");
 *  }
 *
 *  $robotPart = new RobotParts();
 *  $robotPart->setTransaction($transaction);
 *  $robotPart->type = 'head';
 *  if ($robotPart->save() == false) {
 *    $transaction->rollback("Can't save robot part");
 *  }
 *
 *  $transaction->commit();
 *
 *} catch(Phalcon\Mvc\Model\Transaction\Failed $e) {
 *  echo 'Failed, reason: ', $e->getMessage();
 *}
 *
 *</code>
 */
zend_class_entry *phalcon_mvc_model_transaction_ce;

PHP_METHOD(Phalcon_Mvc_Model_Transaction, __construct);
PHP_METHOD(Phalcon_Mvc_Model_Transaction, setTransactionManager);
PHP_METHOD(Phalcon_Mvc_Model_Transaction, begin);
PHP_METHOD(Phalcon_Mvc_Model_Transaction, commit);
PHP_METHOD(Phalcon_Mvc_Model_Transaction, rollback);
PHP_METHOD(Phalcon_Mvc_Model_Transaction, getConnection);
PHP_METHOD(Phalcon_Mvc_Model_Transaction, setIsNewTransaction);
PHP_METHOD(Phalcon_Mvc_Model_Transaction, setRollbackOnAbort);
PHP_METHOD(Phalcon_Mvc_Model_Transaction, isManaged);
PHP_METHOD(Phalcon_Mvc_Model_Transaction, getMessages);
PHP_METHOD(Phalcon_Mvc_Model_Transaction, isValid);
PHP_METHOD(Phalcon_Mvc_Model_Transaction, setRollbackedRecord);
PHP_METHOD(Phalcon_Mvc_Model_Transaction, getRollbackedRecord);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_transaction___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, dependencyInjector)
	ZEND_ARG_INFO(0, autoBegin)
	ZEND_ARG_INFO(0, service)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_transaction_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Transaction, __construct, arginfo_phalcon_mvc_model_transaction___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_Transaction, setTransactionManager, arginfo_phalcon_mvc_model_transactioninterface_settransactionmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Transaction, begin, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Transaction, commit, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Transaction, rollback, arginfo_phalcon_mvc_model_transactioninterface_rollback, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Transaction, getConnection, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Transaction, setIsNewTransaction, arginfo_phalcon_mvc_model_transactioninterface_setisnewtransaction, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Transaction, setRollbackOnAbort, arginfo_phalcon_mvc_model_transactioninterface_setrollbackonabort, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Transaction, isManaged, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Transaction, getMessages, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Transaction, isValid, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Transaction, setRollbackedRecord, arginfo_phalcon_mvc_model_transactioninterface_setrollbackedrecord, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Transaction, getRollbackedRecord, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Transaction initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Transaction){

	PHALCON_REGISTER_CLASS(Phalcon\\Mvc\\Model, Transaction, mvc_model_transaction, phalcon_mvc_model_transaction_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_model_transaction_ce, SL("_connection"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_model_transaction_ce, SL("_activeTransaction"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_model_transaction_ce, SL("_isNewTransaction"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_model_transaction_ce, SL("_rollbackOnAbort"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_transaction_ce, SL("_manager"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_transaction_ce, SL("_messages"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_transaction_ce, SL("_rollbackRecord"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_transaction_ce, 1, phalcon_mvc_model_transactioninterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\Transaction constructor
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @param boolean $autoBegin
 * @param string $service
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction, __construct){

	zval *dependency_injector, *auto_begin = NULL, *s = NULL, service = {}, connection = {};

	phalcon_fetch_params(0, 1, 2, &dependency_injector, &auto_begin, &s);

	if (!auto_begin) {
		auto_begin = &PHALCON_GLOBAL(z_false);
	}

	if (!s || Z_TYPE_P(s) != IS_STRING) {
		ZVAL_STR(&service, IS(db));
	} else {
		ZVAL_COPY(&service, s);
	}

	if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_transaction_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}

	PHALCON_CALL_METHOD(&connection, dependency_injector, "get", &service);
	zval_ptr_dtor(&service);

	phalcon_update_property(getThis(), SL("_connection"), &connection);
	if (zend_is_true(auto_begin)) {
		PHALCON_CALL_METHOD(NULL, &connection, "begin");
	}
	zval_ptr_dtor(&connection);
}

/**
 * Sets transaction manager related to the transaction
 *
 * @param Phalcon\Mvc\Model\Transaction\ManagerInterface $manager
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction, setTransactionManager){

	zval *manager;

	phalcon_fetch_params(0, 1, 0, &manager);
	PHALCON_VERIFY_INTERFACE_EX(manager, phalcon_mvc_model_transaction_managerinterface_ce, phalcon_mvc_model_transaction_exception_ce);

	phalcon_update_property(getThis(), SL("_manager"), manager);
}

/**
 * Starts the transaction
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction, begin){

	zval connection = {};

	phalcon_read_property(&connection, getThis(), SL("_connection"), PH_NOISY|PH_READONLY);
	PHALCON_RETURN_CALL_METHOD(&connection, "begin");
}

/**
 * Commits the transaction
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction, commit){

	zval manager = {}, connection = {};

	phalcon_read_property(&manager, getThis(), SL("_manager"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(manager) == IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, &manager, "notifycommit", getThis());
	}

	phalcon_read_property(&connection, getThis(), SL("_connection"), PH_NOISY|PH_READONLY);
	PHALCON_RETURN_CALL_METHOD(&connection, "commit");
}

/**
 * Rolls back the transaction
 *
 * @param  string $rollbackMessage
 * @param  Phalcon\Mvc\ModelInterface $rollbackRecord
 * @param  int $rollbackCode
 * @param  boolean $noThrowError
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction, rollback){

	zval *message = NULL, *_rollback_record = NULL, *rollback_code = NULL, *nothrowerror = NULL, rollback_record = {}, rollback_message = {}, manager = {}, connection = {}, success = {}, i0 = {};

	phalcon_fetch_params(0, 0, 4, &message, &rollback_record, &rollback_code, &nothrowerror);

	if (!message || !zend_is_true(message)) {
		ZVAL_STRING(&rollback_message, "Transaction aborted");
	} else {
		ZVAL_COPY(&rollback_message, message);
	}

	if (_rollback_record && Z_TYPE_P(_rollback_record) != IS_OBJECT) {
		phalcon_update_property(getThis(), SL("_rollbackRecord"), _rollback_record);
	}

	phalcon_read_property(&rollback_record, getThis(), SL("_rollbackRecord"), PH_NOISY|PH_READONLY);

	if (!rollback_code) {
		rollback_code = &PHALCON_GLOBAL(z_zero);
	}

	if (!nothrowerror) {
		nothrowerror = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&manager, getThis(), SL("_manager"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(manager) == IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, &manager, "notifyrollback", getThis());
	}

	phalcon_read_property(&connection, getThis(), SL("_connection"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&success, &connection, "rollback");

	if (!zend_is_true(nothrowerror) && zend_is_true(&success)) {
		object_init_ex(&i0, phalcon_mvc_model_transaction_failed_ce);
		PHALCON_CALL_METHOD(NULL, &i0, "__construct", &rollback_message, &rollback_record, rollback_code);
		phalcon_throw_exception(&i0);
	}
	zval_ptr_dtor(&rollback_message);

	RETURN_CTOR(&success);
}

/**
 * Returns the connection related to transaction
 *
 * @return Phalcon\Db\AdapterInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction, getConnection){

	zval rollback_on_abort = {}, message = {}, connection = {};

	phalcon_read_property(&rollback_on_abort, getThis(), SL("_rollbackOnAbort"), PH_NOISY|PH_READONLY);
	if (zend_is_true(&rollback_on_abort)) {

		if (PG(connection_status) & PHP_CONNECTION_ABORTED) {
			ZVAL_STRING(&message, "The request was aborted");
			PHALCON_CALL_METHOD(NULL, getThis(), "rollback", &message);
			zval_ptr_dtor(&message);
		}
	}

	phalcon_read_property(&connection, getThis(), SL("_connection"), PH_NOISY|PH_READONLY);

	RETURN_CTOR(&connection);
}

/**
 * Sets if is a reused transaction or new once
 *
 * @param boolean $isNew
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction, setIsNewTransaction){

	zval *is_new;

	phalcon_fetch_params(0, 1, 0, &is_new);

	phalcon_update_property(getThis(), SL("_isNewTransaction"), is_new);

}

/**
 * Sets flag to rollback on abort the HTTP connection
 *
 * @param boolean $rollbackOnAbort
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction, setRollbackOnAbort){

	zval *rollback_on_abort;

	phalcon_fetch_params(0, 1, 0, &rollback_on_abort);

	phalcon_update_property(getThis(), SL("_rollbackOnAbort"), rollback_on_abort);

}

/**
 * Checks whether transaction is managed by a transaction manager
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction, isManaged){

	zval manager = {};

	phalcon_read_property(&manager, getThis(), SL("_manager"), PH_NOISY|PH_READONLY);
	boolean_not_function(return_value, &manager);
}

/**
 * Returns validations messages from last save try
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction, getMessages){


	RETURN_MEMBER(getThis(), "_messages");
}

/**
 * Checks whether internal connection is under an active transaction
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction, isValid){

	zval connection = {};

	phalcon_read_property(&connection, getThis(), SL("_connection"), PH_NOISY|PH_READONLY);
	PHALCON_RETURN_CALL_METHOD(&connection, "isundertransaction");
}

/**
 * Sets object which generates rollback action
 *
 * @param Phalcon\Mvc\ModelInterface $record
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction, setRollbackedRecord){

	zval *record;

	phalcon_fetch_params(0, 1, 0, &record);

	phalcon_update_property(getThis(), SL("_rollbackRecord"), record);

}

/**
 * Gets object which generates rollback action
 *
 * @return Phalcon\Mvc\ModelInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Transaction, getRollbackedRecord){


	RETURN_MEMBER(getThis(), "_rollbackRecord");

}
