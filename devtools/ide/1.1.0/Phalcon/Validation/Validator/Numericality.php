<?php 

namespace Phalcon\Validation\Validator {

	/**
	 * Phalcon\Validation\Validator\Numericality
	 *
	 * Check for a valid numeric value
	 *
	 *<code>
	 *use Phalcon\Validation\Validator\Numericality;
	 *
	 *$validator->add('price', new Numericality(array(
	 *   'message' => ':field is not numeric'
	 *)));
	 *</code>
	 */
	
	class Numericality extends \Phalcon\Validation\Validator implements \Phalcon\Validation\ValidatorInterface {

		/**
		 * Executes the validation
		 *
		 * @param \Phalcon\Validation $validator
		 * @param string $attribute
		 * @return boolean
		 */
		public function validate($validator, $attribute){ }


		/**
		 * Executes the validation
		 *
		 * @param string $value
		 * @return boolean
		 */
		public function valid(){ }

	}
}
