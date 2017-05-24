<?php 

namespace Phalcon {

	/**
	 * Phalcon\Random
	 *
	 *<code>
	 *  $random = new \Phalcon\Random();
	 *
	 *  // Random alnum string
	 *  $alnum = $random->alnum();
	 */
	
	class Random {

		const COLOR_RGB = 0;

		const COLOR_RGBA = 1;

		/**
		 * Generates a random alnum string
		 *
		 *<code>
		 *  $random = new \Phalcon\Random();
		 *  $alnum = $random->alnum();
		 *</code>
		 */
		public function alnum($len=null){ }


		/**
		 * Generates a random alpha string
		 *
		 *<code>
		 *  $random = new \Phalcon\Random();
		 *  $alpha = $random->alpha();
		 *</code>
		 */
		public function alpha($len=null){ }


		/**
		 * Generates a random hexdec string
		 *
		 *<code>
		 *  $random = new \Phalcon\Random();
		 *  $hexdec = $random->hexdec();
		 *</code>
		 */
		public function hexdec($len=null){ }


		/**
		 * Generates a random numeric string
		 *
		 *<code>
		 *  $random = new \Phalcon\Random();
		 *  $numeric = $random->numeric();
		 *</code>
		 */
		public function numeric($len=null){ }


		/**
		 * Generates a random nozero numeric string
		 *
		 *<code>
		 *  $random = new \Phalcon\Random();
		 *  $bytes = $random->nozero();
		 *</code>
		 */
		public function nozero($len=null){ }


		/**
		 * Generates a random color
		 */
		public function color($type=null){ }

	}
}
