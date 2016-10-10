<?php

use Phalcon\Validation\Validator\PresenceOf as PresenceOfValidator;
use Phalcon\Validation\Validator\Email as EmailValidator;
use Phalcon\Validation\Validator\ExclusionIn as ExclusionInValidator;
use Phalcon\Validation\Validator\InclusionIn as InclusionInValidator;
use Phalcon\Validation\Validator\Uniqueness as UniquenessValidator;
use Phalcon\Validation\Validator\Regex as RegexValidator;
use Phalcon\Validation\Validator\StringLength as StringLengthValidator;

use Phalcon\Validation\Message as Message;

/**
 * Abonnes
 *
 * Abonnes is subscriptors in french
 */
class Abonnes extends Phalcon\Mvc\Model
{

	public function getSource()
	{
		return 'subscriptores';
	}

	public function beforeValidation()
	{
		if ($this->courrierElectronique == 'marina@hotmail.com') {
			$this->appendMessage(new Message('Désolé Marina, mais vous n\'êtes pas autorisé ici'));
			return false;
		}
	}

	public function beforeDelete()
	{
		if ($this->courrierElectronique == 'fuego@hotmail.com') {
			//Sorry this cannot be deleted
			$this->appendMessage(new Message('Désolé, ce ne peut pas être supprimé'));
			return false;
		}
	}

	public function validation()
	{
		$validation = new Phalcon\Validation();
		$validation->add('creeA', new PresenceOfValidator(array(
			'message' => "La date de création est nécessaire"
		)));
		$validation->add('courrierElectronique', new EmailValidator(array(
			'message' => 'Le courrier électronique est invalide'
		)));
		$validation->add('statut', new ExclusionInValidator(array(
			'domain' => array('X', 'Z'),
			'message' => 'L\'état ne doit être "X" ou "Z"'
		)));
		$validation->add('statut', new InclusionInValidator(array(
			'domain' => array('P', 'I', 'w'),
			'message' => 'L\'état doit être "P", "I" ou "w"'
		)));
		$validation->add('courrierElectronique', new UniquenessValidator(array(
			'message' => 'Le courrier électronique doit être unique'
		)));
		$validation->add('statut', new RegexValidator(array(
			'pattern' => '/[A-Z]/',
			'message' => "L'état ne correspond pas à l'expression régulière"
		)));
		$validation->add('courrierElectronique', new StringLengthValidator(array(
			'min' => '7',
			'max' => '50',
			'messageMinimum' => "Le courrier électronique est trop court",
			'messageMaximum' => "Le courrier électronique est trop long"
		)));
		
		return $this->validate($validation);
	}

	public function columnMap()
	{
		return array(
			'id' => 'code',
			'email' => 'courrierElectronique',
			'created_at' => 'creeA',
			'status' => 'statut',
		);
	}

}
