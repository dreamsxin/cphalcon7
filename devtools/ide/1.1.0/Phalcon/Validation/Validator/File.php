<?php 

namespace Phalcon\Validation\Validator {

	/**
	 * Phalcon\Validation\Validator\File
	 *
	 * Checks if a value has a correct FILE format
	 *
	 *<code>
	 *use Phalcon\Validation\Validator\File as FileValidator;
	 *
	 *$validator->add('file', new FileValidator(array(
	 *	 'mimes' => array('image/png', 'image/gif'),
	 *	 'minsize' => 100,
	 *	 'maxsize' => 10000,
	 *   'message' => 'The file is not valid'
	 *)));
	 *</code>
	 */
	
	class File extends \Phalcon\Validation\Validator implements \Phalcon\Validation\ValidatorInterface {

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
		 * @param string $file
		 * @param int $minsize
		 * @param int $maxsize
		 * @param array $mimes
		 * @param int $minwidth
		 * @param int $maxwidth
		 * @param int $minheight
		 * @param int $maxheight
		 * @return boolean
		 */
		public function valid(){ }

	}
}
