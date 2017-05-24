<?php 

namespace Phalcon\Image\Adapter {

	/**
	 * Phalcon\Image\Adapter\Imagick
	 *
	 * Image manipulation support. Allows images to be resized, cropped, etc.
	 *
	 *<code>
	 *	$image = new Phalcon\Image\Adapter\Imagick("upload/test.jpg");
	 *	$image->resize(200, 200)->rotate(90)->crop(100, 100);
	 *	if ($image->save()) {
	 *		echo 'success';
	 *	}
	 *</code>
	 */
	
	class Imagick extends \Phalcon\Image\Adapter implements \Phalcon\Image\AdapterInterface {

		protected static $_version;

		protected static $_checked;

		/**
		 * Checks if Imagick is enabled
		 *
		 * @return  boolean
		 */
		public static function check(){ }


		/**
		 * \Phalcon\Image\Imagick constructor
		 *
		 * @param string $file
		 */
		public function __construct($file, $width=null, $height=null){ }


		/**
		 * Execute a resize.
		 *
		 * @param int $width
		 * @param int $height
		 */
		protected function _resize($width, $height){ }


		/**
		 * This method scales the images using liquid rescaling method. Only support Imagick
		 *
		 * @param int $width   new width
		 * @param int $height  new height
		 * @param int $delta_x How much the seam can traverse on x-axis. Passing 0 causes the seams to be straight.
		 * @param int $rigidity Introduces a bias for non-straight seams. This parameter is typically 0.
		 */
		protected function _liquidRescale($width, $height, $delta_x, $regidity){ }


		/**
		 * Execute a crop.
		 *
		 * @param int $width
		 * @param int $height
		 * @param int $offset_x
		 * @param int $offset_y
		 */
		protected function _crop($width, $height, $offset_x, $offset_y){ }


		/**
		 * Execute a rotation.
		 *
		 * @param int $degrees
		 */
		protected function _rotate($degrees){ }


		/**
		 * Execute a flip.
		 *
		 * @param int $direction
		 */
		protected function _flip($direction){ }


		/**
		 * Execute a sharpen.
		 *
		 * @param int $amount
		 */
		protected function _sharpen($amount){ }


		/**
		 * Execute a reflection.
		 *
		 * @param int $height
		 * @param int $opacity
		 * @param boolean $fade_in
		 */
		protected function _reflection($height, $opacity, $fade_in){ }


		/**
		 * Execute a watermarking.
		 *
		 * @param \Phalcon\Image\Adapter $watermark
		 * @param int $offset_x
		 * @param int $offset_y
		 * @param int $opacity
		 */
		protected function _watermark($watermark, $offset_x, $offset_y, $opacity){ }


		/**
		 * Execute a text
		 *
		 * @param string text
		 * @param int $offset_x
		 * @param int $offset_y
		 * @param int $opacity
		 * @param int $r
		 * @param int $g
		 * @param int $b
		 * @param int $size
		 * @param string $fontfile
		 */
		protected function _text($text, $offset_x, $offset_y, $opacity, $r, $g, $b, $size, $fontfile){ }


		/**
		 * Composite one image onto another
		 *
		 * @param \Phalcon\Image\Adapter $mask  mask Image instance
		 */
		protected function _mask($mask){ }


		/**
		 * Execute a background.
		 *
		 * @param int $r
		 * @param int $g
		 * @param int $b
		 * @param int $opacity
		 */
		protected function _background($r, $g, $b, $opacity){ }


		/**
		 * Blur image
		 *
		 * @param int $radius Blur radius
		 */
		protected function _blur($radius){ }


		/**
		 * Pixelate image
		 *
		 * @param int $amount amount to pixelate
		 */
		protected function _pixelate($amount){ }


		/**
		 * Execute a save.
		 *
		 * @param string $file
		 * @param int $quality
		 * @return boolean
		 */
		protected function _save($file, $opacity=null, $interlacing=null){ }


		/**
		 * Execute a render.
		 *
		 * @param string $type
		 * @param int $quality
		 * @return string
		 */
		protected function _render($type, $opacity=null, $interlacing=null){ }


		/**
		 * Destroys the loaded image to free up resources.
		 */
		public function __destruct(){ }


		/**
		 * Draws a line
		 *
		 * @param int $sx
		 * @param int $sy
		 * @param int $ex
		 * @param int $ey
		 * @param string $color
		 * @return \Phalcon\Image\Adapter\Imagick
		 */
		public function line($sx, $sy, $ex, $ey, $color=null){ }


		/**
		 * Draws a polygon
		 *
		 *<code>
		 * $coordinates = array( array( 'x' => 4, 'y' => 6 ), array( 'x' => 8, 'y' => 10 ) );
		 * $image->polygon($coordinates);
		 *</code>
		 *
		 * @param array $coordinates array of x and y
		 * @param string $color
		 * @return \Phalcon\Image\Adapter\Imagick
		 */
		public function polygon($coordinates, $color=null){ }


		public function shadow($color=null, $opacity=null, $sigma=null, $x=null, $y=null){ }


		public function getInternalImInstance(){ }


		public static function setResourceLimit($resource, $limit){ }


		/**
		 * Replicate Colorize function
		 *
		 * @param string $color a hex or rgb(a) color
		 * @param int $composition use imagicks constants here
		 * @return \Phalcon\Image\Adapter\Imagick
		 */
		public function colorize($color, $composition=null){ }


		/**
		 * Change the gamma of an image
		 *
		 * @param float $gamma normally between 0.8 and 2
		 * @param int $channel Use the Imagick constants for this
		 * @return \Phalcon\Image\Adapter\Imagick
		 */
		public function gamma($gamma, $channel=null){ }


		/**
		 * Replicate Photoshop's levels function
		 *
		 * @param float $gamma
		 * @param int $input_min between 0 and 255, same as photoshops
		 * @param int $input_max between 0 and 255, same as photoshops
		 * @param int $output_min between 0 and 255, same as photoshops
		 * @param int $output_max between 0 and 255, same as photoshops
		 * @param int $channel use imagemagicks constants
		 * @return \Phalcon\Image\Adapter\Imagick
		 */
		public function levels($gamma=null, $input_min=null, $input_max=null, $output_min=null, $output_max=null, $channel=null){ }


		/**
		 * Replicate brightness/contrast photoshop function
		 *
		 * Now this one is a bit of a pain. PHP's extension doesn't provide us with this handle (yet?)
		 * So we have to save the image to disk at this point, perform the function using the command line, and reload the image. yay.
		 *
		 * @param int $brightness this is should be -150 <= brightnes <= 150. 0 for no change.
		 * @param int $contrast this is should be -150 <= contrast <= 150. 0 for no change.
		 * @return \Phalcon\Image\Adapter\Imagick
		 */
		public function brightness_contrast($brightness, $contrast){ }


		/**
		 * Replicate HSL function
		 *
		 * Imagemagick calls this 'modulate
		 *
		 * @param int $hue -100 <= hue <= 100. 0 is no change.
		 * @param int $saturation -100 <= hue <= 100. 0 is no change.
		 * @param int $lightness -100 <= hue <= 100. 0 is no change.
		 * @return \Phalcon\Image\Adapter\Imagick
		 */
		public function hsl($hue=null, $saturation=null, $lightness=null){ }


		/**
		 * Perform an imagemagick-style function on each pixel
		 *
		 * @param string $fx the function
		 * @return \Phalcon\Image\Adapter\Imagick
		 */
		public function curves_graph($fx){ }


		/**
		 * Adds a vignette to the image
		 *
		 * @param string $color the colour of the vignette
		 * @param int $composition an imagick constant defining the composition to use
		 * @param float $crop_factor defines the strenth of the vignette
		 * @return \Phalcon\Image\Adapter\Imagick
		 */
		public function vignette($color, $composition=null, $crop_factor=null){ }


		/**
		 * A sort-of sepia filter
		 *
		 * @return \Phalcon\Image\Adapter\Imagick
		 */
		public function earlybird(){ }


		/**
		 * A black and white filter
		 *
		 * @return \Phalcon\Image\Adapter\Imagick
		 */
		public function inkwell(){ }


		public static function convert($command){ }

	}
}
