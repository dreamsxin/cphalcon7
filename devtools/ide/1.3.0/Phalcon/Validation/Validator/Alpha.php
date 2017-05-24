<?php 

namespace Phalcon\Validation\Validator {

	/**
	 * Phalcon\Validation\Validator\Alpha
	 *
	 * Checks if a value has a correct ALPHA format
	 *
	 *<code>
	 *use Phalcon\Validation\Validator\Alpha as AlphaValidator;
	 *
	 *$validator->add('alpha', new AlphaValidator(array(
	 *   'message' => 'The alpha is not valid'
	 *)));
	 *</code>
	 */
	
	class Alpha extends \Phalcon\Validation\Validator implements \Phalcon\Validation\ValidatorInterface {

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
