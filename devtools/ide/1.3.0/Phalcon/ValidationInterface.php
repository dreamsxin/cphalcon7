<?php 

namespace Phalcon {

	/**
	 * Phalcon\Mvc\Model\ValidatorInterface initializer
	 */
	
	interface ValidationInterface {

		/**
		 * Validate a set of data according to a set of rules
		 *
		 * @param array|object $data
		 * @param object $entity
		 * @return \Phalcon\Validation\Message\Group
		 */
		public function validate($data, $entity=null);


		/**
		 * Adds a validator to a field
		 *
		 * @param string $attribute
		 * @param \Phalcon\Validation\ValidatorInterface
		 * @return \Phalcon\Validation
		 */
		public function add($attribute, $validator);


		/**
		 * Get label for field
		 *
		 * @param string|array field
		 * @return string
		 */
		public function getLabel($field);


		/**
		 * Gets the a value to validate in the array/object data source
		 *
		 * @param string $attribute
		 * @return mixed
		 */
		public function getValue($attribute, $entity=null);


		public function appendMessage(\Phalcon\Validation\MessageInterface $message);


		/**
		 * Returns the registered validators
		 *
		 * @return \Phalcon\Validation\Message\Group
		 */
		public function getMessages();

	}
}
