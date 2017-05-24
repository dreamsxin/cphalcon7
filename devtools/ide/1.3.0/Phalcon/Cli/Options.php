<?php 

namespace Phalcon\Cli {

	/**
	 * Phalcon\Cli\Options
	 *
	 *<code>
	 *
	 * $ops = new \Phalcon\Cli\Options('Phalcon CLI');
	 *  $ops->add([
	 * 	'type' => \Phalcon\Cli\Options::TYPE_INT,
	 * 	'name' => 'min'
	 * ]);
	 *  $ops->add([
	 * 	'type' => \Phalcon\Cli\Options::TYPE_INT,
	 * 	'name' => 'max',
	 * 	'shortName' => 'm',
	 * 	'required' => false,
	 * 	'desc' => "int",
	 * 	'help' => "must be int",
	 * 	'defaultValue' => 1
	 * ]);
	 * $ops->add(\Phalcon\Cli\Options::TYPE_STRING, 'name', 'n', true, "name", "must be string", "Phalcon");
	 * $values = $ops->parse();
	 *
	 *</code>
	 */
	
	class Options extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface {

		const TYPE_ANY = 0;

		const TYPE_INT = 1;

		const TYPE_FLOAT = 2;

		const TYPE_BOOLEAN = 3;

		const TYPE_STRING = 4;

		const TYPE_ARRAY = 5;

		protected $_title;

		protected $_program;

		protected $_argString;

		protected $_desc;

		protected $_types;

		protected $_options;

		protected $_longopts;

		protected $_descs;

		protected $_helps;

		protected $_required;

		protected $_defaultValues;

		protected $_names;

		protected $_shortNames;

		/**
		 * \Phalcon\Cli\Options constructor
		 */
		public function __construct($title=null, $program=null, $argString=null, $desc=null, $options=null, $dependencyInjector=null){ }


		/**
		 * Add option
		 */
		public function add($arg, $name=null, $shortname=null, $required=null, $desc=null, $help=null, $defaultValue=null){ }


		/**
		 * Print help
		 */
		public function help(){ }


		/**
		 * Parse and return values
		 */
		public function parse($options=null){ }


		public function addOption($arg, $name=null, $shortname=null, $required=null, $desc=null, $help=null, $defaultValue=null){ }

	}
}
