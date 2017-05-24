<?php 

namespace Phalcon\Cli {

	/**
	 * Phalcon\Cli\Color
	 *
	 *<code>
	 *
	 *	echo Phalcon\Cli\Color::head("head");
	 *	echo Phalcon\Cli\Color::error("error");
	 *	echo Phalcon\Cli\Color::success("success");
	 *	echo Phalcon\Cli\Color::info("info");
	 *</code>
	 */
	
	class Color {

		const FG_BLACK = 0;

		const FG_DARK_GRAY = 1;

		const FG_RED = 2;

		const FG_LIGHT_RED = 3;

		const FG_GREEN = 4;

		const FG_LIGHT_GREEN = 5;

		const FG_BROWN = 6;

		const FG_YELLOW = 7;

		const FG_BLUE = 8;

		const FG_LIGHT_BLUE = 9;

		const FG_PURPLE = 10;

		const FG_LIGHT_PURPLE = 11;

		const FG_CYAN = 12;

		const FG_LIGHT_CYAN = 13;

		const FG_LIGHT_GRAY = 14;

		const FG_WHITE = 15;

		const BG_BLACK = 0;

		const BG_RED = 1;

		const BG_GREEN = 2;

		const BG_YELLOW = 3;

		const BG_BLUE = 4;

		const BG_MAGENTA = 5;

		const BG_CYAN = 6;

		const BG_LIGHT_GRAY = 7;

		const AT_NORMAL = 0;

		const AT_BOLD = 1;

		const AT_ITALIC = 2;

		const AT_UNDERLINE = 3;

		const AT_BLINK = 4;

		const AT_OUTLINE = 5;

		const AT_REVERSE = 6;

		const AT_NONDISP = 7;

		const AT_STRIKE = 8;

		protected static $_fg;

		protected static $_bg;

		protected static $_at;

		/**
		 * Identify if console supports colors
		 *
		 * @return boolean
		 */
		public static function isSupportedShell(){ }


		/**
		 * Colorizes the string using provided colors.
		 *
		 * @param string $str
		 * @param null|integer $fg
		 * @param null|integer $at
		 * @param null|integer $bg
		 * @return string
		 */
		public static function colorize($str, $fg=null, $at=null, $bg=null){ }


		/**
		 * Color style for head messages.
		 *
		 * @param string $str
		 * @return string
		 */
		public static function head($str){ }


		/**
		 * Color style for error messages.
		 *
		 * @param string $str
		 * @return string
		 */
		public static function error($str){ }


		/**
		 * Color style for success messages.
		 *
		 * @param string $str
		 * @return string
		 */
		public static function success($str){ }


		/**
		 * Color style for info messages.
		 *
		 * @param string $str
		 * @return string
		 */
		public static function info($str){ }

	}
}
