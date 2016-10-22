<?php 

namespace Phalcon\Forms\Element {

	/**
	 * Phalcon\Forms\Element\Submit
	 *
	 * Component INPUT[type=submit] for forms
	 */
	
	class Submit extends \Phalcon\Forms\Element implements \Phalcon\Forms\ElementInterface {

		/**
		 * \Phalcon\Forms\Element\Submit constructor
		 *
		 * @param string $name
		 * @param array $attributes
		 * @param array $options
		 * @param array $optionsValues
		 */
		public function __construct($name, $attributes=null, $options=null, $optionsValues=null, $type=null){ }


		/**
		 * Renders the element widget
		 *
		 * @param array $attributes
		 * @return string
		 */
		public function render($attributes=null){ }

	}
}
