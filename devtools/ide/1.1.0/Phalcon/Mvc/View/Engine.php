<?php 

namespace Phalcon\Mvc\View {

	/**
	 * Phalcon\Mvc\View\Engine
	 *
	 * All the template engine adapters must inherit this class. This provides
	 * basic interfacing between the engine and the Phalcon\Mvc\View component.
	 */
	
	abstract class Engine extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Mvc\View\EngineInterface {

		protected $_view;

		protected $_layout;

		protected $_params;

		/**
		 * \Phalcon\Mvc\View\Engine constructor
		 *
		 * @param \Phalcon\Mvc\ViewInterface $view
		 * @param \Phalcon\DiInterface $dependencyInjector
		 */
		public function __construct($view, $dependencyInjector=null){ }


		/**
		 * Returns cached ouput on another view stage
		 *
		 * @return array
		 */
		public function getContent(){ }


		/**
		 * Start a new section block
		 *
		 * @param string $name
		 */
		public function startSection($name){ }


		/**
		 * Stop the current section block
		 *
		 * @return string
		 */
		public function stopSection(){ }


		/**
		 * Returns the content for a section block
		 *
		 * @param string $name
		 * @param string $default
		 * @return string|null
		 */
		public function section($name, $defaultValue=null){ }


		/**
		 * Renders a partial inside another view
		 *
		 * @param string $partialPath
		 * @param array $params
		 * @return string
		 */
		public function partial($partialPath){ }


		/**
		 * Returns the view component related to the adapter
		 *
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function getView(){ }


		/**
		 * Adds a user-defined method
		 *
		 * @param string $name
		 * @param closure $methodCallable
		 * @return \Phalcon\Mvc\View\Engine
		 */
		public function addMethod($name, $handler){ }


		/**
		 * Handles method calls when a method is not implemented
		 *
		 * @param string $method
		 * @param array $arguments
		 * @return mixed
		 */
		public function __call($method, $arguments=null){ }

	}
}
