<?php 

namespace Phalcon\Mvc {

	/**
	 * Phalcon\Mvc\JsonRpc
	 *
	 * This component encapsulates all the complex operations behind instantiating every component
	 * needed and integrating it with the rest to allow the MVC pattern to operate as desired.
	 *
	 *<code>
	 *
	 *	$jsonrpc = new \Phalcon\Mvc\JsonRpc($di);
	 *	echo $jsonrpc->handle()->getContent();
	 *
	 *</code>
	 */
	
	class JsonRpc extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface {

		protected $_defaultModule;

		protected $_modules;

		protected $_moduleObject;

		/**
		 * \Phalcon\Mvc\JsonRpc
		 *
		 * @param \Phalcon\Di $dependencyInjector
		 */
		public function __construct($dependencyInjector=null){ }


		/**
		 * Register an array of modules present in the jsonrpc
		 *
		 *<code>
		 *	$this->registerModules(array(
		 *		'frontend' => array(
		 *			'className' => 'Multiple\Frontend\Module',
		 *			'path' => '../apps/frontend/Module.php'
		 *		),
		 *		'backend' => array(
		 *			'className' => 'Multiple\Backend\Module',
		 *			'path' => '../apps/backend/Module.php'
		 *		)
		 *	));
		 *</code>
		 *
		 * @param array $modules
		 * @param boolean $merge
		 * @param \Phalcon\Mvc\JsonRpc
		 */
		public function registerModules($modules, $merge=null){ }


		/**
		 * Return the modules registered in the jsonrpc
		 *
		 * @return array
		 */
		public function getModules(){ }


		/**
		 * Sets the module name to be used if the router doesn't return a valid module
		 *
		 * @param string $defaultModule
		 * @return \Phalcon\Mvc\JsonRpc
		 */
		public function setDefaultModule($defaultModule){ }


		/**
		 * Returns the default module name
		 *
		 * @return string
		 */
		public function getDefaultModule(){ }


		/**
		 * Handles a MVC request
		 *
		 * @param string $uri
		 * @return \Phalcon\Http\ResponseInterface
		 */
		public function handle(){ }

	}
}
