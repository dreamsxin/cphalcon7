<?php

/**
 * curl http://localhost:8080/index/index
 * curl http://localhost:8080/index/view
 */

$worknum = 4;
for ($i=0; $i < $worknum; $i++) {
	$worker[$i] = Phalcon\Async\Process\ProcessBuilder::fork(__DIR__ . '/webserver/worker.php')->start();
}
$server = Phalcon\Async\Network\TcpServer::listen('localhost', 8080);

$num = 0;
try {
	var_dump($server->getAddress(), $server->getPort());

	while (true) {
		$socket = $server->accept();
		if ($socket === false) {
			continue;
		}
		$num++;
		$socket->export($worker[$num%$worknum]->getIpc());
		$socket->close();
	}
} finally {
	$server->close();
}
