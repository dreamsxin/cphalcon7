<?php 

namespace Phalcon\Forms\Element {

	/**
	 * Phalcon\Forms\Element\Radio
	 *
	 * input[type="radio"] for forms
	 */
	
	class Radio extends \Phalcon\Forms\Element implements \Phalcon\Forms\ElementInterface {

		/**
		 * \Phalcon\Forms\Element\Radio constructor
		 *
		 * @param string $name
		 * @param array $attributes
		 * @param array $options
		 * @param array $optionsValues
		 */
		public function __construct($name, $attributes=null, $options=null, $optionsValues=null, $type=null){ }


		/**
		 * Renders the element widget returning html
		 *
		 * @param array $attributes
		 * @return string
		 */
		public function render($attributes=null){ }

	}
}
