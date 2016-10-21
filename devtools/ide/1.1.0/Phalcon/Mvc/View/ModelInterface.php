<?php 

namespace Phalcon\Mvc\View {

	/**
	 * Phalcon\Mvc\View\ModelInterface initializer
	 */
	
	interface ModelInterface {

		/**
		 * Set the template to be used by this model
		 *
		 * @param  string $template
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function setTemplate($template);


		/**
		 * Get the template to be used by this model
		 *
		 * @return string
		 */
		public function getTemplate();


		/**
		 * Set all the render params
		 *
		 * @param array $params
		 * @param boolean $merge
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function setVars($params, $merge=null);


		public function getVars();


		/**
		 * Set a single view parameter
		 *
		 * @param string $key
		 * @param mixed $value
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function setVar($key, $value);


		public function getVar($key, $default_value=null);


		/**
		 * Add a child model
		 *
		 * @param \Phalcon\Mvc\View\ModelInterface $child
		 * @param null|string $captureTo Optional; if specified, the "capture to" value to set on the child
		 * @param null|bool $append Optional; if specified, append to child  with the same capture
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function addChild($viewmodel, $name=null, $append=null);


		/**
		 * Add a child model
		 *
		 * @param \Phalcon\Mvc\View\ModelInterface $child
		 * @param null|string $captureTo Optional; if specified, the "capture to" value to set on the child
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function appendChild($viewmodel, $name=null);


		/**
		 * Return a child model or all child model
		 *
		 * @param null|string $captureTo
		 * @return array
		 */
		public function getChild($name);


		/**
		 * Does the model have any children?
		 *
		 * @param null|string $captureTo
		 * @return boolean
		 */
		public function hasChild();


		/**
		 * Set the name of the variable to capture this model to, if it is a child model
		 *
		 * @param string $capture
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function setCaptureTo($capture);


		/**
		 * Get the name of the variable to which to capture this model
		 *
		 * @return string
		 */
		public function getCaptureTo();


		/**
		 * Set flag indicating whether or not this is considered a terminal or standalone model
		 *
		 * @param boolean $terminate
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function setTerminal($terminate);


		/**
		 * Is this considered a terminal or standalone model?
		 *
		 * @return boolean
		 */
		public function getTerminal();


		/**
		 * Set the view
		 *
		 * @param \Phalcon\Mvc\ViewInterface $view
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function setAppend($append);


		/**
		 * Is this append to child  with the same capture?
		 *
		 * @return boolean
		 */
		public function isAppend();


		public function setView($view);


		/**
		 * Get the view
		 *
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function getView();


		/**
		 * Renders the view
		 *
		 * @return string
		 */
		public function render();

	}
}
