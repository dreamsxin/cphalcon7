<?php

/**
 * Usage:
 *
 * curl -v -d "SET key value" http://localhost:8888/
 * curl -v -d "GET key" http://localhost:8888/
 */
$lmdb = new Phalcon\Storage\Libmdbx('./testdb');

$server = Phalcon\Async\Network\TcpServer::listen('localhost', 8888);

function generateChunk($sendchunk) {
	return \sprintf("HTTP/1.1 200 OK\r\nServer: webserver\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n%x\r\n%s\r\n0\r\n\r\n", \strlen($sendchunk), $sendchunk);
}

try {
	while (true) {
		$socket = $server->accept();
		if ($socket === false) {
			continue;
		}
		Phalcon\Async\Task::async(function () use ($socket, $lmdb) {
			echo 'New client'.PHP_EOL;
			try {

				$buffer = '';
				$content_length = 0;
				$query = '';
				$content = '';
					
				$parser = new Phalcon\Http\Parser();
				while (!empty($chunk = $socket->read())) {
					$buffer .= $chunk;
					
					if (($result = $parser->execute($chunk))) {
						if (isset($result['QUERY_STRING'])) {
							$query = $result['QUERY_STRING'];
						}
						if (isset($result['BODY'])) {
							$content = $result['BODY'];
						}
						if (isset($result['HEADERS']) && isset($result['HEADERS']['Content-Length'])) {
							$content_length = $result['HEADERS']['Content-Length'];
							break;
						}
					}
					if (!$content_length) {
						if (substr_compare($chunk, "\r\n", -2, 2) === 0) {
							break;
						}
					} elseif (strlen($content) >= $content_length){
						break;
					}
				}
				
				echo 'Content '.$content_length.', '.$content.PHP_EOL;

				if ($content) {
					$parts = explode(" ", $content, 3);
				} else {
					$parts = explode("/", $content, 3);
				}
				if (count($parts) < 2) {
					$sendchunk = generateChunk('error');
					$socket->write($sendchunk);
					return;
				}
				$op = $parts[0];
				$key = $parts[1];
				if ($op == 'GET') {
					$lmdb->begin();
					$sendchunk = 'value '.$lmdb->get($key);
					$lmdb->commit();
				} elseif ($op == 'SET' || $op == 'PUT') {
					if (!isset($parts[2])) {
						$sendchunk = generateChunk('error');
						$socket->write($sendchunk);
						return;
					}
					$sendchunk = 'ok';
					$value = $parts[2];
					$lmdb->begin();
					$lmdb->put($key, $value);
					$lmdb->commit();
				} else {
					$sendchunk = 'error op is not valid';
				}

				$sendchunk = generateChunk($sendchunk);
				$socket->write($sendchunk);
			} catch (\Throwable $e) {
				echo $e, "\n\n";
			} finally {
				$socket->close();
			}
		});
	}
} finally {
	$server->close();
}
