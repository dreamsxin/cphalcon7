<?php 

namespace Phalcon\Chart {

	/**
	 * Phalcon\Chart\QRcode
	 * 
	 *<code>
	 * $qr = new \Phalcon\Chart\QRcode();
	 * $ret = $qr->generate('Phalcon framework');
	 * $data = $qr->render();
	 * $data = $qr->render(NULL, NULL, 'FFCC00', '000000');
	 * $ret = $qr->save('unit-tests/assets/qr.png');
	 * $ret = $qr->save('unit-tests/assets/qr.png', NULL, NULL, 'FFCC00', '000000');
	 * $ret = $qr->scan('unit-tests/assets/qr.png');
	 *</code>
	 */
	
	class QRcode {

		const MODE_NUL = -1;

		const MODE_NUM = 0;

		const MODE_8 = 2;

		const MODE_KANJI = 3;

		const LEVEL_L = 0;

		const LEVEL_M = 1;

		const LEVEL_Q = 2;

		const LEVEL_H = 3;

		protected $_qr;

		protected $_text;

		protected $_version;

		protected $_level;

		protected $_mode;

		protected $_casesensitive;

		/**
		 * \Phalcon\Chart\QRcode constructor
		 *
		 *     $qr = new \Phalcon\Chart\QRcode;
		 *     $qr->generate('Phalcon is a web framework', 4, \Phalcon\Chart\QRcode::LEVEL_L, \Phalcon\Chart\QRcode::MODE_KANJI, TRUE);
		 *     $qr->save('qr.png');
		 */
		public function __construct(){ }


		/**
		 * Generate QR data
		 *
		 * @param string $text
		 * @param int $version
		 * @param int $level
		 * @param int $mode
		 * @param boolean $casesensitive
		 * @return boolean
		 */
		public function generate($data, $version=null, $level=null, $mode=null, $casesensitive=null){ }


		/**
		 * Render the image and return the binary string.
		 *
		 *     $qr = new \Phalcon\Chart\QRcode;
		 *     $qr->generate('Phalcon is a web framework');
		 *     $data = \Phalcon\Chart\QRcode::render();
		 *
		 * @param int $size
		 * @param int $margin.
		 * @param string $foreground
		 * @param string $background
		 * @return string
		 */
		public function render($size=null, $margin=null, $foreground=null, $background=null){ }


		/**
		 * Save the image
		 *
		 *     $qr = new \Phalcon\Chart\QRcode;
		 *     $qr->generate('Phalcon is a web framework', 4, \Phalcon\Chart\QRcode::LEVEL_L, \Phalcon\Chart\QRcode::MODE_KANJI, TRUE);
		 *     $qr->save('qr.png');
		 *
		 * @param filename $filename
		 * @param size $size
		 * @param margin $margin.
		 * @return boolean
		 */
		public function save($filename=null, $size=null, $margin=null, $foreground=null, $background=null){ }


		/**
		 * Scan the image.
		 *
		 *     $qr = new \Phalcon\Chart\QRcode;
		 *     $ret = $qr->san('qr.png');
		 *
		 * @param string filename
		 * @return string
		 */
		public function scan($filename){ }

	}
}
