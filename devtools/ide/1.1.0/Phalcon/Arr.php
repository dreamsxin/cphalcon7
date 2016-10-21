<?php 

namespace Phalcon {

	/**
	 * Phalcon\Arr
	 *
	 * Provides utilities to work with arrs
	 */
	
	abstract class Arr {

		public static $delimiter;

		/**
		 * Tests if an array is associative or not.
		 *
		 *     // Returns TRUE
		 *     \Phalcon\Arr::is_assoc(array('username' => 'john.doe'))
		 *
		 * @param array $array
		 * @return boolean
		 */
		public static function is_assoc($array){ }


		/**
		 * Test if a value is an array with an additional check for array-like objects.
		 *
		 *     // Returns TRUE
		 *     \Phalcon\Arr::is_array(array());
		 *
		 * @param mixed $value
		 * @return boolean
		 */
		public static function is_array($array){ }


		/**
		 * Gets a value from an array using a dot separated path.
		 *
		 *     // Get the value of $array['foo']['bar']
		 *     $value = \Phalcon\Arr::path($array, 'foo.bar');
		 *
		 * Using a wildcard "*" will search intermediate arrays and return an array.
		 *
		 *     // Get the values of "color" in theme
		 *     $colors = \Phalcon\Arr::path($array, 'theme.*.color');
		 *
		 *     // Using an array of keys
		 *     $colors = \Phalcon\Arr::path($array, array('theme', '*', 'color'));
		 *
		 * @param array $array
		 * @param mixed $path
		 * @param mixed $default
		 * @param string $delimiter
		 * @return mixed
		 */
		public static function path($array, $path, $default_value=null, $delimiter=null){ }


		/**
		 * Set a value on an array by path.
		 *
		 * Using a wildcard "*" will search intermediate arrays and return an array.
		 *
		 *     // Set the values of "color" in theme
		 *     $array = array('theme' => array('one' => array('color' => 'green'), 'two' => array('size' => 11));
		 *     \Phalcon\Arr::set_path($array, 'theme.*.color', 'red');
		 *     // Result: array('theme' => array('one' => array('color' => 'red'), 'two' => array('size' => 11, 'color' => 'red'));
		 *
		 * @param array $array
		 * @param string $path
		 * @param mixed $value
		 * @param string $delimiter
		 */
		public static function set_path($array, $path, $value, $delimiter=null, $flag=null){ }


		/**
		 * Fill an array with a range of numbers.
		 *
		 *     // Fill an array with values 5, 10, 15, 20
		 *     $values = \Phalcon\Arr::range(5, 20);
		 *
		 * @param integer $step
		 * @param integer $max
		 * @return array
		 */
		public static function range($step=null, $max=null){ }


		/**
		 * Retrieve a single key from an array. If the key does not exist in the
		 * array, the default value will be returned instead.
		 *
		 *     // Get the value "username" from $_POST, if it exists
		 *     $username = \Phalcon\Arr::get($_POST, 'username');
		 *
		 * @param array $array
		 * @param string|array|\Closure $key
		 * @param mixed $default_value
		 * @return mixed
		 */
		public static function get($array, $key, $default_value=null){ }


		/**
		 * Choice one value, If the key does not exist in the array, the value2 will be returned instead.
		 *
		 *     // Choice the "value1", if exists the value "email" of $_POST
		 *     $username = \Phalcon\Arr::choice($_POST, 'email', 'value1', 'value2');
		 *
		 * @param array $array
		 * @param string $key
		 * @param string $value1
		 * @param string $value2
		 * @return mixed
		 */
		public static function choice($array, $key, $value1, $value2=null){ }


		/**
		 * Retrieves multiple paths from an array. If the path does not exist in the
		 * array, the default value will be added instead.
		 *
		 *     // Get the values "username", "password" from $_POST
		 *     $auth = \Phalcon\Arr::extract($_POST, array('username', 'password'));
		 *     
		 *     // Get the value "level1.level2a" from $data
		 *     $data = array('level1' => array('level2a' => 'value 1', 'level2b' => 'value 2'));
		 *     \Phalcon\Arr::extract($data, array('level1.level2a', 'password'));
		 *
		 * @param array $array
		 * @param array $paths
		 * @param mixed $default_value
		 * @return array
		 */
		public static function extract($array, $paths, $default_value=null){ }


		/**
		 * Retrieves muliple single-key values from a list of arrays.
		 *
		 *     // Get all of the "id" values from a result
		 *     $ids = \Phalcon\Arr::pluck($result, 'id');
		 *
		 * @param array $array
		 * @param string $key
		 * @return array
		 */
		public static function pluck($array, $key){ }


		/**
		 * Adds a value to the beginning of an associative array.
		 *
		 *     // Add an empty value to the start of a select list
		 *     \Phalcon\Arr::unshift($array, 'none', 'Select a value');
		 *
		 * @param array $array
		 * @param string $key
		 * @param mixed $val
		 * @return array
		 */
		public static function unshift($array, $key, $val){ }


		/**
		 * Recursive version of [array_map](http://php.net/array_map), applies one or more
		 * callbacks to all elements in an array, including sub-arrays.
		 *
		 *     // Apply "strip_tags" to every element in the array
		 *     $array = \Phalcon\Arr::map('strip_tags', $array);
		 *
		 *     // Apply $this->filter to every element in the array
		 *     $array = \Phalcon\Arr::map(array(array($this,'filter')), $array);
		 *
		 * @param mixed $callbacks
		 * @param array $array
		 * @param array $keys
		 * @return array
		 */
		public static function map($callbacks, $array, $keys=null){ }


		/**
		 * Recursively merge two or more arrays. Values in an associative array
		 * overwrite previous values with the same key. Values in an indexed array
		 * are appended, but only when they do not already exist in the result.
		 *
		 * Note that this does not work the same as [array_merge_recursive](http://php.net/array_merge_recursive)!
		 *
		 *     $john = array('name' => 'john', 'children' => array('fred', 'paul', 'sally', 'jane'));
		 *     $mary = array('name' => 'mary', 'children' => array('jane'));
		 *
		 *     // John and Mary are married, merge them together
		 *     $john = \Phalcon\Arr::merge($john, $mary);
		 *
		 *     // The output of $john will now be:
		 *     array('name' => 'mary', 'children' => array('fred', 'paul', 'sally', 'jane'))
		 *
		 * @param array $array1
		 * @param array $array2,...
		 * @return array
		 */
		public static function merge($array1, $array2){ }


		/**
		 * Overwrites an array with values from input arrays.
		 * Keys that do not exist in the first array will not be added!
		 *
		 *     $a1 = array('name' => 'john', 'mood' => 'happy', 'food' => 'bacon');
		 *     $a2 = array('name' => 'jack', 'food' => 'tacos', 'drink' => 'beer');
		 *
		 *     // Overwrite the values of $a1 with $a2
		 *     $array = \Phalcon\Arr::overwrite($a1, $a2);
		 *
		 *     // The output of $array will now be:
		 *     array('name' => 'jack', 'mood' => 'happy', 'food' => 'tacos')
		 *
		 * @param array $array1
		 * @param array $array2
		 * @return array
		 */
		public static function overwrite($array1, $array2){ }


		/**
		 * Creates a callable function and parameter list from a string representation.
		 * Note that this function does not validate the callback string.
		 *
		 *     // Get the callback function and parameters
		 *     list($func, $params) = \Phalcon\Arr::callback('Foo::bar(apple,orange)');
		 *
		 *     // Get the result of the callback
		 *     $result = call_user_func_array($func, $params);
		 *
		 * @param string $str
		 * @return array function, params
		 */
		public static function callback($str){ }


		/**
		 * Convert a multi-dimensional array into a single-dimensional array.
		 *
		 *     $array = array('set' => array('one' => 'something'), 'two' => 'other');
		 *
		 *     // Flatten the array
		 *     $array = \Phalcon\Arr::flatten($array);
		 *
		 *     // The array will now be
		 *     array('one' => 'something', 'two' => 'other');
		 *
		 * @param array $array
		 * @return array
		 */
		public static function flatten($array){ }


		/**
		 * Convert a array to a array object.
		 *
		 *     $array = array('name' => 'Phalcon7', 'version' => '1.0.x');
		 *
		 *     $arrayobject = \Phalcon\Arr::arrayobject($array);
		 *
		 * @param array $array
		 * @return ArrayObject
		 */
		public static function arrayobject($array){ }


		/**
		 * Gets array key of the postion
		 *
		 *     $array = array('name' => 'Phalcon7', 'version' => '1.0.x');
		 *
		 *     $key = \Phalcon\Arr::key($array, 1);
		 *
		 * @param array $array
		 * @param int $postion
		 * @return mixed
		 */
		public static function key($array, $postion=null){ }


		/**
		 * Filters elements of an array using a the filter
		 *
		 *     $array = array('name' => 'Phalcon7', 'version' => '1.0.x');
		 *
		 *     $key = \Phalcon\Arr::filter($array, 'int');
		 *
		 * @param array $array
		 * @param mixed $filters
		 * @return array
		 */
		public static function filter($array, $callback=null){ }


		/**
		 * Return the sum of all the values in the array using a dot separated path
		 *
		 * @param array $array
		 * @param mixed $path
		 * @param mixed $default
		 * @param string $delimiter
		 * @return mixed
		 */
		public static function sum($array, $path=null){ }


		/**
		 * Converts an object or an array of objects into an array
		 *
		 *<code>
		 *	print_r(Phalcon\Arr::toArray($user);
		 *</code>
		 *
		 * @param object|array|string $object
		 * @param array $properties
		 * @param bool $recursive
		 * @return array
		 */
		public static function toArray($object, $recursive=null, $properties=null){ }

	}
}
