<?php 

namespace Phalcon\Validation\Validator {

	/**
	 * Phalcon\Validation\Validator\Uniqueness
	 *
	 * Check that a field is unique in the related table
	 *
	 *<code>
	 *use Phalcon\Validation\Validator\Uniqueness as UniquenessValidator;
	 *
	 *$validator->add('name', new UniquenessValidator(array(
	 *   'model' => new Users(),
	 *   'message' => ':field must be unique'
	 *)));
	 *</code>
	 */
	
	class Uniqueness extends \Phalcon\Validation\Validator implements \Phalcon\Validation\ValidatorInterface {

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
		 * @param \Phalcon\Mvc\ModelInterface $record
		 * @param array $values
		 * @return boolean
		 */
		public function valid(){ }

	}
}
