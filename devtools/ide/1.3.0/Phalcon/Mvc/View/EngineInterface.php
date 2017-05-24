<?php 

namespace Phalcon\Mvc\View {

	/**
	 * Phalcon\Mvc\View\EngineInterface initializer
	 */
	
	interface EngineInterface {

		/**
		 * Returns cached ouput on another view stage
		 *
		 * @return array
		 */
		public function getContent();


		/**
		 * Start a new section block
		 *
		 * @param string $name
		 */
		public function startSection($name);


		/**
		 * Stop the current section block
		 *
		 * @return string
		 */
		public function stopSection();


		/**
		 * Returns the content for a section block
		 *
		 * @param string $name
		 * @param string $default
		 * @return string|null
		 */
		public function section($name, $defaultValue=null);


		/**
		 * Renders a partial inside another view
		 *
		 * @param string $partialPath
		 * @return string
		 */
		public function partial($partialPath);


		/**
		 * Renders a view using the template engine
		 *
		 * @param string $path
		 * @param array $params
		 * @param boolean $mustClean
		 */
		public function render($path, $params, $mustClean=null);


		/**
		 * Adds a user-defined method
		 *
		 * @param string $name
		 * @param callable $handler
		 * @return \Phalcon\Mvc\View\EngineInterface
		 */
		public function addMethod($name, \Closure $handler);


		/**
		 * Handles method calls when a method is not implemented
		 *
		 * @param string $method
		 * @param array $arguments
		 * @return mixed
		 */
		public function __call($method, $arguments=null);

	}
}
