<?php

class HttpTest extends PHPUnit\Framework\TestCase
{
	public function testParser()
	{
		$body = <<<HTML
GET /http/test?arg=value#frag HTTP/1.1
Host: localhost
User-Agent: Phalcon Client
Accept-Language: en-us,en,zh;q=0.5
Accept-Encoding: gzip, deflate
Connection: keep-alive
Referer: http://localhost/
Cookie: name=phalcon
Cache-Control: max-age=0

HTML;
		$parser = new Phalcon\Http\Parser();
		$result = $parser->execute($body);
		$this->assertTrue(is_array($result));

		$this->assertTrue(isset($result['HEADERS']) && isset($result['HEADERS']['Cookie']));
		$this->assertTrue(isset($result['QUERY_STRING']));
		return;

		/**
		 * chunk=chunk-size[chunk-ext]CRLFchunk-dataCRLF
		 * 使用若干个Chunk组成，由一个标明长度为0的chunk结束，每个Chunk有两部分组成，第一部分是该Chunk的长度和长度单位（一般不 写），第二部分就是指定长度的内容，每个部分用CRLF隔开。
		 */

		$chunk =  "POST /chunked_w_trailing_headers HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n0\r\nVary: *\r\nContent-Type: text/plain\r\n\r\n";

		$parser = new Phalcon\Http\Parser();
		$result = $parser->execute($chunk);
		var_dump(($result));

		$chunk = "POST /two_chunks_mult_zero_end HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n000\r\n\r\n";

		$parser = new Phalcon\Http\Parser();
		$result = $parser->execute($chunk);
		var_dump(($result));

		$chunk =  "POST /chunked_w_trailing_headers HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n";

		$parser = new Phalcon\Http\Parser();
		$result = $parser->execute($chunk);
		var_dump(($result));

		$chunk = "6\r\n world\r\n0\r\nVary: *\r\nContent-Type: text/plain\r\n\r\n";
		$result = $parser->execute($chunk);
		var_dump(($result));
	}
}
