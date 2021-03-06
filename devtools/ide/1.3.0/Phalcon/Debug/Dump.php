<?php 

namespace Phalcon\Debug {

	/**
	 * Phalcon\Debug\Dump
	 *
	 * Dumps information about a variable(s)
	 *
	 * <code>
	 *    $foo = 123;
	 *    echo (new \Phalcon\Debug\Dump())->variable($foo, "foo");
	 *</code>
	 *
	 * <code>
	 *    $foo = "string";
	 *    $bar = ["key" => "value"];
	 *    $baz = new stdClass();
	 *    echo (new \Phalcon\Debug\Dump())->variables($foo, $bar, $baz);
	 *</code>
	 */
	
	class Dump {

		protected $_detailed;

		protected $_styles;

		protected $_objects;

		/**
		 * \Phalcon\Debug\Dump constructor
		 *
		 * @param array styles set styles for vars type
		 * @param boolean detailed debug object's private and protected properties
		 */
		public function __construct($styles=null, $detailed=null){ }


		/**
		 * Alias of variables() method
		 *
		 * @param mixed variable
		 * @param ...
		 */
		public function all(){ }


		/**
		 * Get style for type
		 */
		public function getStyle($type){ }


		/**
		 * Set styles for vars type
		 */
		public function setStyles($styles=null){ }


		/**
		 * Prepare an HTML string of information about a single variable.
		 */
		public function output($variable, $name=null, $tab=null){ }


		/**
		 * Returns an HTML string of information about a single variable.
		 *
		 * <code>
		 *    echo (new \Phalcon\Debug\Dump())->variable($foo, "foo");
		 * </code>
		 */
		public function variable($variable, $name=null){ }


		/**
		 * Returns an HTML string of debugging information about any number of
		 * variables, each wrapped in a "pre" tag.
		 *
		 * <code>
		 *    $foo = "string";
		 *    $bar = ["key" => "value"];
		 *    $baz = new stdClass();
		 *    echo (new \Phalcon\Debug\Dump())->variables($foo, $bar, $baz);
		 *</code>
		 *
		 * @param mixed variable
		 * @param ...
		 */
		public function variables(){ }


		public function one($variable, $name=null){ }


		public function var($variable, $name=null){ }


		public function vars(){ }

	}
}
