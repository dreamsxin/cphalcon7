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
	}
}
