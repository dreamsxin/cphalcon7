<?php

class Socks5
{
	const  STATUS_INIT = 0;
	const  STATUS_AUTH = 1;
	const  STATUS_ADDR = 2;
	const  STATUS_UDP_ASSOC = 3; // CMD_UDP_ASSOCIATE
	const  STATUS_DNS = 4;
	const  STATUS_CONNECTING = 5;
	const  STATUS_STREAM = 6;

	const  CMD_CONNECT = 1;
	const  CMD_BIND = 2;
	const  CMD_UDP_ASSOCIATE = 3;

	const  TYPE_IPV4= 1;
	const  TYPE_HOST= 3;
	const  TYPE_IPV6= 4;

	const  METHOD_NO_AUTH= 0;
	const  METHOD_GSSAPI= 1;
	const  METHOD_USER_PASS= 2;

	const  REPLY_ADDR	= "\x05\x00\x00\x01\x00\x00\x00\x00\x10\x10";

	public static function parseAuth($buffer)
	{
		
		Socks5Server::info('Socks5::parseAuth');
		if(strlen($buffer) < 3) {
			return true;
		}
		$version = ord($buffer[0]);
		$num = ord($buffer[1]);
		if(strlen($buffer) < (2 + $num)) {
			return true;
		}
		$types = [];
		for ($i=0; $i < $num; $i++) {
			$types[] = ord($buffer[2 + $i]);
		}
		return $types;
	}

	public static function parseAddr($buffer)
	{
		Socks5Server::info('Socks5::parseAddr');
		if(strlen($buffer) < 4) {
			return true;
		}
		$addr_type = ord($buffer[3]);
		switch($addr_type)
		{
			case self::TYPE_IPV4:
				if(strlen($buffer) < 10) {
					return true;
				}
				$dest_addr = ord($buffer[4]).'.'.ord($buffer[5]).'.'.ord($buffer[6]).'.'.ord($buffer[7]);
				$port_data = unpack('n', substr($buffer, -2));
				$dest_port = $port_data[1];
				$header_length = 10;
				break;
			case self::TYPE_IPV6:
				if(strlen($buffer) < 22) {
					return true;
				}
				$ip = substr($buffer, 4, 16);
				if (substr($ip, 0, 12) == "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff") {
					$dest_addr = "::ffff:" . ord($ip[12]) . '.' . ord($ip[13]) . '.' . ord($ip[14]) . '.' . ord($ip[15]);
				} else {
					$hex = bin2hex($ip);
					$groups = str_split($hex, 4);
					$in_collapse = false;
					$done_collapse = false;
					foreach ($groups as $index => $group) {
						if ($group == '0000' && !$done_collapse) {
							if ($in_collapse) {
								$groups[$index] = '';
								continue;
							}
							$groups[$index] = ':';
							$in_collapse = true;
							continue;
						}
						if ($in_collapse) {
							$done_collapse = true;
						}
						$groups[$index] = ltrim($groups[$index], '0');
						if (strlen($groups[$index]) === 0) {
							$groups[$index] = '0';
						}
					}
					$ip = join(':', array_filter($groups, 'strlen'));
					$ip = str_replace(':::', '::', $ip);
					$dest_addr = ($ip == ':') ? '::' : $ip;
				}
				$port_data = unpack('n', substr($buffer, -2));
				$dest_port = $port_data[1];
				$header_length = 22;
				break;
			case self::TYPE_HOST:
				if(strlen($buffer) < 10) {
					return true;
				}
				$addrlen = ord($buffer[4]);
				if(strlen($buffer) < $addrlen + 5) {
					return true;
				}
				$dest_addr = substr($buffer, 5, $addrlen);
				$port_data = unpack('n', substr($buffer, -2));
				$dest_port = $port_data[1];
				$header_length = $addrlen + 7;
				break;
			default:
				return false;
		}
		return array($addr_type, $dest_addr, $dest_port, $header_length);
	}
}

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
			Socks5Server::info('Pool count '.$this->count);
			\Phalcon\Async\Task::asyncWithContext($this->context, static function (iterable $it) {
				try {
					foreach ($it as list ($defer, $context, $work, $socket, $args)) {
						try {
							$defer->resolve($context->run($work, $socket, ...$args));
						} catch (\Throwable $e) {
							Socks5Server::err($e->getMessage());
							$defer->fail($e);
						} finally {
						}
					}
				} catch (\Throwable $e) {
					Socks5Server::err($e->getMessage());
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

class Socks5Server
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
			$socket->status = Socks5::STATUS_INIT;
			$socket->is_closing = false;
			try {
				$buffer = '';
				while (!$socket->is_closing && null !== ($chunk = $socket->read())) {

					$buffer .= $chunk;
					switch ($socket->status) {
						case Socks5::STATUS_INIT:
							/**
							 * 协商版本以及验证方式
							 * +---------+-----------------+------------------+
							 * |协议版本 |支持的验证式数量 |验证方式          |
							 * +---------+-----------------+------------------+
							 * |1个字节  |1个字节          |1种式占一个字节   |
							 * +---------+-----------------+------------------+
							 * |0x05     |0x02             |0x00,0x02         |
							 * +---------+-----------------+------------------+
							 */
							/**
							 * 0x00 无验证需求
							 * 0x01 通用安全服务应用程序接口(GSSAPI)
							 * 0x02 用户名/密码(USERNAME/PASSWORD)
							 * 0x03 至 X’7F’ IANA 分配(IANA ASSIGNED)
							 * 0x80 至 X’FE’ 私人方法保留(RESERVED FOR PRIVATE METHODS)
							 * 0xFF 无可接受方法(NO ACCEPTABLE METHODS)
							 */
							$authtypes = Socks5::parseAuth($buffer);
							if ($authtypes === false) {
								$socket->is_closing = true;
								break;
							}
							if ($authtypes === true)  { // continue
								break;
							}
							$socket->status = Socks5::STATUS_ADDR;
							$socket->write("\x05\x00"); // TODO: 暂时
							$buffer = '';
							break;
						case Socks5::STATUS_AUTH:
							/**
							 * 验证账号密码
							 * +--------+-----------+-------------------------+---------+-------------------------------+
							 * |协议版本|用户名长度 |用户名                   |密码长度 |密码                           |
							 * +--------+-----------+-------------------------+---------+-------------------------------+
							 * |1个字节 |1个字节    |用户名字节数据           |1个字节  |密码字节数据                   |
							 * +--------+-----------+-------------------------+---------+-------------------------------+
							 * |0x01    |0x05       |0x61,0x62,0x63,0x64,0x65 |0x06     |0x31,0x32,0x33,0x34,0x35,0x36  |
							 * +--------+-----------+-------------------------+---------+-------------------------------+
							 */
							if ($callback && \is_callable($callback)) {
								// TODO
							}
							break;
						case Socks5::STATUS_ADDR:
							self::info('Socks5::STATUS_ADDR');
							/**
							 * 建立代理连接
							 * +----------+------------+---------+-----------+-----------------------+------------+
							 * |协议版本  |请求的类型  |保留字段 |地址类型   |地址数据               |地址端口    |
							 * +----------+------------+---------+-----------+-----------------------+------------+
							 * |1个字节   |1个字节     |1个字节  |1个字节    |变长                   |2个字节     |
							 * +----------+------------+---------+-----------+-----------------------+------------+
							 * |0x05      |0x01        |0x00     |0x01       |0x0a,0x00,0x01,0x0a    |0x00,0x50   |
							 * +----------+------------+---------+-----------+-----------------------+------------+
							 */
							/**
							 * 请求类型
							 * CONNECT : 0x01, 建立代理连接
							 * BIND : 0x02,告诉代理服务器监听目标机器的连接,也就是让代理服务器创建socket监听来自目标机器的连接。FTP这类需要服务端主动联接客户端的的应用场景。
							 *     1. 只有在完成了connnect操作之后才能进行bind操作
							 *     2. bind操作之后，代理服务器会有两次响应, 第一次响应是在创建socket监听完成之后，第二次是在目标机器连接到代理服务器上之后。
							 * UDP ASSOCIATE : 0x03, udp 协议请求代理。
							 */
							if (strlen($buffer) < 2) {
								break;
							}
							$cmd = ord($buffer[1]);
							if ($cmd != Socks5::CMD_CONNECT) {
								self::err("Bad command ".$cmd);
								$socket->is_closing = true;
								break;
							}
							$headers = Socks5::parseAddr($buffer);
							if (!$headers)  {
								self::err('Error header');
								$socket->is_closing = true;
								break;
							}
							if ($headers === true)  { // continue
								break;
							}
							/**
							 * 数据包转发
							 * +----+------+------+----------+----------+----------+
							 * |RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
							 * +----+------+------+----------+----------+----------+
							 * | 2  |  1   |  1   | Variable |    2     | Variable |
							 * +----+------+------+----------+----------+----------+
							 */
							$buffer = substr($buffer, $headers[3]);
							$socket->status  = Socks5::STATUS_CONNECTING;
							if (!in_array($headers[0], [Socks5::TYPE_IPV4, Socks5::TYPE_HOST, Socks5::TYPE_IPV6])) {
								self::err('Addr type error');
								$socket->is_closing = true;
								break;
							}
							$tls = NULL;
							if ($headers[2] == 443) {
								$tls = new \Phalcon\Async\Network\TlsClientEncryption();
								$tls = $tls->withAllowSelfSigned(true);
							}
							$socket->client = \Phalcon\Async\Network\TcpSocket::connect($headers[1], $headers[2], $tls);
							$socket->write(Socks5::REPLY_ADDR);
							$socket->status  = Socks5::STATUS_STREAM;
	
							\Phalcon\Async\Task::async(function ($client) use ($socket) {
								while (!$socket->is_closing && null !== ($chunk = $client->read())) {
									$socket->write($chunk);
								}
							}, $socket->client);
							break;
						case Socks5::STATUS_STREAM:
							self::info('Socks5::STATUS_STREAM');
							$socket->client->write($buffer);
							$buffer = '';
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
			$this->server = \Phalcon\Async\Network\TcpServer::listen($this->host, $this->port);
			echo Phalcon\Cli\Color::info('start server listen:'.$this->host.':'.$this->port).PHP_EOL;
			while (true) {
				$socket = $this->server->accept();
				if ($socket === false) {
					continue;
				}
				$this->pool->submit($worker, $socket);
			}
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
$sserver = new Socks5Server(\Phalcon\Arr::get($vals, 'server', '0.0.0.0'), \Phalcon\Arr::get($vals, 'port', 10002), function($socket, $status, $data) {
	// TODO
}, \Phalcon\Arr::get($vals, 'concurrency', 500), \Phalcon\Arr::get($vals, 'capacity', 1));
$sserver->start();
