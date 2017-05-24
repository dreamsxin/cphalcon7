<?php 

namespace Phalcon {

	class Kernel {

		const EXT = php;

		protected static $_defaultMessages;

		protected static $_basePath;

		protected static $_messages;

		protected static $_messagesDir;

		protected static $_alias;

		protected static $_aliasDir;

		public static function preComputeHashKey($arrKey){ }


		public static function preComputeHashKey32($arrKey){ }


		public static function preComputeHashKey64($arrKey){ }


		public static function setBasePath($path){ }


		public static function setMessagesDir($path){ }


		public static function message($file, $key_path=null, $default_value=null, $ext=null, $absolute_path=null){ }


		public static function setMessages($messages){ }


		public static function getMessages(){ }


		public static function setAliasDir($path){ }


		public static function alias($file, $str, $ext=null, $absolute_path=null){ }


		public static function setAlias($alias){ }


		public static function getAlias(){ }

	}
}
