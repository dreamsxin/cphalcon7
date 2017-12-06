<?php

/*
	+------------------------------------------------------------------------+
	| Phalcon Framework                                                      |
	+------------------------------------------------------------------------+
	| Copyright (c) 2011-2012 Phalcon Team (http://www.phalconphp.com)       |
	+------------------------------------------------------------------------+
	| This source file is subject to the New BSD License that is bundled     |
	| with this package in the file docs/LICENSE.txt.                        |
	|                                                                        |
	| If you did not receive a copy of the license and are unable to         |
	| obtain it through the world-wide-web, please send an email             |
	| to license@phalconphp.com so we can send you a copy immediately.       |
	+------------------------------------------------------------------------+
	| Authors: Andres Gutierrez <andres@phalconphp.com>                      |
	|          Eduar Carvajal <eduar@phalconphp.com>                         |
	+------------------------------------------------------------------------+
*/

use Phalcon\Validation\Validator\PresenceOf,
	Phalcon\Validation\Validator\Identical,
	Phalcon\Validation\Validator\Confirmation,
	Phalcon\Validation\Validator\Regex,
	Phalcon\Validation\Validator\InclusionIn,
	Phalcon\Validation\Validator\ExclusionIn,
	Phalcon\Validation\Validator\StringLength,
	Phalcon\Validation\Validator\Email,
	Phalcon\Validation\Validator\Between,
	Phalcon\Validation\Validator\Url,
	Phalcon\Validation\Validator\File,
	Phalcon\Validation\Validator\Json,
	Phalcon\Validation\Validator\Uniqueness;

class ValidationTest extends PHPUnit\Framework\TestCase
{

	public function setUp()
	{
		date_default_timezone_set('UTC');
		spl_autoload_register(array($this, 'modelsAutoloader'));
	}

	public function tearDown()
	{
		spl_autoload_unregister(array($this, 'modelsAutoloader'));
	}

	public function modelsAutoloader($className)
	{
		if (file_exists('unit-tests/models/'.$className.'.php')) {
			require 'unit-tests/models/'.$className.'.php';
		}
		if (file_exists('unit-tests/validators/'.$className.'.php')) {
			require 'unit-tests/validators/'.$className.'.php';
		}
	}

	protected function _getDI(){

		Phalcon\Di::reset();

		$di = new Phalcon\Di();

		$di->set('modelsManager', function(){
			return new Phalcon\Mvc\Model\Manager();
		});

		$di->set('modelsMetadata', function(){
			return new Phalcon\Mvc\Model\Metadata\Memory();
		});

		$di->set('modelsQuery', 'Phalcon\Mvc\Model\Query');
		$di->set('modelsQueryBuilder', 'Phalcon\Mvc\Model\Query\Builder');
		$di->set('modelsCriteria', 'Phalcon\\Mvc\\Model\\Criteria');

		return $di;
	}

	public function testValidationAddValidation()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('name', 'PresenceOf');
		$validation->add('last_name', 'PresenceOf');

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'Field name is required',
					'_field' => 'name',
					'_code' => '0',
				)),
				1 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'Field last_name is required',
					'_field' => 'last_name',
					'_code' => '0',
				)),
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST['last_name'] = '';

		$validation = new Phalcon\Validation();
		$validation->add('last_name', ['PresenceOf', 'StringLength' => ['min' => 10]]);
	
		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'Field last_name is required',
					'_field' => 'last_name',
					'_code' => '0',
				)),
				1 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'TooShort',
					'_message' => 'Field last_name must be at least 10 characters long',
					'_field' => 'last_name',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);
	}

	public function testValidationGroup()
	{

		$message1 = new Phalcon\Validation\Message('This a message #1', 'field1', 'Type1');
		$message2 = new Phalcon\Validation\Message('This a message #2', 'field2', 'Type2');
		$message3 = new Phalcon\Validation\Message('This a message #3', 'field3', 'Type3');

		$messages = new Phalcon\Validation\Message\Group(array($message1, $message2));

		$this->assertEquals(count($messages), 2);
		$this->assertEquals($messages[0], $message1);
		$this->assertEquals($messages[1], $message2);

		$this->assertTrue(isset($messages[0]));
		$this->assertTrue(isset($messages[1]));

		$messages->appendMessage($message3);

		$this->assertEquals($messages[2], $message3);

		$number = 0;
		foreach ($messages as $position => $message) {
			$this->assertEquals($position, $number);
			$this->assertEquals($messages[$position]->getMessage(), $message->getMessage());
			$this->assertEquals($messages[$position]->getField(), $message->getField());
			$this->assertEquals($messages[$position]->getType(), $message->getType());
			$number++;
		}
		$this->assertEquals($number, 3);
	}

	public function testValidationPresenceOf()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('name', new PresenceOf());

		$validation->add('last_name', new PresenceOf());

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'Field name is required',
					'_field' => 'name',
					'_code' => '0',
				)),
				1 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'Field last_name is required',
					'_field' => 'last_name',
					'_code' => '0',
				)),
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST['last_name'] = 'Walter';

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'Field name is required',
					'_field' => 'name',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);
	}

	public function testValidationPresenceOfCustomMessage()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('name', new PresenceOf(array(
			'message' => 'The name is required'
		)));

		$validation->add('last_name', new PresenceOf(array(
			'message' => 'The last name is required'
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'The name is required',
					'_field' => 'name',
					'_code' => '0',
				)),
				1 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'The last name is required',
					'_field' => 'last_name',
					'_code' => '0',
				)),
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST['last_name'] = 'Walter';

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'The name is required',
					'_field' => 'name',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);
	}

	public function testValidationIdentical()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('name', new Identical(array(
			'value' => 'Peter'
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'Identical',
					'_message' => 'Field name does not have the expected value',
					'_field' => 'name',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('name' => 'Peter');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationIdenticalCustomMessage()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('name', new Identical(array(
			'value' => 'Peter',
			'message' => 'The name must be peter'
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'Identical',
					'_message' => 'The name must be peter',
					'_field' => 'name',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('name' => 'Peter');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationRegex()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('car_plate', new Regex(array(
			'pattern' => '/[A-Z]{3}\-[0-9]{3}/'
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'Regex',
					'_message' => 'Field car_plate does not match the required format',
					'_field' => 'car_plate',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('car_plate' => 'XYZ-123');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationRegexCustomMessage()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('car_plate', new Regex(array(
			'pattern' => '/[A-Z]{3}\-[0-9]{3}/',
			'message' => 'The car plate is not valid'
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'Regex',
					'_message' => 'The car plate is not valid',
					'_field' => 'car_plate',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('car_plate' => 'XYZ-123');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationEmail()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('email', new Email());

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'Email',
					'_message' => 'Field email must be an email address',
					'_field' => 'email',
					'_code' => 0,
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('email' => 'x=1');

		$messages = $validation->validate($_POST);

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('email' => 'x.x@hotmail.com');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationEmailCustomMessage()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('email', new Email(array(
			'message' => 'The email is not valid'
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'Email',
					'_message' => 'The email is not valid',
					'_field' => 'email',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('email' => 'x=1');

		$messages = $validation->validate($_POST);

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('email' => 'x.x@hotmail.com');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationInclusionIn()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('status', new InclusionIn(array(
			'domain' => array('A', 'I')
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'InclusionIn',
					'_message' => 'Field status must be a part of list: A, I',
					'_field' => 'status',
					'_code' => 0,
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('status' => 'X');

		$messages = $validation->validate($_POST);

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('status' => 'A');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationInclusionInCustomMessage()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('status', new InclusionIn(array(
			'message' => 'The status must be A=Active or I=Inactive',
			'domain' => array('A', 'I')
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'InclusionIn',
					'_message' => 'The status must be A=Active or I=Inactive',
					'_field' => 'status',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('status' => 'x=1');

		$messages = $validation->validate($_POST);

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('status' => 'A');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationExclusionIn()
	{
		$_POST = array('status' => 'A');

		$validation = new Phalcon\Validation();

		$validation->add('status', new ExclusionIn(array(
			'domain' => array('A', 'I')
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'ExclusionIn',
					'_message' => 'Field status must not be a part of list: A, I',
					'_field' => 'status',
					'_code' => 0,
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('status' => 'A');

		$messages = $validation->validate($_POST);

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('status' => 'X');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationExclusionInCustomMessage()
	{
		$_POST = array('status' => 'A');

		$validation = new Phalcon\Validation();

		$validation->add('status', new ExclusionIn(array(
			'message' => 'The status must not be A=Active or I=Inactive',
			'domain' => array('A', 'I')
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'ExclusionIn',
					'_message' => 'The status must not be A=Active or I=Inactive',
					'_field' => 'status',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('status' => 'A');

		$messages = $validation->validate($_POST);

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('status' => 'X');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationStringLengthMinimum()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('name', new StringLength(array(
			'min' => 3
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'TooShort',
					'_message' => 'Field name must be at least 3 characters long',
					'_field' => 'name',
					'_code' => 0,
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('name' => 'p');

		$messages = $validation->validate($_POST);

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('name' => 'peter');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationStringLengthMinimumCustomMessage()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('name', new StringLength(array(
			'min' => 3,
			'messageMinimum' => 'The message is too short'
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'TooShort',
					'_message' => 'The message is too short',
					'_field' => 'name',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('name' => 'p');

		$messages = $validation->validate($_POST);

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('name' => 'peter');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationStringLengthMaximum()
	{
		$_POST = array('name' => 'Johannes');

		$validation = new Phalcon\Validation();

		$validation->add('name', new StringLength(array(
			'max' => 4
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'TooLong',
					'_message' => 'Field name must not exceed 4 characters long',
					'_field' => 'name',
					'_code' => 0,
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('name' => 'a');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationStringLengthMaximumCustomMessage()
	{
		$_POST = array('name' => 'Johannes');

		$validation = new Phalcon\Validation();

		$validation->add('name', new StringLength(array(
			'max' => 4,
			'messageMaximum' => 'The message is too long'
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'TooLong',
					'_message' => 'The message is too long',
					'_field' => 'name',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('name' => 'pet');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationBetween()
	{
		$_POST = array('price' => 5);

		$validation = new Phalcon\Validation();

		$validation->add('price', new Between(array(
			'minimum' => 1,
			'maximum' => 3
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'Between',
					'_message' => 'Field price must be within the range of 1 to 3',
					'_field' => 'price',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array();

		$messages = $validation->validate($_POST);

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('price' => 2);

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationBetweenCustomMessage()
	{
		$_POST = array('price' => 5);

		$validation = new Phalcon\Validation();

		$validation->add('price', new Between(array(
			'minimum' => 1,
			'maximum' => 3,
			'message' => 'The price must be between 1 and 3'
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'Between',
					'_message' => 'The price must be between 1 and 3',
					'_field' => 'price',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array();

		$messages = $validation->validate($_POST);

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('price' => 2);

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationMixed()
	{

		$validation = new Phalcon\Validation();

		$validation
			->add('name', new PresenceOf(array(
				'message' => 'The name is required'
			)))
			->add('email', new PresenceOf(array(
				'message' => 'The email is required'
			)))
			->add('login', new PresenceOf(array(
				'message' => 'The login is required'
			)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 =>  Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'The name is required',
					'_field' => 'name',
					'_code' => '0',
				)),
				1 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'The email is required',
					'_field' => 'email',
					'_code' => '0',
				)),
				2 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'The login is required',
					'_field' => 'login',
					'_code' => '0',
				)),
			),
		));

		$this->assertEquals($messages, $expectedMessages);
	}

	public function testValidationCancelOnFail()
	{

		$validation = new Phalcon\Validation();

		$validation
			->add('name', new PresenceOf(array(
				'message' => 'The name is required'
			)))
			->add('email', new PresenceOf(array(
				'message' => 'The email is required',
				'cancelOnFail' => true
			)))
			->add('login', new PresenceOf(array(
				'message' => 'The login is required'
			)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 =>  Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'The name is required',
					'_field' => 'name',
					'_code' => '0',
				)),
				1 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'The email is required',
					'_field' => 'email',
					'_code' => '0',
				))
			),
		));

		$this->assertEquals($messages, $expectedMessages);
	}

	public function testValidationFiltering()
	{

		$validation = new Phalcon\Validation();

		$validation->setDI(new Phalcon\Di\FactoryDefault());

		$validation
			->add('name', new PresenceOf(array(
				'message' => 'The name is required'
			)))
			->add('email', new PresenceOf(array(
				'message' => 'The email is required'
			)));

		$validation->setFilters('name', 'trim');
		$validation->setFilters('email', 'trim');

		$_POST = array('name' => '  ', 'email' => '    ');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 2);

		$filtered = $messages->filter('email');

		$expectedMessages = array(
			0 => Phalcon\Validation\Message::__set_state(array(
				'_type' => 'PresenceOf',
				'_message' => 'The email is required',
				'_field' => 'email',
				'_code' => '0',
			))
		);

		$this->assertEquals($filtered, $expectedMessages);

		$_POST = array();
	}

	public function testValidationUrl()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('url', new Url());

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
						'_type' => 'Url',
						'_message' => 'Field url must be a url',
						'_field' => 'url',
						'_code' => 0,
					))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('url' => 'x=1');

		$messages = $validation->validate($_POST);

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('url' => 'http://phalconphp.com');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationUrlCustomMessage()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('url', new Url(array(
			'message' => 'The url is not valid'
		)));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
						'_type' => 'Url',
						'_message' => 'The url is not valid',
						'_field' => 'url',
						'_code' => '0',
					))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('url' => 'x=1');

		$messages = $validation->validate($_POST);

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('url' => 'http://phalconphp.com');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationFile()
	{
		$data = array('file' => 'unit-tests/assets/phalconphp.jpg');

		$validation = new Phalcon\Validation();

		$validation->add('file', new Phalcon\Validation\Validator\File(array(
			'mimes' => array('image/png', 'image/jpeg'),
			'minsize' => 10*1024,
			'maxsize' => 30*1024
		)));

		$messages = $validation->validate($data);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'TooLarge',
					'_message' => 'File file the size must not exceed 30720',
					'_field' => 'file',
					'_code' => 0,
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$data = array();

		$messages = $validation->validate($data);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'File',
					'_message' => 'Field file is not valid',
					'_field' => 'file',
					'_code' => 0,
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$data = array('file' => 'unit-tests/assets/logo.png');

		$messages = $validation->validate($data);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationSetLabels()
	{
		$_POST = array('email' => '', 'firstname' => '');

		$validation = new Phalcon\Validation();

		$validation->add('email', new PresenceOf(array(
                            'message' => 'The :field is required'
		)));
		$validation->add('email', new Email(array(
                            'message' => 'The :field must be email',
                            'label' => 'E-mail'
		)));
		$validation->add('firstname', new PresenceOf(array(
                            'message' => 'The :field is required'
		)));
		$validation->add('firstname', new StringLength(array(
                            'min' => 4,
                            'messageMinimum' => 'The :field is too short'
		)));

		$validation->setLabels(array('firstname' => 'First name'));

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'The email is required',
					'_field' => 'email',
					'_code' => '0',
				)),
				1 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'Email',
					'_message' => 'The E-mail must be email',
					'_field' => 'email',
					'_code' => '0',
				)),
				2 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'PresenceOf',
					'_message' => 'The First name is required',
					'_field' => 'firstname',
					'_code' => '0',
				)),
				3 => Phalcon\Validation\Message::__set_state(array(
					'_type' => 'TooShort',
					'_message' => 'The First name is too short',
					'_field' => 'firstname',
					'_code' => '0',
				))
			)
		));

		$this->assertEquals($expectedMessages, $messages);
	}

	public function testValidationJson()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('json', new Json());

		$messages = $validation->validate($_POST);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
						'_type' => 'Json',
						'_message' => 'Field json must be a json',
						'_field' => 'json',
						'_code' => 0,
					))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('json' => 'Phalcon');

		$messages = $validation->validate($_POST);

		$this->assertEquals($expectedMessages, $messages);

		$_POST = array('json' => '{"version":1.3}');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);
	}

	public function testValidationUniqueness()
	{
		require 'unit-tests/config.db.php';
		if (empty($configPostgresql)) {
			$this->markTestSkipped("Skipped");
			return;
		}

		$di = $this->_getDI();

		$di->set('db', function(){
			require 'unit-tests/config.db.php';
			return new Phalcon\Db\Adapter\Pdo\Postgresql($configPostgresql);
		}, true);

		$connection = $di->getShared('db');
		$success = $connection->delete("subscriptores");
		$this->assertTrue($success);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
						'_type' => 'Uniqueness',
						'_message' => 'email must be unique',
						'_field' => 'email',
						'_code' => 0,
					))
			)
		));

		$createdAt = new Phalcon\Db\RawValue('now()');

		$subscriptor = new Subscriptores();
		$subscriptor->email = 'dreamsxin@qq.com';
		$subscriptor->created_at = $createdAt;
		$subscriptor->status = 'P';
		$this->assertTrue($subscriptor->save());

		$validation = new Phalcon\Validation();
		$validation->add('email', new Uniqueness(array(
			'model' => $subscriptor,
			'message' => ':field must be unique'
		)));

		$messages = $validation->validate();

		$this->assertEquals($expectedMessages, $messages);

		$validation = new Phalcon\Validation();
		$validation->add('email', new Uniqueness(array(
			'model' => $subscriptor,
			'message' => ':field must be unique',
			'except' => 'id' // array('id', 'email')
		)));

		$messages = $validation->validate();
		$this->assertEquals(count($messages), 0);

		$subscriptor = new Subscriptores();
		$subscriptor->email = 'dreamsxin@qq.com';

		$validation = new Phalcon\Validation();
		$validation->add('email', new Uniqueness(array(
			'model' => $subscriptor,
			'message' => ':field must be unique'
		)));

		$messages = $validation->validate();

		$this->assertEquals($expectedMessages, $messages);

		$createdAt = date('Y-m-d H:i:s');

		$subscriptor->email = 'dreamsxin2@qq.com';
		$subscriptor->created_at = $createdAt;
		$subscriptor->status = 'P';
		$this->assertTrue($subscriptor->save());

		$subscriptor = new Subscriptores();
		$subscriptor->email = 'dreamsxin2@qq.com';
		$subscriptor->created_at = $createdAt;

		$validation = new Phalcon\Validation();
		$validation->add(array('email', 'created_at'), new Uniqueness(array(
			'model' => $subscriptor,
			'message' => ':field must be unique'
		)));

		$messages = $validation->validate();
		$this->assertEquals(count($messages), 1);

		$expectedMessages = Phalcon\Validation\Message\Group::__set_state(array(
			'_messages' => array(
				0 => Phalcon\Validation\Message::__set_state(array(
						'_type' => 'Uniqueness',
						'_message' => 'email, created_at must be unique',
						'_field' => array('email', 'created_at'),
						'_code' => 0,
					))
			)
		));

		$this->assertEquals($expectedMessages, $messages);

	}

	public function testValidationAlnum()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('username', new Phalcon\Validation\Validator\Alnum());

		$_POST = array('username' => 'AbCd1zyZ9');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);

		$_POST = array('username' => 'foo!#$bar');

		$messages = $validation->validate($_POST);

		$this->assertTrue(count($messages) > 0);
	}

	public function testValidationAlpha()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('username', new Phalcon\Validation\Validator\Alpha());

		$_POST = array('username' => 'Abcd');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);

		$_POST = array('username' => 'abcd123');

		$messages = $validation->validate($_POST);

		$this->assertTrue(count($messages) > 0);
	}

	public function testValidationDigit()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('username', new Phalcon\Validation\Validator\Digit());

		$_POST = array('username' => '12345');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);

		$_POST = array('username' => 'abcd123');

		$messages = $validation->validate($_POST);

		$this->assertTrue(count($messages) > 0);
	}

	public function testValidationDate()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();

		$validation->add('date', new Phalcon\Validation\Validator\Date());

		$_POST = array('date' => '2016-10-20');

		$messages = $validation->validate($_POST);

		$this->assertEquals(count($messages), 0);

		$_POST = array('username' => '20161020');

		$messages = $validation->validate($_POST);

		$this->assertTrue(count($messages) > 0);
	}

	public function testValidationAllowEmpty()
	{
		$_POST = array();

		$validation = new Phalcon\Validation();
		$validation->add('data', new Phalcon\Validation\Validator\Alnum(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\Alpha(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\Between(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\Confirmation(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\Date(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\Digit(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\Email(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\ExclusionIn(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\File(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\Identical(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\InclusionIn(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\Numericality(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\Regex(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\StringLength(array('allowEmpty' => TRUE)));
		$validation->add('data', new Phalcon\Validation\Validator\Url(array('allowEmpty' => TRUE)));

		$messages = $validation->validate($_POST);
		$this->assertEquals(count($messages), 0);

		$validation = new Phalcon\Validation(NULL, array('allowEmpty' => TRUE));
		$validation->add('data', new Phalcon\Validation\Validator\Alnum());
		$validation->add('data', new Phalcon\Validation\Validator\Alpha());
		$validation->add('data', new Phalcon\Validation\Validator\Between());
		$validation->add('data', new Phalcon\Validation\Validator\Confirmation());
		$validation->add('data', new Phalcon\Validation\Validator\Date());
		$validation->add('data', new Phalcon\Validation\Validator\Digit());
		$validation->add('data', new Phalcon\Validation\Validator\Email());
		$validation->add('data', new Phalcon\Validation\Validator\ExclusionIn());
		$validation->add('data', new Phalcon\Validation\Validator\File());
		$validation->add('data', new Phalcon\Validation\Validator\Identical());
		$validation->add('data', new Phalcon\Validation\Validator\InclusionIn());
		$validation->add('data', new Phalcon\Validation\Validator\Numericality());
		$validation->add('data', new Phalcon\Validation\Validator\Regex());
		$validation->add('data', new Phalcon\Validation\Validator\StringLength());
		$validation->add('data', new Phalcon\Validation\Validator\Url());

		$messages = $validation->validate($_POST);
		$this->assertEquals(count($messages), 0);
	}

	public function testValidationMessages()
	{
		Phalcon\Kernel::setBasePath("unit-tests/");
		Phalcon\Kernel::setMessagesDir("messages/");
		$this->assertEquals(Phalcon\Validation::getMessage('TooLarge'), "Field :field scale is out of range");
		$this->assertEquals(Phalcon\Validation::getMessage('Alnum'), "字段 :field 只能包含字母和数字");

		Phalcon\Validation::setMessageFilename('MyValidation');
		$this->assertEquals(Phalcon\Validation::getMessage('TooLarge'), "字段 :field 精度超出了范围");


		$_POST = array('username' => 'Phalcon7');

		$validation = new Phalcon\Validation(NULL, array('messageFilename' => 'MyValidation2'));
		$validation->add('username', new Phalcon\Validation\Validator\Digit());
		$messages = $validation->validate($_POST);
		$this->assertEquals($messages[0]->getMessage(), "用户名只能使用数字字符");
	}

	public function testCustomValidators()
	{
		Phalcon\Kernel::setBasePath("unit-tests/");
		Phalcon\Kernel::setMessagesDir("messages/");

		$_POST = array('idcard' => 'Phalcon7');

		$validation = new Phalcon\Validation();
		$validation->add('idcard', new IdCard(array('label' => '身份证', 'message' => ':field格式不正确')));
		$messages = $validation->validate($_POST);
		$this->assertEquals($messages[0]->getMessage(), "身份证格式不正确");

		Phalcon\Validation::setLabelDelimiter('、');
		$validation = new Phalcon\Validation();
		$validation->setLabels(array('id'=>'编号', 'name' => '姓名'));
		$validation->add(array('id', 'name', 'sex'), new MultiCheck(array('num' => 2)));
		$messages = $validation->validate($_POST);
		$this->assertEquals($messages[0]->getMessage(), "Field 编号、姓名、sex must set more than 2");
	}
}
