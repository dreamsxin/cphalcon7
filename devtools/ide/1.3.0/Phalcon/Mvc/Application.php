<?php 

namespace Phalcon\Mvc {

	/**
	 * Phalcon\Mvc\Application
	 *
	 * This component encapsulates all the complex operations behind instantiating every component
	 * needed and integrating it with the rest to allow the MVC pattern to operate as desired.
	 *
	 *<code>
	 *
	 * class Application extends \Phalcon\Mvc\Application
	 * {
	 *
	 *		/\**
	 *		 * Register the services here to make them general or register
	 *		 * in the ModuleDefinition to make them module-specific
	 *		 *\/
	 *		protected function _registerServices()
	 *		{
	 *
	 *		}
	 *
	 *		/\**
	 *		 * This method registers all the modules in the application
	 *		 *\/
	 *		public function main()
	 *		{
	 *			$this->registerModules(array(
	 *				'frontend' => array(
	 *					'className' => 'Multiple\Frontend\Module',
	 *					'path' => '../apps/frontend/Module.php'
	 *				),
	 *				'backend' => array(
	 *					'className' => 'Multiple\Backend\Module',
	 *					'path' => '../apps/backend/Module.php'
	 *				)
	 *			));
	 *		}
	 *	}
	 *
	 *	$application = new Application();
	 *	$application->main();
	 *
	 *</code>
	 */
	
	class Application extends \Phalcon\Application implements \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {

		protected $_implicitView;

		/**
		 * \Phalcon\Mvc\Application
		 *
		 * @param \Phalcon\Di $dependencyInjector
		 */
		public function __construct(\Phalcon\DiInterface $dependencyInjector=null){ }


		/**
		 * By default. The view is implicitly buffering all the output
		 * You can full disable the view component using this method
		 *
		 * @param boolean $implicitView
		 * @return \Phalcon\Mvc\Application
		 */
		public function useImplicitView($implicitView){ }


		/**
		 * Handles a MVC request
		 *
		 * @param string $uri
		 * @return \Phalcon\Http\ResponseInterface
		 */
		public function handle($data=null){ }


		/**
		 * Does a HMVC request in the application
		 *
		 * @param array $location
		 * @param array $data
		 * @return mixed
		 */
		public function request($uri, $data=null, \Phalcon\DiInterface $dependencyInjector=null){ }

	}
}
