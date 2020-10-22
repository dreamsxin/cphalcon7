<?php

$loader = new \Phalcon\Loader();

$loader->registerDirs(
		array(
			__DIR__.'/mvc/controller',
		)
)->register();

$logfile = __DIR__ . '/mvc/debug.log';
$logger = new \Phalcon\Logger\Adapter\File($logfile);

function debug(string $data, $lineno = 'NULL') {

	$message = ['data' => $data, 'line' => $lineno];
	print_r($data);
}

$server = Phalcon\Async\Network\TcpServer::listen('localhost', 8080);

try {
	var_dump($server->getAddress(), $server->getPort());

	$router = new \Phalcon\Mvc\Router();
	while (true) {
		$socket = $server->accept();
		if ($socket === false) {
			continue;
		}
		Phalcon\Async\Task::async(function () use ($socket, $router) {
			//\Phalcon\Debug::enable();
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

				$di = new \Phalcon\DI;
				$di->set('dispatcher', function () {
					$dispatcher = new \Phalcon\Mvc\Dispatcher();
					return $dispatcher;
				}, TRUE);
				$di->set('request', function () {
					$request = new \Phalcon\Http\Request();
					return $request;
				}, TRUE);
				$di->set('response', function () {
					$response = new \Phalcon\Http\Response();
					return $response;
				}, TRUE);
				$di->set('router', $router, TRUE);
				$di->set('view', function () {
					$view = new \Phalcon\Mvc\View();
					$view->setBasePath(__DIR__.DIRECTORY_SEPARATOR.'mvc/views');
					return $view;
				}, TRUE);

				$application = new \Phalcon\Mvc\Application;
				$application->useImplicitView(false);
				$sendchunk = $application->handle($uri)->getContent();

				debug($sendchunk, __LINE__);
				$sendchunk = \sprintf("HTTP/1.1 200 OK\r\nServer: webserver\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n%x\r\n%s\r\n0\r\n\r\n", \strlen($sendchunk), $sendchunk);
				$socket->write($sendchunk);
			} catch (\Throwable $e) {
				debug($e->getMessage(), __LINE__);
			} finally {
				//\Phalcon\Debug::disable();
				$socket->close();
				//debug('CLIENT DISCONNECTED', __LINE__);
			}
		});
	}
} finally {
	$server->close();
}
