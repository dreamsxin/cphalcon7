<?php 

namespace Phalcon\Http {

	/**
	 * Phalcon\Http\Client
	 *
	 * Creates a new request object for the given URI.
	 *
	 *<code>
	 *	$client = Phalcon\Http\Client::factory();
	 *</code>
	 *
	 */
	
	abstract class Client {

		public static function factory($uri=null, $method=null){ }

	}
}
