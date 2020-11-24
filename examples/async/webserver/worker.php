<?php

var_dump(Phalcon\Async\Process\Process::isWorker());
var_dump($ipc = Phalcon\Async\Process\Process::connect());

$loader = new \Phalcon\Loader();

$loader->registerDirs(
		array(
			__DIR__.'/mvc/controller',
		)
)->register();

function debug(string $data, $lineno = 'NULL') {
	$message = ['data' => $data, 'line' => $lineno];
	print_r($data);
}

while ($socket = Phalcon\Async\Network\TcpSocket::import($ipc)) {

	Phalcon\Async\Task::async(function () use ($socket) {
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

			/*
			$sendchunk = 'hello world';
			$sendchunk = \sprintf("HTTP/1.1 200 OK\r\nServer: webserver\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n%x\r\n%s\r\n0\r\n\r\n", \strlen($sendchunk), $sendchunk);
			$socket->write($sendchunk);
			return;
			*/

			$app = new \Phalcon\Mvc\Micro();

			// 超始路由
			$app->get('/', function () use ($socket) {
				
				$sendchunk = "<h1>Welcome!</h1>";

				//debug($sendchunk, __LINE__);
				$sendchunk = \sprintf("HTTP/1.1 200 OK\r\nServer: webserver\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n%x\r\n%s\r\n0\r\n\r\n", \strlen($sendchunk), $sendchunk);
				$socket->write($sendchunk);
			});

			$app->handle($uri);
			return;

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
			$di->set('router', function () {
				$router = new \Phalcon\Mvc\Router();
				return $router;
			}, TRUE);
			$di->set('view', function () {
				$view = new \Phalcon\Mvc\View();
				$view->setBasePath(__DIR__.DIRECTORY_SEPARATOR.'mvc/views');
				return $view;
			}, TRUE);

			$application = new \Phalcon\Mvc\Application;
			$application->useImplicitView(false);
			$sendchunk = $application->handle($uri)->getContent();

			//debug($sendchunk, __LINE__);
			$sendchunk = \sprintf("HTTP/1.1 200 OK\r\nServer: webserver\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n%x\r\n%s\r\n0\r\n\r\n", \strlen($sendchunk), $sendchunk);
			$socket->write($sendchunk);
		} catch (\Throwable $e) {
			$sendchunk = "<h1>Not found!</h1>";
			$sendchunk = \sprintf("HTTP/1.1 404 NOT FOUND\r\nServer: webserver\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n%x\r\n%s\r\n0\r\n\r\n", \strlen($sendchunk), $sendchunk);
			$socket->write($sendchunk);
			debug($e->getMessage(), __LINE__);
		} finally {
			//\Phalcon\Debug::disable();
			$socket->close();
			//debug('CLIENT DISCONNECTED', __LINE__);
		}
	});
}
