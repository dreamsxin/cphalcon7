<?php 

namespace Phalcon {

	/**
	 * Phalcon\Mvc\RouterInterface initializer
	 */
	
	interface RouterInterface {

		/**
		 * Sets the name of the default module
		 *
		 * @param string $moduleName
		 */
		public function setDefaultModule($moduleName);


		/**
		 * Gets the name of the default module
		 *
		 * @return string
		 */
		public function getDefaultModule();


		public function setDefaultNamespace($namespaceName);


		public function getDefaultNamespace();


		/**
		 * Sets the default handle name
		 *
		 * @param string $handlerName
		 */
		public function setDefaultHandler($handlerName);


		/**
		 * Gets the default handle name
		 *
		 * @return string
		 */
		public function getDefaultHandler();


		/**
		 * Sets the default action name
		 *
		 * @param string $actionName
		 */
		public function setDefaultAction($actionName);


		/**
		 * Gets the default action name
		 *
		 * @return string
		 */
		public function getDefaultAction();


		/**
		 * Sets the default extra params
		 *
		 * @param array $actionName
		 */
		public function setDefaultParams($params);


		/**
		 * Gets the default extra params
		 *
		 * @return array
		 */
		public function getDefaultParams();


		/**
		 * Sets the case sensitive
		 *
		 * @param boolean $caseSensitive
		 */
		public function setCaseSensitive($caseSensitive);


		/**
		 * Gets the case sensitive
		 *
		 * @return int
		 */
		public function getCaseSensitive();


		/**
		 * Sets the mode
		 *
		 * @param int $mode
		 */
		public function setMode($mode);


		/**
		 * Gets the mode
		 *
		 * @return int
		 */
		public function getMode();


		/**
		 * Sets processed module name
		 *
		 * @param string $moduleName
		 */
		public function setModuleName($moduleName);


		/**
		 * Returns processed module name
		 *
		 * @return string
		 */
		public function getModuleName();


		/**
		 * Sets processed namespace name
		 *
		 * @param string $namespaceName
		 */
		public function setNamespaceName($namespaceName);


		/**
		 * Returns processed namespace name
		 *
		 * @return string
		 */
		public function getNamespaceName();


		/**
		 * Sets processed handle name
		 *
		 * @param string $handleName
		 */
		public function setHandlerName($handlerName);


		/**
		 * Returns processed handle name
		 *
		 * @return string
		 */
		public function getHandlerName();


		/**
		 * Sets processed action name
		 *
		 * @param string $actionName
		 */
		public function setActionName($actionName);


		/**
		 * Returns processed action name
		 *
		 * @return string
		 */
		public function getActionName();


		/**
		 * Sets processed extra params
		 *
		 * @param array $params
		 */
		public function setParams($params);


		/**
		 * Returns processed extra params
		 *
		 * @return array
		 */
		public function getParams();


		/**
		 * Handles routing information received from the rewrite engine
		 *
		 * @param string $uri
		 */
		public function handle($uri=null);

	}
}
