<?php 

namespace Phalcon\Binary {

	/**
	 * Phalcon\Binary\Writer
	 *
	 * Provides utilities to work with binary data
	 *
	 *<code>
	 *	$fp = fopen('unit-tests/assets/data.bin', 'wb');
	 *	$bin = new Phalcon\Binary\Writer($fp);
	 *	$bin->writeUnsignedChar(1);
	 *</code>
	 */
	
	class Writer {

		protected $_endian;

		protected $_output;

		protected $_position;

		/**
		 * \Phalcon\Binary\Writer constructor
		 *
		 * @param  string|resource $data
		 * @param  int $endian
		 * @throws \InvalidArgumentException
		 */
		public function __construct($data=null, $endian=null){ }


		/**
		 * Gets the endian
		 *
		 * @return int
		 */
		public function getEndian(){ }


		/**
		 * Gets the ouput
		 *
		 * @return int
		 */
		public function getOutput(){ }


		/**
		 * Gets the ouput
		 *
		 * @return int
		 */
		public function getContent(){ }


		/**
		 * Gets the current postion
		 *
		 * @return int
		 */
		public function getPosition(){ }


		/**
		 * Write bytes to the current position in the file pointer
		 *
		 * @return \Phalcon\Binary\Writer
		 */
		public function write($data, $length){ }


		/**
		 * Write a signed char to the current position in the file pointer
		 *
		 * @return \Phalcon\Binary\Writer
		 */
		public function writeChar($byte){ }


		/**
		 * Write a unsigned char to the current position in the file pointer
		 *
		 * @return \Phalcon\Binary\Writer
		 */
		public function writeUnsignedChar($byte){ }


		/**
		 * Write a signed short int to the current position in the file pointer
		 *
		 * @return \Phalcon\Binary\Writer
		 */
		public function writeInt16($num){ }


		/**
		 * Write a unsigned short int to the current position in the file pointer
		 *
		 * @return \Phalcon\Binary\Writer
		 */
		public function writeUnsignedInt16($num, $endian=null){ }


		/**
		 * Write a signed int to the current position in the file pointer
		 *
		 * @return \Phalcon\Binary\Writer
		 */
		public function writeInt($num){ }


		/**
		 * Write a unsigned int to the current position in the file pointer
		 *
		 * @return \Phalcon\Binary\Writer
		 */
		public function writeUnsignedInt($num){ }


		/**
		 * Write a signed long int to the current position in the file pointer
		 *
		 * @return \Phalcon\Binary\Writer
		 */
		public function writeInt32($num){ }


		/**
		 * Write a unsigned long int to the current position in the file pointer
		 *
		 * @return \Phalcon\Binary\Writer
		 */
		public function writeUnsignedInt32($num, $endian=null){ }


		/**
		 * Write a float to the current position in the file pointer
		 *
		 * @return \Phalcon\Binary\Writer
		 */
		public function writeFloat($num){ }


		/**
		 * Write a double to the current position in the file pointer
		 *
		 * @return \Phalcon\Binary\Writer
		 */
		public function writeDouble($num){ }


		/**
		 * Write string to the current position in the file pointer
		 *
		 * @return \Phalcon\Binary\Writer
		 */
		public function writeString($str, $length=null, $exact=null){ }


		/**
		 * Write hex string to the current position in the file pointer
		 *
		 * @return \Phalcon\Binary\Writer
		 */
		public function writeHexString($str, $length=null, $lowNibble=null){ }

	}
}
