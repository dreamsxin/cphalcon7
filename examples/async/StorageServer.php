<?php

/**
 * Usage:
 *
 * curl -v -d "SET key value" http://localhost:8888/
 * curl -v -d "GET key" http://localhost:8888/
 * ab -n 1000 -c 10 -p task.log 'http://localhost:8888/set/key'
 */
$lmdb = new Phalcon\Storage\Libmdbx('./testdb');

$server = Phalcon\Async\Network\TcpServer::listen('localhost', 8888);

function generateChunk($sendchunk) {
	return \sprintf("HTTP/1.1 200 OK\r\nServer: webserver\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n%x\r\n%s\r\n0\r\n\r\n", \strlen($sendchunk), $sendchunk);
}

$lmdb->begin();
try {
	while (true) {
		$socket = $server->accept();
		if ($socket === false) {
			continue;
		}
		Phalcon\Async\Task::async(function () use ($socket, $lmdb) {
			//echo 'New client'.PHP_EOL;
			try {
				$content_length = 0;
				$query = '';
				$content = '';
					
				$parser = new Phalcon\Http\Parser();
				while (!empty($chunk = $socket->read())) {
					
					if (($result = $parser->execute($chunk, true))) {
						if (isset($result['QUERY_STRING'])) {
							$query = $result['QUERY_STRING'];
						}
						if (isset($result['BODY'])) {
							$content = $result['BODY'];
						}
						if (isset($result['HEADERS'])) {
							if (isset($result['HEADERS']['Content-Length'])) {
								$content_length = (int)$result['HEADERS']['Content-Length'];
							} elseif (isset($result['HEADERS']['Content-length'])) {
								$content_length = (int)$result['HEADERS']['Content-length'];
							}
						}
					}
					//var_dump($result);
					if (!$content_length) {
						if (substr_compare($chunk, "\r\n\r\n", -4, 4) === 0) {
							//echo "end".PHP_EOL;
							break;
						}
					} elseif (strlen($content) >= $content_length){
						//echo "end2".PHP_EOL;
						break;
					}
				}

				//echo 'Query '.$query.PHP_EOL;
				//echo 'Content '.$content_length.', '.$content.PHP_EOL;

				if (strlen($query) > 2) {
					$parts = explode("/", $query, 3);
					if (count($parts) < 3) {
						$sendchunk = generateChunk('error');
						$socket->write($sendchunk);
						return;
					}
					$op = $parts[1];
					$key = $parts[2];
					$value = $content;
				} else if ($content) {
					$parts = explode(" ", $content, 3);
					if (count($parts) < 2) {
						$sendchunk = generateChunk('error');
						$socket->write($sendchunk);
						return;
					}
					$op = $parts[0];
					$key = $parts[1];
					$value = isset($parts[2]) ? $parts[2] : '';
				}
				if (!isset($op)) {
					return;
				}
				$op = strtolower($op);
				//echo 'op '.$op.', key '.$key.PHP_EOL;
				if ($op == 'get') {
					$sendchunk = 'value '.$lmdb->get($key);
				} elseif ($op == 'set' || $op == 'put') {
					if (!isset($parts[2])) {
						$sendchunk = generateChunk('error');
						$socket->write($sendchunk);
						return;
					}
					$sendchunk = 'ok';
					$lmdb->put($key, $value);
				} elseif ($op == 'delete' || $op == 'del' || $op == 'rm') {
					$sendchunk = 'ok';
					$lmdb->del($key);
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
$lmdb->commit();
