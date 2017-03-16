<?php

class Songs extends Phalcon\Mvc\Model
{
	public function validation()
	{
		$validation = new Phalcon\Validation();
		$validation->add('albums_id', new Phalcon\Validation\Validator\Digit);
		$validation->add('name', new Phalcon\Validation\Validator\StringLength(array(
			'min' => 12,
		)));

		return $this->validate($validation);
	}

	public function getLabel($field)
	{
		$labels = \Phalcon\Kernel::message(__DIR__.'/../messages/songs', 'labels', NULL, NULL, TRUE);
		return \Phalcon\Arr::get($labels, $field);
	}
}
