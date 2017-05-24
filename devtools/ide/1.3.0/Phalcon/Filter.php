<?php 

namespace Phalcon {

	/**
	 * Phalcon\Filter
	 *
	 * The Phalcon\Filter component provides a set of commonly needed data filters. It provides
	 * object oriented wrappers to the php filter extension. Also allows the developer to
	 * define his/her own filters
	 *
	 *<code>
	 *	$filter = new Phalcon\Filter();
	 *	$filter->sanitize("some(one)@exa\\mple.com", "email"); // returns "someone@example.com"
	 *	$filter->sanitize("hello<<", "string"); // returns "hello"
	 *	$filter->sanitize("!100a019", "int"); // returns "100019"
	 *	$filter->sanitize("!100a019.01a", "float"); // returns "100019.01"
	 *</code>
	 */
	
	class Filter implements \Phalcon\FilterInterface {

		protected $_filters;

		protected $_allowTags;

		protected $_allowAttributes;

		protected $_dateFormat;

		public static $FILTER_EMAIL;

		public static $FILTER_ABSINT;

		public static $FILTER_INT;

		public static $FILTER_INT_CAST;

		public static $FILTER_STRING;

		public static $FILTER_FLOAT;

		public static $FILTER_FLOAT_CAST;

		public static $FILTER_ALPHANUM;

		public static $FILTER_TRIM;

		public static $FILTER_STRIPTAGS;

		public static $FILTER_LOWER;

		public static $FILTER_UPPER;

		public static $FILTER_XSS;

		/**
		 * \Phalcon\Filter constructor
		 */
		public function __construct($options=null, $handler=null){ }


		/**
		 * Adds a user-defined filter
		 *
		 * @param string $name
		 * @param callable $handler
		 * @return \Phalcon\Filter
		 */
		public function add($name, $handler){ }


		/**
		 * Sanitizes a value with a specified single or set of filters
		 *
		 * @param mixed $value
		 * @param mixed $filters
		 * @param boolean $norecursive
		 * @param array $options
		 * @return mixed
		 */
		public function sanitize($value, $filters, $noRecursive=null, $options=null){ }


		/**
		 * Internal sanitize wrapper to filter_var
		 *
		 * @param  mixed $value
		 * @param  string $filter
		 * @return mixed
		 */
		protected function _sanitize(){ }


		/**
		 * Return the user-defined filters in the instance
		 *
		 * @return object[]
		 */
		public function getFilters(){ }

	}
}
