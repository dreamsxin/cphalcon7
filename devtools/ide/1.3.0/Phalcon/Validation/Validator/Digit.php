<?php 

namespace Phalcon\Validation\Validator {

	/**
	 * Phalcon\Validation\Validator\Digit
	 *
	 * Checks if a value has a correct DIGIT format
	 *
	 *<code>
	 *use Phalcon\Validation\Validator\Digit as DigitValidator;
	 *
	 *$validator->add('digit', new DigitValidator(array(
	 *   'message' => 'The digit is not valid'
	 *)));
	 *</code>
	 */
	
	class Digit extends \Phalcon\Validation\Validator implements \Phalcon\Validation\ValidatorInterface {

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
