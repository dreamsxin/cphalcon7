<?php 

namespace Phalcon\Validation\Validator {

	/**
	 * Phalcon\Validation\Validator\Alnum
	 *
	 * Checks if a value has a correct ALNUM format
	 *
	 *<code>
	 *use Phalcon\Validation\Validator\Alnum as AlnumValidator;
	 *
	 *$validator->add('alnum', new AlnumValidator(array(
	 *   'message' => 'The alnum is not valid'
	 *)));
	 *</code>
	 */
	
	class Alnum extends \Phalcon\Validation\Validator implements \Phalcon\Validation\ValidatorInterface {

		/**
		 * Executes the validation
		 *
		 * @param \Phalcon\Validation $validaton
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
