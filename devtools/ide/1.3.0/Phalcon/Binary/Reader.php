<?php 

namespace Phalcon\Binary {

	/**
	 * Phalcon\Binary\Reader
	 *
	 * Provides utilities to work with binary data
	 *
	 *<code>
	 *	$fp = fopen('unit-tests/assets/data.bin', 'rb');
	 *	$bin = new Phalcon\Binary\Reader($fp);
	 *	$v = $bin->readUnsignedChar();
	 *</code>
	 */
	
	class Reader {

		protected $_endian;

		protected $_data;

		protected $_input;

		protected $_position;

		protected $_eofPosition;

		/**
		 * \Phalcon\Binary\Reader constructor
		 *
		 * @param  string|resource $data
		 * @param  int $endian
		 * @throws \InvalidArgumentException
		 */
		public function __construct($data, $endian=null){ }


		/**
		 * Gets the endian
		 *
		 * @return int
		 */
		public function getEndian(){ }


		/**
		 * Gets the input
		 *
		 * @return int
		 */
		public function getInput(){ }


		/**
		 * Gets the binary data
		 *
		 * @return int
		 */
		public function getContent(){ }


		/**
		 * Sets the position in the the file pointer
		 *
		 * @param integer $position
		 * @param integer $whence
		 * @return boolean
		 */
		public function setPosition($position, $whence=null){ }


		/**
		 * Gets the current postion in the the file pointer
		 *
		 * @return int
		 */
		public function getPosition(){ }


		/**
		 * Gets the eof postion the file pointer
		 *
		 * @return int
		 */
		public function getEofPosition(){ }


		/**
		 * Checks if end of the file pointer was reached
		 *
		 * @return boolean
		 */
		public function isEof(){ }


		/**
		 * Read num bytes from the current position in the file pointer
		 *
		 * @param integer $length
		 * @return string
		 */
		public function read($length){ }


		/**
		 * Read a signed char from the current position in the file pointer
		 *
		 * @return string
		 */
		public function readChar(){ }


		/**
		 * Read a unsigned char from the current position in the file pointer
		 *
		 * @return string
		 */
		public function readUnsignedChar(){ }


		/**
		 * Read a signed short int from the current position in the file pointer
		 *
		 * @return int
		 */
		public function readInt16(){ }


		/**
		 * Read a unsigned short int from the current position in the file pointer
		 *
		 * @return int
		 */
		public function readUnsignedInt16($endian=null){ }


		/**
		 * Read a signed int from the current position in the file pointer
		 *
		 * @return int
		 */
		public function readInt(){ }


		/**
		 * Read a unsigned int from the current position in the file pointer
		 *
		 * @return int
		 */
		public function readUnsignedInt(){ }


		/**
		 * Read a signed long int from the current position in the file pointer
		 *
		 * @return int
		 */
		public function readInt32(){ }


		/**
		 * Read a unsigned long int from the current position in the file pointer
		 *
		 * @return int
		 */
		public function readUnsignedInt32($endian=null){ }


		/**
		 * Read a float from the current position in the file pointer
		 *
		 * @return float
		 */
		public function readFloat(){ }


		/**
		 * Read a double from the current position in the file pointer
		 *
		 * @return double
		 */
		public function readDouble(){ }


		/**
		 * Read string from the current position in the file pointer
		 *
		 * @return string
		 */
		public function readString($length=null){ }


		/**
		 * Read hex string from the current position in the file pointer
		 *
		 * @return string
		 */
		public function readHexString($length=null, $lowNibble=null){ }

	}
}
