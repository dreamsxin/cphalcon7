<?php

class RandomTest extends PHPUnit\Framework\TestCase
{
	public function testRandom()
	{
		$random = new \Phalcon\Random();

		// Random alnum string
		$this->assertEquals(strlen($random->alnum()), 8);
		$this->assertEquals(strlen($random->alnum(1)), 1);

		// Random alpha string
		$this->assertEquals(strlen($random->alpha()), 8);
		$this->assertEquals(strlen($random->alpha(1)), 1);

		// Random hexdec string
		$this->assertTrue(!empty($random->hexdec()));

		// Random numeric string
		$this->assertTrue(!empty($random->numeric()));

		// Random nozero string
		$this->assertTrue(!empty($random->nozero()));

		// Random nozero string
		$this->assertTrue(!empty($random->color()));
		$this->assertTrue(!empty($random->color(Phalcon\Random::COLOR_RGBA)));
	}

	public function testSecurityRandom()
	{
		$random = new \Phalcon\Security\Random();

		// Random binary string
		$this->assertEquals(strlen($random->bytes()), 16);

		// Random hex string
		$this->assertEquals(strlen($random->hex(10)), 20);
		$this->assertEquals(strlen($random->hex(11)), 22);

		// Random base58 string
		$this->assertTrue(!empty($random->base58()));

		// Random base64 string
		$this->assertTrue(!empty($random->base64()));
		$this->assertTrue(!empty($random->base64(12)));
		$this->assertTrue(!empty($random->base64(16)));

		// Random URL-safe base64 string
		$this->assertTrue(!empty($random->base64Safe()));
		$this->assertTrue(!empty($random->base64Safe(8)));
		$this->assertTrue(!empty($random->base64Safe(null, true)));

		// Random UUID
		$this->assertTrue(!empty($random->uuid()));

		// Random number between 0 and $len
		$number = $random->number(256);
		$this->assertTrue(is_int($number) && $number >= 0 && $number <= 256);
	}
}
