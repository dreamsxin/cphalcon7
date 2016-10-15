<?php

use Phalcon\Validation\Validator\PresenceOf as PresenceOfValidator;
use Phalcon\Validation\Validator\Email as EmailValidator;
use Phalcon\Validation\Validator\ExclusionIn as ExclusionInValidator;
use Phalcon\Validation\Validator\InclusionIn as InclusionInValidator;
use Phalcon\Validation\Validator\Uniqueness as UniquenessValidator;
use Phalcon\Validation\Validator\Regex as RegexValidator;
use Phalcon\Validation\Validator\StringLength as StringLengthValidator;

use Phalcon\Validation\Message as Message;

class Subscriptores extends Phalcon\Mvc\Model
{

	public function beforeValidation()
	{
		if ($this->email == 'marina@hotmail.com') {
			$this->appendMessage(new Message('Sorry Marina, but you are not allowed here'));
			return false;
		}
	}

	public function beforeDelete()
	{
		if ($this->email == 'fuego@hotmail.com') {
			$this->appendMessage(new Message('Sorry this cannot be deleted'));
			return false;
		}
	}

	public function validation()
	{
		$validation = new Phalcon\Validation();
		$validation->add('created_at', new PresenceOfValidator());
		$validation->add('email', new StringLengthValidator(array(
			'min' => '7',
			'max' => '50'
		)));
		$validation->add('email', new EmailValidator());
		$validation->add('status', new ExclusionInValidator(array(
			'domain' => array('X', 'Z')
		)));
		$validation->add('status', new InclusionInValidator(array(
			'domain' => array('P', 'I', 'w')
		)));
		$validation->add('email', new UniquenessValidator());
		$validation->add('status', new RegexValidator(array(
			'pattern' => '/[A-Z]/'
		)));

		return $this->validate($validation);
	}

}
