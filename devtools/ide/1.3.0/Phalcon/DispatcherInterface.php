<?php 

namespace Phalcon {

	/**
	 * Phalcon\DispatcherInterface initializer
	 */
	
	interface DispatcherInterface {

		/**
		 * Sets the default action suffix
		 *
		 * @param string $actionSuffix
		 */
		public function setActionSuffix($actionSuffix);


		/**
		 * Sets the default module
		 *
		 * @param string $module
		 */
		public function setDefaultModule($module);


		/**
		 * Gets the default module
		 *
		 * @return string
		 */
		public function getDefaultModule();


		/**
		 * Sets the default namespace
		 *
		 * @param string $namespace
		 */
		public function setDefaultNamespace($namespace);


		/**
		 * Gets the default namespace
		 *
		 * @return string
		 */
		public function getDefaultNamespace();


		/**
		 * Sets the default handler name
		 *
		 * @param string $handlerName
		 */
		public function setDefaultHandler($handlerName);


		/**
		 * Gets the default handler name
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
		 * Sets the module name to be dispatched
		 *
		 * @param string $moduleName
		 */
		public function setModuleName($module);


		/**
		 * Gets last dispatched module name
		 *
		 * @return string
		 */
		public function getModuleName();


		/**
		 * Sets the namespace to be dispatched
		 *
		 * @param string $namespaceName
		 */
		public function setNamespaceName($namespace);


		/**
		 * Gets last dispatched namespace
		 *
		 * @return string
		 */
		public function getNamespaceName();


		/**
		 * Sets the handler name to be dispatched
		 *
		 * @param string $handlerName
		 */
		public function setHandlerName($handlerName);


		/**
		 * Gets last dispatched handler name
		 *
		 * @return string
		 */
		public function getHandlerName();


		/**
		 * Sets the action name to be dispatched
		 *
		 * @param string $actionName
		 */
		public function setActionName($actionName);


		/**
		 * Gets last dispatched action name
		 *
		 * @return string
		 */
		public function getActionName();


		/**
		 * Sets action params to be dispatched
		 *
		 * @param array $params
		 */
		public function setParams($params);


		/**
		 * Gets action params
		 *
		 * @return array
		 */
		public function getParams();


		public function hasParam($param);


		/**
		 * Set a param by its name or numeric index
		 *
		 * @param  mixed $param
		 * @param  mixed $value
		 */
		public function setParam($param, $value);


		/**
		 * Gets a param by its name or numeric index
		 *
		 * @param  mixed $param
		 * @param  string|array $filters
		 * @return mixed
		 */
		public function getParam($param, $filters=null);


		/**
		 * Checks if the dispatch loop is finished or has more pendent controllers/tasks to disptach
		 *
		 * @return boolean
		 */
		public function isFinished();


		/**
		 * Returns value returned by the lastest dispatched action
		 *
		 * @return mixed
		 */
		public function getReturnedValue();


		/**
		 * Dispatches a handle action taking into account the routing parameters
		 *
		 * @return object
		 */
		public function dispatch();


		/**
		 * Forwards the execution flow to another controller/action
		 *
		 * @param array $forward
		 */
		public function forward($forward);


		/**
		 * Forwards the execution flow to another controller/action
		 *
		 * @param array $forward
		 */
		public function camelizeNamespace($camelize);


		/**
		 * Set error handler
		 *
		 * @param mixed $handler
		 * @param int $exception_code
		 * @return \Phalcon\DispatcherInterface
		 */
		public function setErrorHandler($callback, $exception_code=null);


		/**
		 * Get error handler
		 *
		 * @param int $exception_code
		 * @return mixed
		 */
		public function getErrorHandler($exception_code);

	}
}
