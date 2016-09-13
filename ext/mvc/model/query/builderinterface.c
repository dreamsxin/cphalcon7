
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

#include "mvc/model/query/builderinterface.h"
#include "kernel/main.h"

zend_class_entry *phalcon_mvc_model_query_builderinterface_ce;

static const zend_function_entry phalcon_mvc_model_query_builderinterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_Mvc_Model_Query_BuilderInterface, getType, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_Model_Query_BuilderInterface, compile, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_Model_Query_BuilderInterface, getPhql, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_Model_Query_BuilderInterface, getQuery, NULL)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Query\BuilderInterface initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Query_BuilderInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon\\Mvc\\Model\\Query, BuilderInterface, mvc_model_query_builderinterface, phalcon_mvc_model_query_builderinterface_method_entry);

	return SUCCESS;
}

/**
 * Gets the type of PHQL queries
 *
 * @return int
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_Model_Query_BuilderInterface, getType);

/**
 * Compile the PHQL query
 *
 * @return Phalcon\Mvc\Model\QueryInterface
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_Model_Query_BuilderInterface, compile);

/**
 * Returns a PHQL statement built based on the builder parameters
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_Model_Query_BuilderInterface, getPhql);

/**
 * Returns the query built
 *
 * @return Phalcon\Mvc\Model\QueryInterface
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_Model_Query_BuilderInterface, getQuery);
