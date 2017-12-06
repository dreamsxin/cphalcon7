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
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

class BinaryTest extends PHPUnit\Framework\TestCase
{

	public function testNormal()
	{
		$file = 'unit-tests/assets/data.bin';

		$fp = fopen($file, 'rb');
		$bin = new Phalcon\Binary\Reader($fp);
		$this->assertEquals($bin->getPosition(), 0);
		$this->assertEquals($bin->getEofPosition(), 26);
		$data1 = $bin->getContent();

		$this->assertEquals($bin->readUnsignedChar(), 160);
		$this->assertEquals($bin->readUnsignedChar(), 177);
		$this->assertEquals($bin->readUnsignedChar(), 194); 
		$this->assertEquals($bin->readUnsignedChar(), 211);
		$this->assertEquals($bin->readUnsignedInt16(), 240); 
		$this->assertEquals($bin->readUnsignedInt16(), 320);
		$this->assertEquals($bin->readUnsignedInt32(), 80001);
		$this->assertEquals($bin->getPosition(), 12);
		$this->assertEquals($bin->readString(), 'yi_tuo');
		$this->assertEquals($bin->getPosition(), 19);
		$this->assertEquals($bin->readString(), 'm999');
		$this->assertEquals($bin->getPosition(), 24);
		$this->assertEquals($bin->readString(), "\r\n");
		$this->assertEquals($bin->getEofPosition(), 26);

		$file = 'unit-tests/assets/test.bin';
		@unlink($file);
		$fp = fopen($file, 'wb+');
		$bin = new Phalcon\Binary\Writer($fp);
		$bin->writeUnsignedChar(160);
		$bin->writeUnsignedChar(177);
		$bin->writeUnsignedChar(194);
		$bin->writeUnsignedChar(211);
		$bin->writeUnsignedInt16(240);
		$bin->writeUnsignedInt16(320);
		$bin->writeUnsignedInt32(80001);
		$this->assertEquals($bin->getPosition(), 12);
		$bin->writeString('yi_tuo');
		$this->assertEquals($bin->getPosition(), 19);
		$bin->writeString("m999");
		$this->assertEquals($bin->getPosition(), 24);
		$bin->writeString("\r\n", 2, TRUE);
		$this->assertEquals($bin->getPosition(), 26);
		$data2 = $bin->getContent();
		$this->assertEquals($data1, $data2);

		$bin = new Phalcon\Binary\Reader($fp);
		$this->assertEquals($bin->getPosition(), 0);
		$this->assertEquals($bin->getEofPosition(), 26);

		$this->assertEquals($bin->readUnsignedChar(), 160);
		$this->assertEquals($bin->readUnsignedChar(), 177);
		$this->assertEquals($bin->readUnsignedChar(), 194); 
		$this->assertEquals($bin->readUnsignedChar(), 211);
		$this->assertEquals($bin->readUnsignedInt16(), 240); 
		$this->assertEquals($bin->readUnsignedInt16(), 320);
		$this->assertEquals($bin->readUnsignedInt32(), 80001);
		$this->assertEquals($bin->getPosition(), 12);
		$this->assertEquals($bin->readString(), 'yi_tuo');
		$this->assertEquals($bin->getPosition(), 19);
		$this->assertEquals($bin->readString(), 'm999');
		$this->assertEquals($bin->getPosition(), 24);
		$this->assertEquals($bin->getEofPosition(), 26);
	}

	public function testHexString()
	{
		$value = '136123456780';
		$bin = new Phalcon\Binary\Writer();
		$bin->writeHexString(str_pad($value, 28, 'f'), 28, TRUE);
		$this->assertEquals($bin->getPosition(), 14);
		$data = $bin->getContent();
		$this->assertEquals(bin2hex($data), '311632547608ffffffffffffffff');

		$bin = new Phalcon\Binary\Reader($data);
		$this->assertEquals($bin->getPosition(), 0);
		$this->assertEquals($bin->getEofPosition(), 14);

		$this->assertEquals($bin->readHexString(NULL, TRUE), str_pad($value, 28, 'f'));
		$this->assertEquals($bin->getPosition(), 14);
	}
}
