<?php 

namespace Phalcon\Http {

	/**
	 * Phalcon\Http\Parser
	 *
	 *<code>
	 *	$parser = new Phalcon\Http\Parser(Phalcon\Http\Parser::TYPE_BOTH);
	 *  $result = $parser->execute($body);
	 *</code>
	 */
	
	class Parser {

		const TYPE_REQUEST = 0;

		const HTTP_RESPONSE = 1;

		const TYPE_BOTH = 2;

		protected $_type;

		/**
		 * \Phalcon\Http\Parser constructor
		 *
		 * @param int $type
		 */
		public function __construct($type=null){ }


		/**
		 * \Phalcon\Http\Parser constructor
		 *
		 * @param string $body http message.
		 * @return array|boolean $result
		 */
		public function execute($body){ }

	}
}
