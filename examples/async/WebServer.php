<?php

$loader = new \Phalcon\Loader();

$loader->registerDirs(
		array(
			__DIR__.'/mvc/controller',
		)
)->register();

$server = Phalcon\Async\Network\TcpServer::listen('localhost', 8080);

try {
	var_dump($server->getAddress(), $server->getPort());
	while (true) {
		$socket = $server->accept();
		if ($socket === false) {
			continue;
		}
		Phalcon\Async\Task::async(function () use ($socket) {
			try {
				$uri = $chunk = '';
				$ret = NULL;
				$parser = new \Phalcon\Http\Parser();
				while($buffer = $socket->read()) {
					$ret = $parser->execute($buffer);
					if (!$ret) {
						throw new \Exception('HTTP parse failed');
					}
					if ($parser->status() == \Phalcon\Http\Parser::STATUS_END) {
						$uri = \Phalcon\Arr::get($ret, 'QUERY_STRING');
						$body = \Phalcon\Arr::get($ret, 'BODY');
						break;
					}
				}

				if (empty($ret)) {
					$socket->close();
					return;
				}

				$di = new \Phalcon\DI\FactoryDefault;
				$di->set('view', function () {
					$view = new \Phalcon\Mvc\View();
					$view->setBasePath(__DIR__.DIRECTORY_SEPARATOR.'mvc/views');
					return $view;
				}, TRUE);

				$application = new \Phalcon\Mvc\Application;
				$application->useImplicitView(false);
				$sendchunk = $application->handle($uri)->getContent();
				var_dump($sendchunk);
				$sendchunk = \sprintf("HTTP/1.1 200 OK\r\nServer: webserver\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n%x\r\n%s\r\n0\r\n\r\n", \strlen($sendchunk), $sendchunk);
				$socket->write($sendchunk);
			} catch (\Throwable $e) {
				var_dump($e->getMessage());
			} finally {
				$socket->close();
				var_dump('CLIENT DISCONNECTED');
			}
		});
	}
} finally {
	$server->close();
}
