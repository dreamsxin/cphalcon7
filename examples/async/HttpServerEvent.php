<?php

class HttpServer
{
	static public $debug = false;
	private $server;

	private $host;
	private $port;

	private $clients;

	public function __construct($host, $port, callable $callback = NULL)
	{
		$this->host = $host;
		$this->port = $port;
		$this->callback = $callback;
	}

	public function start()
	{
		$callback = $this->callback;
		$ws = $this;
		try {
			$pool = $this->pool;
			$this->server = \Phalcon\Async\Network\HttpServer::bind($this->host, $this->port);
			echo Phalcon\Cli\Color::info('start server listen:'.$this->host.':'.$this->port).PHP_EOL;
			$this->server->on(\Phalcon\Async\Network\TcpServer::EVENT_ONCONNECT, function () use ($ws, $callback) {
				//echo 'connect'.PHP_EOL;
			});
			$this->server->on(\Phalcon\Async\Network\TcpServer::EVENT_ONREQUEST, function () use ($ws, $callback) {
				return 'hello word';
			});
			$this->server->start();
		} catch (\Throwable $e) {
		} finally {
			if ($this->server) {
				$this->server->close();
			}
		}

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
});
$sserver->start();
