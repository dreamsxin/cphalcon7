<?php 

namespace Phalcon {

	/**
	 * Phalcon\Image
	 *
	 * Image manipulation support. Allows images to be resized, cropped, etc.
	 *
	 *<code>
	 *	$image = Phalcon\Image::factory("upload/test.jpg");
	 *	$image->resize(200, 200);
	 *	$image->save();
	 *</code>
	 */
	
	abstract class Image {

		const NONE = 1;

		const WIDTH = 2;

		const HEIGHT = 3;

		const AUTO = 4;

		const INVERSE = 5;

		const PRECISE = 6;

		const TENSILE = 7;

		const NARROW = 8;

		const HORIZONTAL = 11;

		const VERTICAL = 12;

		const GD = 21;

		const IMAGICK = 22;

		public static function factory($file, $width=null, $height=null){ }

	}
}
