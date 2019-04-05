<?php

$server = Phalcon\Async\Network\TcpServer::listen('localhost', 8080);

try {
	var_dump($server->getAddress(), $server->getPort());

	while (true) {
		$socket = $server->accept();
		if ($socket === false) {
			continue;
		}
		Phalcon\Async\Task::async(function () use ($socket) {
			//var_dump('CLIENT CONNECTED');
			try {

				$chunk = $socket->read();

				if (empty($chunk)) {
					$socket->close();
					return;
				}

				var_dump($chunk);
				$sendchunk = 'Hello World';
				$sendchunk = \sprintf("HTTP/1.1 200 OK\r\nServer: webserver\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n%x\r\n%s\r\n0\r\n\r\n", \strlen($sendchunk), $sendchunk);
				$socket->write($sendchunk);

			} catch (\Throwable $e) {
				echo $e, "\n\n";
			} finally {
				$socket->close();
				//var_dump('CLIENT DISCONNECTED');
			}
		});
	}
} finally {
	$server->close();
}
