<?php 

namespace Phalcon\Di {

	/**
	 * Phalcon\Di\Injectable
	 *
	 * This class allows to access services in the services container by just only accessing a public property
	 * with the same name of a registered service
	 */
	
	abstract class Injectable implements \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {

		protected $_dependencyInjector;

		protected $_eventsManager;

		protected $_eventCallbacks;

		/**
 		 * @var \Phalcon\Mvc\Dispatcher|\Phalcon\Mvc\DispatcherInterface
 		 */
		public $dispatcher;

		/**
 		 * @var \Phalcon\Mvc\Router|\Phalcon\Mvc\RouterInterface
 		 */
		public $router;

		/**
 		 * @var \Phalcon\Mvc\Url|\Phalcon\Mvc\UrlInterface
 		 */
		public $url;

		/**
 		 * @var \Phalcon\Http\Request|\Phalcon\HTTP\RequestInterface
 		 */
		public $request;

		/**
 		 * @var \Phalcon\Http\Response|\Phalcon\HTTP\ResponseInterface
 		 */
		public $response;

		/**
 		 * @var \Phalcon\Http\Response\Cookies|\Phalcon\Http\Response\CookiesInterface
 		 */
		public $cookies;

		/**
 		 * @var \Phalcon\Filter|\Phalcon\FilterInterface
 		 */
		public $filter;

		/**
 		 * @var \Phalcon\Flash\Direct
 		 */
		public $flash;

		/**
 		 * @var \Phalcon\Flash\Session
 		 */
		public $flashSession;

		/**
 		 * @var \Phalcon\Session\Adapter\Files|\Phalcon\Session\Adapter|\Phalcon\Session\AdapterInterface
 		 */
		public $session;

		/**
 		 * @var \Phalcon\Events\Manager
 		 */
		public $eventsManager;

		/**
 		 * @var \Phalcon\Db
 		 */
		public $db;

		/**
 		 * @var \Phalcon\Security
 		 */
		public $security;

		/**
 		 * @var \Phalcon\Crypt
 		 */
		public $crypt;

		/**
 		 * @var \Phalcon\Tag
 		 */
		public $tag;

		/**
 		 * @var \Phalcon\Escaper|\Phalcon\EscaperInterface
 		 */
		public $escaper;

		/**
 		 * @var \Phalcon\Annotations\Adapter\Memory|\Phalcon\Annotations\Adapter
 		 */
		public $annotations;

		/**
 		 * @var \Phalcon\Mvc\Model\Manager|\Phalcon\Mvc\Model\ManagerInterface
 		 */
		public $modelsManager;

		/**
 		 * @var \Phalcon\Mvc\Model\MetaData\Memory|\Phalcon\Mvc\Model\MetadataInterface
 		 */
		public $modelsMetadata;

		/**
 		 * @var \Phalcon\Mvc\Model\Transaction\Manager
 		 */
		public $transactionManager;

		/**
 		 * @var \Phalcon\Assets\Manager
 		 */
		public $assets;

		/**
		 * @var \Phalcon\Di|\Phalcon\DiInterface
	 	 */
		public $di;

		/**
		 * @var \Phalcon\Session\Bag
	 	 */
		public $persistent;

		/**
 		 * @var \Phalcon\Mvc\View|\Phalcon\Mvc\ViewInterface
 		 */
		public $view;
		
		/**
		 * Sets the dependency injector
		 *
		 * @param \Phalcon\DiInterface $dependencyInjector
		 * @throw \Phalcon\Di\Exception
		 */
		public function setDI($dependencyInjector){ }


		/**
		 * Returns the internal dependency injector
		 *
		 * @return \Phalcon\DiInterface
		 */
		public function getDI($error=null, $notUseDefault=null){ }


		/**
		 * Sets the event manager
		 *
		 * @param \Phalcon\Events\ManagerInterface $eventsManager
		 */
		public function setEventsManager(\Phalcon\Events\ManagerInterface $eventsManager){ }


		/**
		 * Returns the internal event manager
		 *
		 * @return \Phalcon\Events\ManagerInterface
		 */
		public function getEventsManager(){ }


		/**
		 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
		 *
		 * @param string $eventName
		 * @return boolean
		 */
		public function fireEvent($eventName, $data=null, $cancelable=null){ }


		/**
		 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
		 * This method stops if one of the callbacks/listeners returns boolean false
		 *
		 * @param string $eventName
		 * @return boolean
		 */
		public function fireEventCancel($eventName, $data=null, $cancelable=null){ }


		/**
		 * Fires an event, return data
		 *
		 * @param string $eventName
		 * @param mixed $data
		 * @return mixed
		 */
		public function fireEventData($eventName, $data=null){ }


		/**
		 * Check whether the DI contains a service by a name
		 *
		 * @param string $name
		 * @return boolean
		 */
		public function hasService($name){ }


		/**
		 * Sets a service from the DI
		 *
		 * @param string $serviceName
		 * @param mixed $definition
		 * @param boolean $shared
		 * @return \Phalcon\Di\ServiceInterface
		 */
		public function setService($name){ }


		/**
		 * Obtains a service from the DI
		 *
		 * @param string $serviceName
		 * @return object|null
		 */
		public function getService($name){ }


		/**
		 * Resolves the service based on its configuration
		 *
		 * @param string $name
		 * @param array $parameters
		 * @param boolean $noError
		 * @param boolean $noShared
		 * @return mixed
		 */
		public function getResolveService($name, $args=null, $noerror=null, $noshared=null){ }


		/**
		 * Attach a listener to the events
		 *
		 * @param string $eventType
		 * @param Closure $callback
		 */
		public function attachEvent($eventType, \Closure $callback){ }


		/**
		 * Magic method __get
		 *
		 * @param string $propertyName
		 */
		public function __get($property){ }


		public function __sleep(){ }


		public function __debugInfo(){ }

	}
}
