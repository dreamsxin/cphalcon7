<?php 

namespace Phalcon\Chart {

	/**
	 * Phalcon\Chart\Captcha
	 * 
	 *<code>
	 * header('Content-Type: image/png');
	 * $captcha = new \Phalcon\Chart\Captcha(NULL, NULL, 30, 150, 50);
	 * echo $captcha = $qr->render('Phalcon', 15, -10);
	 *</code>
	 */
	
	class Captcha {

		protected $_imagick;

		protected $_draw;

		protected $_word;

		protected $_font;

		protected $_fontSize;

		protected $_width;

		protected $_height;

		/**
		 * \Phalcon\Chart\Captcha constructor
		 *
		 *     $qr = new \Phalcon\Chart\Captcha;
		 *     $qr->generate('Phalcon is a web framework');
		 *     $qr->save('qr.png');
		 */
		public function __construct(){ }


		/**
		 * Sets a font
		 *
		 * @param string $font
		 * @return boolean
		 */
		public function setFont($font){ }


		/**
		 * Sets a font size
		 *
		 * @param string $fontSize
		 * @return boolean
		 */
		public function setFontSize($size){ }


		/**
		 * Generate Captcha data
		 *
		 *<code>
		 *     $qr = new \Phalcon\Chart\Captcha;
		 *     $qr->reander('Phalcon is a web framework');
		 *</code>
		 *
		 * @param string $word
		 * @param string $margin
		 * @return String
		 */
		public function render($word, $margin=null, $foreground=null, $background=null){ }


		/**
		 * Save the image
		 *
		 *<code>
		 *     $qr = new \Phalcon\Chart\Captcha;
		 *     $qr->reander('Phalcon is a web framework');
		 *     $qr->save('captcha.png');
		 *</code>
		 *
		 * @param filename $filename
		 * @return boolean
		 */
		public function save($filename=null){ }

	}
}
