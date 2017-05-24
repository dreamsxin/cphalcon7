<?php 

namespace Phalcon\Validation\Validator {

	/**
	 * Phalcon\Validation\Validator\Json
	 *
	 * Checks if a value has a correct JSON format
	 *
	 *<code>
	 *use Phalcon\Validation\Validator\Json as JsonValidator;
	 *
	 *$validator->add('json', new JsonValidator(array(
	 *   'keys' => array('name'),
	 *   'message' => 'The json is not valid'
	 *)));
	 *</code>
	 */
	
	class Json extends \Phalcon\Validation\Validator implements \Phalcon\Validation\ValidatorInterface {

		/**
		 * Executes the validation
		 *
		 * @param \Phalcon\Validation $validator
		 * @param string $attribute
		 * @return boolean
		 */
		public function validate(\Phalcon\ValidationInterface $validator, $attribute, $allowEmpty=null){ }


		/**
		 * Executes the validation
		 *
		 * @param string $value
		 * @return boolean
		 */
		public function valid(){ }

	}
}
