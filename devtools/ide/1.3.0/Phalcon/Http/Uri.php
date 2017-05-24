<?php 

namespace Phalcon\Http {

	/**
	 * Phalcon\Http\Uri
	 *
	 *<code>
	 *	$uri1 = new Phalcon\Http\Uri('http://phalconphp.com/foo/bar/baz?var1=a&var2=1');
	 *
	 *	$uri2 = $uri1->resolve('/last');
	 *	echo $uri2->build(); // http://phalconphp.com/last?var1=a&var2=1
	 *
	 *
	 *	$uri3 = $uri1->resolve('last');
	 *	echo $uri3->build(); // http://phalconphp.com/foo/bar/baz/last?var1=a&var2=1
	 *
	 *	$uri4 = new Phalcon\Http\Uri(array(
	 *	    'scheme' => 'https',
	 *	    'host' => 'admin.example.com',
	 *	    'user' => 'john',
	 *	    'pass' => 'doe'
	 *	));
	 *
	 *	$uri5 = $uri1->resolve($uri4);
	 *	echo $uri5->build(); // https://john:doe@admin.example.com/foo/bar/baz?var1=a&var2=1
	 *</code>
	 */
	
	class Uri {

		protected $_parts;

		/**
		 * \Phalcon\Http\Uri constructor
		 *
		 * @param mixed $uri
		 */
		public function __construct($uri=null){ }


		/**
		 * Magic __toString method returns uri
		 *
		 * @return string
		 */
		public function __toString(){ }


		/**
		 * Magic __unset method
		 *
		 * @param string $key
		 */
		public function __unset($key){ }


		/**
		 * Magic __set method
		 *
		 * @param string $key
		 */
		public function __set($key, $value){ }


		/**
		 * Magic __get method
		 *
		 * @param string $key
		 * @return string
		 */
		public function __get($key){ }


		/**
		 * Magic __isset method
		 *
		 * @param string $key
		 * @return boolean
		 */
		public function __isset($key){ }


		/**
		 * Returns parts
		 *
		 * @return array
		 */
		public function getParts(){ }


		/**
		 * Retrieve the URI path
		 *
		 * @return string
		 */
		public function getPath(){ }


		/**
		 * Returns uri
		 *
		 * @return string
		 */
		public function build(){ }


		public function resolve($uri){ }


		public function extend($uri){ }


		public function extendQuery($param){ }

	}
}
