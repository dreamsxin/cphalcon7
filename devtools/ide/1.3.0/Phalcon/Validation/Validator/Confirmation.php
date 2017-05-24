<?php 

namespace Phalcon\Validation\Validator {

	/**
	 * Phalcon\Validation\Validator\Confirmation
	 *
	 * Checks that two values have the same value
	 *
	 *<code>
	 *use Phalcon\Validation\Validator\Confirmation;
	 *
	 *$validator->add('password', new Confirmation(array(
	 *   'message' => 'Password doesn\'t match confirmation',
	 *   'with' => 'confirmPassword'
	 *)));
	 *</code>
	 */
	
	class Confirmation extends \Phalcon\Validation\Validator implements \Phalcon\Validation\ValidatorInterface {

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
		 * @param string $with_value
		 * @return boolean
		 */
		public function valid(){ }

	}
}
