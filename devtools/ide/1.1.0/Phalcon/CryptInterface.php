<?php 

namespace Phalcon {

	/**
	 * Phalcon\CryptInterface initializer
	 */
	
	interface CryptInterface {

		/**
		 * Sets the cipher method
		 *
		 * @param string $cipher
		 * @return \Phalcon\CryptInterface
		 */
		public function setMethod($method);


		/**
		 * Gets the cipher method
		 *
		 * @return string
		 */
		public function getMethod();


		/**
		 * Sets the encryption key
		 *
		 * @param string $key
		 * @return \Phalcon\CryptInterface
		 */
		public function setKey($key);


		/**
		 * Returns the encryption key
		 *
		 * @return string
		 */
		public function getKey();


		/**
		 * Encrypts a text
		 *
		 * @param string $text
		 * @param string $key
		 * @return string
		 */
		public function encrypt($text, $key=null, $options=null);


		/**
		 * Decrypts a text
		 *
		 * @param string $text
		 * @param string $key
		 * @return string
		 */
		public function decrypt($text, $key=null, $options=null);


		/**
		 * Encrypts a text returning the result as a base64 string
		 *
		 * @param string $text
		 * @param string $key
		 * @param bool $url_safe
		 * @return string
		 */
		public function encryptBase64($text, $key=null, $safe=null);


		/**
		 * Decrypt a text that is coded as a base64 string
		 *
		 * @param string $text
		 * @param string $key
		 * @param bool $url_safe
		 * @return string
		 */
		public function decryptBase64($text, $key=null, $safe=null);


		/**
		 * Returns a list of available methods
		 *
		 * @return array
		 */
		public function getAvailableMethods();

	}
}
