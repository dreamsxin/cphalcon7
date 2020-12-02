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
		$this->context = \Phalcon\Async\Context::background();
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
			HttpServer::info('Pool count '.$this->count);
			\Phalcon\Async\Task::asyncWithContext($this->context, static function (iterable $it) {
				try {
					foreach ($it as list ($defer, $context, $work, $socket, $args)) {
						try {
							$defer->resolve($context->run($work, $socket, ...$args));
						} catch (\Throwable $e) {
							HttpServer::err($e->getMessage());
							$defer->fail($e);
						} finally {
						}
					}
				} catch (\Throwable $e) {
					HttpServer::err($e->getMessage());
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

class HttpServer
{
	static public $debug = false;
	private $server;

	private $host;
	private $port;

	private $clients;

	public function __construct($host, $port, callable $callback = NULL, int $concurrency = 1, int $capacity = 0)
	{
		$this->host = $host;
		$this->port = $port;
		$this->callback = $callback;
		$this->pool = new Pool($concurrency, $capacity);
	}

	public function start()
	{
		$callback = $this->callback;
		$ws = $this;
		$worker = static function ($socket) use ($ws, $callback) {
			
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
						break;
					}
				}
			} catch (\Throwable $e) {
				self::err($e->getMessage());
			} finally {
				$socket->close();
			}
		};

		try {
			$pool = $this->pool;
			$this->server = \Phalcon\Async\Network\HttpServer::bind($this->host, $this->port);
			$this->server->on(\Phalcon\Async\Network\TcpServer::EVENT_ONCONNECT, function () use ($worker, $pool) {
				//echo 'connect'.PHP_EOL;
			});
			$this->server->on(\Phalcon\Async\Network\TcpServer::EVENT_ONREQUEST, function () use ($worker, $pool) {
				//echo 'request'.PHP_EOL;
				return 'hello world';
			});
			$this->server->start();
		} catch (\Throwable $e) {
			self::err($e->getMessage());
		} finally {
			if ($this->server) {
				$this->server->close();
			}
		}

	}

	static public function info($message)
	{
		if (self::$debug) {
			echo Phalcon\Cli\Color::info($message).PHP_EOL;
		}
	}

	static public function err($message)
	{
		echo Phalcon\Cli\Color::error($message).PHP_EOL;
	}
}


$opts = new \Phalcon\Cli\Options('Websocket CLI');
$opts->add([
    'type' => \Phalcon\Cli\Options::TYPE_STRING,
    'name' => 'server',
    'shortName' => 's',
    'required' => false, // 可选，需要用=号赋值
	'help' => "address"
]);
$opts->add([
    'type' => \Phalcon\Cli\Options::TYPE_INT,
    'name' => 'port',
    'shortName' => 'p',
    'required' => false,
	'help' => "port"
]);
$opts->add([
    'type' => \Phalcon\Cli\Options::TYPE_BOOLEAN,
    'name' => 'concurrency',
    'shortName' => 'c',
    'required' => false
]);
$opts->add([
    'type' => \Phalcon\Cli\Options::TYPE_BOOLEAN,
    'name' => 'capacity',
    'shortName' => 'C',
    'required' => false
]);
$opts->add([
    'type' => \Phalcon\Cli\Options::TYPE_BOOLEAN,
    'name' => 'debug',
    'shortName' => 'v',
    'required' => false,
	'help' => "enable debug"
]);
$vals = $opts->parse();
if ($vals === false ) {
	exit;
}
/**
 * 运行 php websocket-server.php
 */
if (isset($vals['debug'])) {
	Socks5Server::$debug = true;
	echo Phalcon\Cli\Color::info('Use debug mode').PHP_EOL;
}
$sserver = new HttpServer(\Phalcon\Arr::get($vals, 'server', '0.0.0.0'), \Phalcon\Arr::get($vals, 'port', 8080), function($socket, $status, $data) {
	// TODO
}, \Phalcon\Arr::get($vals, 'concurrency', 500), \Phalcon\Arr::get($vals, 'capacity', 1));
$sserver->start();
