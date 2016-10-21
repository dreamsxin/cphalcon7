<?php 

namespace Phalcon {

	/**
	 * Phalcon\Crypt
	 *
	 * Provides encryption facilities to phalcon applications
	 *
	 *<code>
	 *	$crypt = new Phalcon\Crypt();
	 *
	 *	$key = 'le password';
	 *	$text = 'This is a secret text';
	 *
	 *	$encrypted = $crypt->encrypt($text, $key);
	 *
	 *	echo $crypt->decrypt($encrypted, $key);
	 *</code>
	 */
	
	class Crypt implements \Phalcon\CryptInterface {

		protected $_key;

		protected $_method;

		protected $_options;

		protected $_beforeEncrypt;

		protected $_afterEncrypt;

		protected $_beforeDecrypt;

		protected $_afterDecrypt;

		/**
		 * Sets the cipher method
		 *
		 * @param string $method
		 * @return \Phalcon\Encrypt
		 */
		public function setMethod($method){ }


		/**
		 * Returns the current cipher method
		 *
		 * @return string
		 */
		public function getMethod(){ }


		/**
		 * Sets the encryption key
		 *
		 * @param string $key
		 * @return \Phalcon\Encrypt
		 */
		public function setKey($key){ }


		/**
		 * Returns the encryption key
		 *
		 * @return string
		 */
		public function getKey(){ }


		/**
		 * Encrypts a text
		 *
		 *<code>
		 *	$encrypted = $crypt->encrypt("Ultra-secret text", "encrypt password");
		 *</code>
		 *
		 * @param string $text
		 * @param string $key
		 * @return string
		 */
		public function encrypt($text, $key=null, $options=null){ }


		/**
		 * Decrypts an encrypted text
		 *
		 *<code>
		 *	echo $crypt->decrypt($encrypted, "decrypt password");
		 *</code>
		 *
		 * @param string $text
		 * @param string $key
		 * @return string
		 */
		public function decrypt($text, $key=null, $options=null){ }


		/**
		 * Encrypts a text returning the result as a base64 string
		 *
		 * @param string $text
		 * @param string $key
		 * @return string
		 */
		public function encryptBase64($text, $key=null, $safe=null){ }


		/**
		 * Decrypt a text that is coded as a base64 string
		 *
		 * @param string $text
		 * @param string $key
		 * @return string
		 */
		public function decryptBase64($text, $key=null, $safe=null){ }


		/**
		 * Returns a list of available modes
		 *
		 * @return array
		 */
		public function getAvailableMethods(){ }


		/**
		 * Adds a internal hooks before encrypts a text
		 *
		 * @param callable $handler
		 */
		public function beforeEncrypt($handler){ }


		/**
		 * Adds a internal hooks after encrypts a text
		 *
		 * @param callable $handler
		 */
		public function afterEncrypt($handler){ }


		/**
		 * Adds a internal hooks before decrypts an encrypted text
		 *
		 * @param callable $handler
		 */
		public function beforeDecrypt($handler){ }


		/**
		 * Adds a internal hooks after decrypts an encrypted text
		 *
		 * @param callable $handler
		 */
		public function afterDecrypt($handler){ }

	}
}
