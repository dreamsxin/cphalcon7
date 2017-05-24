<?php 

namespace Phalcon\Validation\Validator {

	/**
	 * Phalcon\Validation\Validator\Date
	 *
	 * Checks if a value has a correct DATE format
	 *
	 *<code>
	 *use Phalcon\Validation\Validator\Date as DateValidator;
	 *
	 *$validator->add('date', new DateValidator(array(
	 *   'message' => 'The date is not valid'
	 *)));
	 *</code>
	 */
	
	class Date extends \Phalcon\Validation\Validator implements \Phalcon\Validation\ValidatorInterface {

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
