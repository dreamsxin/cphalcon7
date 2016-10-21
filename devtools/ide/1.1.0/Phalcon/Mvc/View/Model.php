<?php 

namespace Phalcon\Mvc\View {

	/**
	 * Phalcon\Mvc\View\Model
	 *
	 * This component allows to render views without hicherquical levels
	 *
	 *<code>
	 * $view = new Phalcon\Mvc\View\Model();
	 * echo $view->render('templates/my-view', array('content' => $html));
	 *</code>
	 *
	 */
	
	class Model implements \Phalcon\Mvc\View\ModelInterface {

		protected $_viewParams;

		protected $_captureTo;

		protected $_template;

		protected $_childs;

		protected $_terminal;

		protected $_append;

		protected $_view;

		/**
		 * \Phalcon\Mvc\View\Model constructor
		 *
		 * @param array $options
		 */
		public function __construct($vars=null, $template=null, $capture=null){ }


		/**
		 * Set the template to be used by this model
		 *
		 * @param  string $template
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function setTemplate($template){ }


		/**
		 * Get the template to be used by this model
		 *
		 * @return string
		 */
		public function getTemplate(){ }


		/**
		 * Set all the render params
		 *
		 * @param array $params
		 * @param boolean $merge
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function setVars($params, $merge=null){ }


		/**
		 * Get the vars
		 *
		 * @return string
		 */
		public function getVars(){ }


		/**
		 * Set a single view parameter
		 *
		 * @param string $key
		 * @param mixed $value
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function setVar($key, $value){ }


		/**
		 * Get the vars
		 *
		 * @param string $key
		 * @return mixed
		 */
		public function getVar($key, $default_value=null){ }


		/**
		 * Add a child model
		 *
		 * @param  \Phalcon\Mvc\View\ModelInterface $child
		 * @param  null|string $captureTo Optional; if specified, the "capture to" value to set on the child
		 * @param  null|bool $append Optional; if specified, append to child  with the same capture
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function addChild($viewmodel, $name=null, $append=null){ }


		/**
		 * Append a child model
		 *
		 * @param  \Phalcon\Mvc\View\ModelInterface $child
		 * @param  null|string $captureTo Optional; if specified, the "capture to" value to set on the child
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function appendChild($viewmodel, $name=null){ }


		/**
		 * Return a child model or all child model
		 *
		 * @param null|string $captureTo
		 * @return array
		 */
		public function getChild($name){ }


		/**
		 * Does the model have any children?
		 *
		 * @param null|string $captureTo
		 * @return boolean
		 */
		public function hasChild(){ }


		/**
		 * Set the name of the variable to capture this model to, if it is a child model
		 *
		 * @param string $capture
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function setCaptureTo($capture){ }


		/**
		 * Get the name of the variable to which to capture this model
		 *
		 * @return string
		 */
		public function getCaptureTo(){ }


		/**
		 * Set flag indicating whether or not this is considered a terminal or standalone model
		 *
		 * @param boolean $terminate
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function setTerminal($terminate){ }


		/**
		 * Is this considered a terminal or standalone model?
		 *
		 * @return boolean
		 */
		public function getTerminal(){ }


		/**
		 * Set flag indicating whether or not append to child  with the same capture
		 *
		 * @param boolean $append
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function setAppend($append){ }


		/**
		 * Is this append to child  with the same capture?
		 *
		 * @return boolean
		 */
		public function isAppend(){ }


		/**
		 * Set the view
		 *
		 * @param \Phalcon\Mvc\ViewInterface $view
		 * @return \Phalcon\Mvc\View\ModelInterface
		 */
		public function setView($view){ }


		/**
		 * Get the view
		 *
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function getView(){ }


		/**
		 * Renders the view
		 *
		 * @return string
		 */
		public function render(){ }


		/**
		 * Magic method to pass variables to the views
		 *
		 * @param string $key
		 * @param mixed $value
		 */
		public function __set($property, $value){ }


		/**
		 * Magic method to retrieve a variable passed to the view
		 *
		 * @param string $key
		 * @return mixed
		 */
		public function __get($property){ }


		/**
		 * Magic method to inaccessible a variable passed to the view
		 *
		 * @param string $key
		 * @return boolean
		 */
		public function __isset($property){ }

	}
}
