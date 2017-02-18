<?php 

namespace Phalcon {

	/**
	 * Phalcon\Dispatcher
	 *
	 * This is the base class for Phalcon\Mvc\Dispatcher and Phalcon\CLI\Dispatcher.
	 * This class can't be instantiated directly, you can use it to create your own dispatchers
	 */
	
	abstract class Dispatcher extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\DispatcherInterface {

		const EXCEPTION_NO_DI = 1;

		const EXCEPTION_CYCLIC_ROUTING = 2;

		const EXCEPTION_HANDLER_NOT_FOUND = 4;

		const EXCEPTION_INVALID_HANDLER = 8;

		const EXCEPTION_INVALID_PARAMS = 16;

		const EXCEPTION_ACTION_NOT_FOUND = 32;

		protected $_activeHandler;

		protected $_finished;

		protected $_forwarded;

		protected $_moduleName;

		protected $_namespaceName;

		protected $_handlerName;

		protected $_actionName;

		protected $_params;

		protected $_returnedValue;

		protected $_lastHandler;

		protected $_defaultNamespace;

		protected $_defaultHandler;

		protected $_defaultAction;

		protected $_handlerSuffix;

		protected $_actionSuffix;

		protected $_isExactHandler;

		protected $_previousNamespaceName;

		protected $_previousHandlerName;

		protected $_previousActionName;

		protected $_previousParams;

		protected $_camelizeNamespace;

		protected $_camelizeController;

		protected $_errorHandlers;

		/**
		 * \Phalcon\Dispatcher constructor
		 */
		public function __construct(){ }


		/**
		 * Sets the default action suffix
		 *
		 * @param string $actionSuffix
		 */
		public function setActionSuffix($actionSuffix){ }


		/**
		 * Sets the module where the controller is (only informative)
		 *
		 * @param string $moduleName
		 */
		public function setModuleName($moduleName){ }


		/**
		 * Gets the module where the controller class is
		 *
		 * @return string
		 */
		public function getModuleName(){ }


		/**
		 * Sets the namespace where the controller class is
		 *
		 * @param string $namespaceName
		 */
		public function setNamespaceName($namespaceName){ }


		/**
		 * Gets a namespace to be prepended to the current handler name
		 *
		 * @return string
		 */
		public function getNamespaceName(){ }


		/**
		 * Sets the default namespace
		 *
		 * @param string $namespace
		 */
		public function setDefaultNamespace($namespace){ }


		/**
		 * Returns the default namespace
		 *
		 * @return string
		 */
		public function getDefaultNamespace(){ }


		/**
		 * Sets the default action name
		 *
		 * @param string $actionName
		 */
		public function setDefaultAction($actionName){ }


		/**
		 * Sets the action name to be dispatched
		 *
		 * @param string $actionName
		 */
		public function setActionName($actionName){ }


		/**
		 * Gets the lastest dispatched action name
		 *
		 * @return string
		 */
		public function getActionName(){ }


		/**
		 * Sets action params to be dispatched
		 *
		 * @param array $params
		 */
		public function setParams($params){ }


		/**
		 * Gets action params
		 *
		 * @return array
		 */
		public function getParams(){ }


		/**
		 * Set a param by its name or numeric index
		 *
		 * @param  mixed $param
		 * @param  mixed $value
		 */
		public function setParam($param, $value){ }


		/**
		 * Gets a param by its name or numeric index
		 *
		 * @param  mixed $param
		 * @param  string|array $filters
		 * @param  mixed $defaultValue
		 * @return mixed
		 */
		public function getParam($param, $filters=null){ }


		/**
		 * Returns the current method to be/executed in the dispatcher
		 *
		 * @return string
		 */
		public function getActiveMethod(){ }


		/**
		 * Checks if the dispatch loop is finished or has more pendent controllers/tasks to disptach
		 *
		 * @return boolean
		 */
		public function isFinished(){ }


		/**
		 * Sets the finished
		 *
		 * @param boolean $finished
		 */
		public function setFinished($finished){ }


		/**
		 * Sets the latest returned value by an action manually
		 *
		 * @param mixed $value
		 */
		public function setReturnedValue($value){ }


		/**
		 * Returns value returned by the lastest dispatched action
		 *
		 * @return mixed
		 */
		public function getReturnedValue(){ }


		/**
		 * Dispatches a handle action taking into account the routing parameters
		 *
		 * @return object
		 */
		public function dispatch(){ }


		/**
		 * Forwards the execution flow to another controller/action
		 * Dispatchers are unique per module. Forwarding between modules is not allowed
		 *
		 *<code>
		 *  $this->dispatcher->forward(array('controller' => 'posts', 'action' => 'index'));
		 *</code>
		 *
		 * @param string|array $forward
		 * @return bool
		 */
		public function forward($forward){ }


		/**
		 * Check if the current executed action was forwarded by another one
		 *
		 * @return boolean
		 */
		public function wasForwarded(){ }


		/**
		 * Possible class name that will be located to dispatch the request
		 *
		 * @return string
		 */
		public function getHandlerClass(){ }


		/**
		 * Enables/Disables automatically camelize namespace 
		 *
		 *<code>
		 *  $this->dispatcher->camelizeNamespace(FALSE);
		 *</code>
		 *
		 * @param bool $camelize
		 */
		public function camelizeNamespace($camelize){ }


		/**
		 * Enables/Disables automatically camelize controller 
		 *
		 *<code>
		 *  $this->dispatcher->camelizeController(FALSE);
		 *</code>
		 *
		 * @param bool $camelize
		 */
		public function camelizeController($camelize){ }


		/**
		 * Set error handler
		 *
		 * @param mixed $handler
		 * @param int $exception_code
		 * @return \Phalcon\DispatcherInterface
		 */
		public function setErrorHandler($callback, $exception_code=null){ }


		/**
		 * Get error handler
		 *
		 * @param int $exception_code
		 * @return mixed
		 */
		public function getErrorHandler($exception_code){ }


		/**
		 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
		 *
		 * @param string $eventName
		 * @param string $data
		 * @param string $cancelable
		 * @return boolean
		 */
		public function fireEvent($eventName, $data=null, $cancelable=null){ }

	}
}
