<?php 

namespace Phalcon\JsonRPc\Client {

	class Response {

		protected $_body;

		protected $_id;

		protected $_data;

		protected $_error;

		protected $_code;

		public function __construct(){ }


		public function setId($id){ }


		public function getId(){ }


		public function setResult($data){ }


		public function getResult(){ }


		public function setError($error){ }


		public function getError(){ }


		public function setCode($code){ }


		public function getCode(){ }

	}
}
