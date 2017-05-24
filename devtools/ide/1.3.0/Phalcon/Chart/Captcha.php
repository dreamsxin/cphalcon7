<?php 

namespace Phalcon\Chart {

	/**
	 * Phalcon\Chart\Captcha
	 *
	 *<code>
	 * header('Content-Type: image/png');
	 * $captcha = new \Phalcon\Chart\Captcha(Phalcon\Text::random(Phalcon\Text::RANDOM_ALNUM, 4), NULL, 30, 150, 50);
	 * echo $captcha = $captcha->render();
	 *</code>
	 */
	
	class Captcha {

		const PAD_BOTH = 0;

		const PAD_LEFT = 1;

		const PAD_RIGHT = 2;

		protected $_imagick;

		protected $_word;

		protected $_font;

		protected $_fontSize;

		protected $_width;

		protected $_height;

		protected $_padType;

		protected $_padSize;

		/**
		 * \Phalcon\Chart\Captcha constructor
		 *
		 *     $captcha = new \Phalcon\Chart\Captcha;
		 *     $captcha->generate('Phalcon is a web framework');
		 *     $captcha->save('qr.png');
		 */
		public function __construct($word=null, $font=null, $fontSize=null, $width=null, $height=null, $pad_size=null, $pad_type=null){ }


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
		 * @param int $fontSize
		 * @return boolean
		 */
		public function setFontSize($size){ }


		/**
		 * Generate Captcha data
		 *
		 *<code>
		 *     $captcha = new \Phalcon\Chart\Captcha;
		 *     $captcha->reander('Phalcon is a web framework');
		 *</code>
		 *
		 * @param string $word
		 * @param string $margin
		 * @return String
		 */
		public function render($filename=null, $word=null, $offset_x=null, $offset_y=null, $foreground=null, $background=null, $width=null, $height=null){ }


		/**
		 * Save the image
		 *
		 *<code>
		 *     $captcha = new \Phalcon\Chart\Captcha;
		 *     $captcha->reander('Phalcon is a web framework');
		 *     $captcha->save('captcha.png');
		 *</code>
		 *
		 * @param filename $filename
		 * @return boolean
		 */
		public function save($filename=null){ }

	}
}
