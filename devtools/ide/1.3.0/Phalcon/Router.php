<?php 

namespace Phalcon {

	/**
	 * Phalcon\Router
	 *
	 * Base class for Phalcon\Router and Phalcon\Mvc\Router
	 *
	 */
	
	abstract class Router extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\RouterInterface {

		const MODE_DEFAULT = 0;

		const MODE_NONE = 1;

		const MODE_REST = 2;

		protected $_module;

		protected $_namespace;

		protected $_handler;

		protected $_action;

		protected $_params;

		protected $_defaultModule;

		protected $_defaultNamespace;

		protected $_defaultHandler;

		protected $_defaultAction;

		protected $_defaultParams;

		protected $_caseSensitive;

		protected $_mode;

		/**
		 * Sets the name of the default module
		 *
		 * @param string $moduleName
		 */
		public function setDefaultModule($moduleName){ }


		/**
		 * Gets the name of the default module
		 *
		 * @return string
		 */
		public function getDefaultModule(){ }


		/**
		 * Sets the name of the default namespace
		 *
		 * @param string $namespaceName
		 */
		public function setDefaultNamespace($namespaceName){ }


		/**
		 * Gets the name of the default namespace
		 *
		 * @return string
		 */
		public function getDefaultNamespace(){ }


		/**
		 * Sets the default handle name
		 *
		 * @param string $handleName
		 */
		public function setDefaultHandler($handlerName){ }


		/**
		 * Gets the default handle name
		 *
		 * @return string
		 */
		public function getDefaultHandler(){ }


		/**
		 * Sets the default action name
		 *
		 * @param string $actionName
		 */
		public function setDefaultAction($actionName){ }


		/**
		 * Gets the default action name
		 *
		 * @return string
		 */
		public function getDefaultAction(){ }


		/**
		 * Sets the default extra params
		 *
		 * @param string $actionName
		 */
		public function setDefaultParams($params){ }


		/**
		 * Gets the default extra params
		 *
		 * @return string
		 */
		public function getDefaultParams(){ }


		/**
		 * Sets the case sensitive
		 * @param boolean $caseSensitive
		 * @return string
		 */
		public function setCaseSensitive($caseSensitive){ }


		/**
		 * Returns the case sensitive
		 *
		 * @return boolean
		 */
		public function getCaseSensitive(){ }


		/**
		 * Sets the mode
		 *
		 * @param int $mode
		 */
		public function setMode($mode){ }


		/**
		 * Gets the mode
		 *
		 * @param int $mode
		 */
		public function getMode(){ }


		/**
		 * Sets proccesed module name
		 *
		 * @param string $moduleName
		 */
		public function setModuleName($moduleName){ }


		/**
		 * Returns proccesed module name
		 *
		 * @return string
		 */
		public function getModuleName(){ }


		/**
		 * Sets proccesed namespace name
		 *
		 * @param string $namespaceName
		 */
		public function setNamespaceName($namespaceName){ }


		/**
		 * Returns proccesed namespace name
		 *
		 * @return string
		 */
		public function getNamespaceName(){ }


		/**
		 * Sets proccesed handle name
		 *
		 * @param string $handleName
		 */
		public function setHandlerName($handlerName){ }


		/**
		 * Returns proccesed handle name
		 *
		 * @return string
		 */
		public function getHandlerName(){ }


		/**
		 * Sets proccesed action name
		 *
		 * @param string $actionName
		 */
		public function setActionName($actionName){ }


		/**
		 * Returns proccesed action name
		 *
		 * @return string
		 */
		public function getActionName(){ }


		/**
		 * Sets proccesed extra params
		 *
		 * @param array $params
		 */
		public function setParams($params){ }


		/**
		 * Returns proccesed extra params
		 *
		 * @return array
		 */
		public function getParams(){ }

	}
}
