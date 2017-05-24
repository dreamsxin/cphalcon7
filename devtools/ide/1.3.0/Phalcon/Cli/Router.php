<?php 

namespace Phalcon\Cli {

	/**
	 * Phalcon\Cli\Router
	 *
	 * <p>Phalcon\Cli\Router is the standard framework router. Routing is the
	 * process of taking a command-line arguments and
	 * decomposing it into parameters to determine which module, task, and
	 * action of that task should receive the request</p>
	 *
	 *<code>
	 *	$router = new Phalcon\Cli\Router();
	 *	$router->handle(array(
	 *		'module' => 'main',
	 *		'task' => 'videos',
	 *		'action' => 'process'
	 *	));
	 *	echo $router->getTaskName();
	 *</code>
	 *
	 */
	
	class Router extends \Phalcon\Router implements \Phalcon\RouterInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {

		const MODE_DEFAULT = 0;

		const MODE_NONE = 1;

		const MODE_REST = 2;

		/**
		 * \Phalcon\Cli\Router constructor
		 */
		public function __construct(){ }


		/**
		 * Sets the default task name
		 *
		 * @param string $taskName
		 */
		public function setDefaultTask($handlerName){ }


		/**
		 * Returns proccesed task name
		 *
		 * @return string
		 */
		public function getTaskName(){ }


		/**
		 * Handles routing information received from command-line arguments
		 *
		 * @param array $arguments
		 */
		public function handle($arguments=null){ }

	}
}
