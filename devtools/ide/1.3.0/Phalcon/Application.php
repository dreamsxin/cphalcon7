<?php 

namespace Phalcon {

	/**
	 * Phalcon\Application
	 *
	 * Base class for Phalcon\Cli\Console and Phalcon\Mvc\Application
	 *
	 *</code>
	 */
	
	abstract class Application extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface {

		protected $_defaultModule;

		protected $_modules;

		protected $_implicitView;

		/**
		 * Register an array of modules present in the application
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
		 * @param \Phalcon\Application
		 */
		public function registerModules($modules, $merge=null){ }


		/**
		 * Return the modules registered in the application
		 *
		 * @return array
		 */
		public function getModules(){ }


		/**
		 * Sets the module name to be used if the router doesn't return a valid module
		 *
		 * @param string $defaultModule
		 * @return \Phalcon\Application
		 */
		public function setDefaultModule($defaultModule){ }


		/**
		 * Returns the default module name
		 *
		 * @return string
		 */
		public function getDefaultModule(){ }


		abstract public function handle($data=null);

	}
}
