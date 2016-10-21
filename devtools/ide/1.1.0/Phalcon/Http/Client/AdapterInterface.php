<?php 

namespace Phalcon\Http\Client {

	/**
	 * Phalcon\Http\Client\AdapterInterface initializer
	 */
	
	interface AdapterInterface {

		public function setUserAgent($useragent);


		public function setAuth($username, $password, $authtype=null, $digest=null, $entityBody=null);


		public function setHeader($name, $value);


		public function setHeaders($headers);


		public function setData($data, $type=null);


		public function setFiles($files);


		public function get($uri=null, $data=null);


		public function head($uri=null, $data=null);


		public function post($uri=null, $data=null);


		public function put($uri=null, $data=null);


		public function delete($uri=null, $data=null);


		public function setUri($uri);


		public function getUri();


		public function setBaseUri($uri=null);


		public function setMethod($method);


		public function setTimeOut($time);


		public function send($uri=null);

	}
}
