<?php 

namespace Phalcon\Mvc {

	/**
	 * Phalcon\Mvc\Dispatcher
	 *
	 * Dispatching is the process of taking the request object, extracting the module name,
	 * controller name, action name, and optional parameters contained in it, and then
	 * instantiating a controller and calling an action of that controller.
	 *
	 *<code>
	 *
	 *	$di = new Phalcon\Di();
	 *
	 *	$dispatcher = new Phalcon\Mvc\Dispatcher();
	 *
	 *  $dispatcher->setDI($di);
	 *
	 *	$dispatcher->setControllerName('posts');
	 *	$dispatcher->setActionName('index');
	 *	$dispatcher->setParams(array());
	 *
	 *	$controller = $dispatcher->dispatch();
	 *
	 *</code>
	 */
	
	class Dispatcher extends \Phalcon\Dispatcher implements \Phalcon\DispatcherInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface, \Phalcon\Mvc\DispatcherInterface {

		const EXCEPTION_NO_DI = 1;

		const EXCEPTION_CYCLIC_ROUTING = 2;

		const EXCEPTION_HANDLER_NOT_FOUND = 4;

		const EXCEPTION_INVALID_HANDLER = 8;

		const EXCEPTION_INVALID_PARAMS = 16;

		const EXCEPTION_ACTION_NOT_FOUND = 32;

		protected $_handlerSuffix;

		protected $_defaultHandler;

		protected $_defaultAction;

		/**
		 * Sets the default controller suffix
		 *
		 * @param string $controllerSuffix
		 */
		public function setControllerSuffix($controllerSuffix){ }


		/**
		 * Sets the default controller name
		 *
		 * @param string $controllerName
		 */
		public function setDefaultController($controllerName){ }


		/**
		 * Sets the controller name to be dispatched
		 *
		 * @param string $controllerName
		 */
		public function setControllerName($controllerName, $isExact=null){ }


		/**
		 * Gets last dispatched controller name
		 *
		 * @return string
		 */
		public function getControllerName(){ }


		/**
		 * Throws an internal exception
		 *
		 * @param string $message
		 * @param int $exceptionCode
		 */
		protected function _throwDispatchException(){ }


		/**
		 * Handles a user exception
		 *
		 * @param \Exception $exception
		 *
		 * @warning If any additional logic is to be implemented here, please check
		 * phalcon_dispatcher_fire_event() first
		 */
		protected function _handleException(){ }


		/**
		 * Possible controller class name that will be located to dispatch the request
		 *
		 * @return string
		 */
		public function getControllerClass(){ }


		/**
		 * Returns the lastest dispatched controller
		 *
		 * @return \Phalcon\Mvc\ControllerInterface
		 */
		public function getLastController(){ }


		/**
		 * Returns the active controller in the dispatcher
		 *
		 * @return \Phalcon\Mvc\ControllerInterface
		 */
		public function getActiveController(){ }


		/**
		 * Returns the previous controller in the dispatcher
		 *
		 * @return string
		 */
		public function getPreviousControllerName(){ }


		/**
		 * Returns the previous action in the dispatcher
		 *
		 * @return string
		 */
		public function getPreviousActionName(){ }

	}
}
