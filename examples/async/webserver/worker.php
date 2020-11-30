<?php

class Pool
{
	private $channel;
	private $concurrency;
	private $count = 0;
	private $context;

	public function __construct(int $concurrency = 1, int $capacity = 0)
	{
		$this->concurrency = max(1, $concurrency);
		$this->channel = new \Phalcon\Async\Channel($capacity);
		$this->context = \Phalcon\Async\Context::current();
	}

	public function close(?\Throwable $e = null): void
	{
		$this->count = \PHP_INT_MAX;
		$this->channel->close($e);
	}

	public function submit(callable $work, $socket, ...$args): \Phalcon\Async\Awaitable
	{
		if ($this->count < $this->concurrency) {
			$this->count++;
			\Phalcon\Async\Task::asyncWithContext($this->context, static function (iterable $it) {
				try {
					foreach ($it as list ($defer, $context, $work, $socket, $args)) {
						try {
							$defer->resolve($context->run($work, $socket, ...$args));
						} catch (\Throwable $e) {
							$defer->fail($e);
						} finally {
						}
					}
				} catch (\Throwable $e) {
				} finally {
					--$this->count;
				}
			}, $this->channel->getIterator());
		}

		$this->channel->send([
			$defer = new \Phalcon\Async\Deferred(),
			\Phalcon\Async\Context::current(),
			$work,
			$socket,
			$args
		]);

		return $defer->awaitable();
	}
}

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

$worker = static function ($socket) {
	
	$parser = new \Phalcon\Http\Parser();

	try {
		$buffer = '';
		while (null !== ($chunk = $socket->read())) {

			$ret = $parser->execute($chunk);
			if (!$ret) {
				throw new \Exception('HTTP parse failed');
			}
			if ($parser->status() == \Phalcon\Http\Parser::STATUS_END) {
				$uri = \Phalcon\Arr::get($ret, 'QUERY_STRING');
				$body = \Phalcon\Arr::get($ret, 'BODY');
				
				$sendchunk = 'hello world';
				$sendchunk = \sprintf("HTTP/1.1 200 OK\r\nServer: webserver\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n%x\r\n%s\r\n0\r\n\r\n", \strlen($sendchunk), $sendchunk);
				$socket->write($sendchunk);
				/*
				$app = new \Phalcon\Mvc\Micro();

				$app->get('/', function () use ($socket) {
					
					$sendchunk = "<h1>Welcome!</h1>";
					$sendchunk = \sprintf("HTTP/1.1 200 OK\r\nServer: webserver\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n%x\r\n%s\r\n0\r\n\r\n", \strlen($sendchunk), $sendchunk);
					$socket->write($sendchunk);
				});

				$app->handle($uri);
				*/
				break;
			}
		}
	} catch (\Throwable $e) {
		self::err($e->getMessage());
	} finally {
		$socket->close();
	}
};

$pool = new Pool(500, 1);
while ($socket = Phalcon\Async\Network\TcpSocket::import($ipc)) {

	$pool->submit($worker, $socket);
}
