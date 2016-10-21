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
		 * Returns the registered validators
		 *
		 * @return \Phalcon\Validation\Message\Group
		 */
		public function getMessages();

	}
}
