<?php
//Phalcon\Debug::enable();
$loader = new Phalcon\Loader();

$loader->registerDirs(array(
	__DIR__.DIRECTORY_SEPARATOR."websocket-plugins".DIRECTORY_SEPARATOR,
));

$loader->register();

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
			Websocket::info('Pool count '.$this->count);
			\Phalcon\Async\Task::asyncWithContext($this->context, static function (iterable $it) {
				try {
					foreach ($it as list ($defer, $context, $work, $socket, $args)) {
						try {
							$defer->resolve($context->run($work, $socket, ...$args));
						} catch (\Throwable $e) {
							Websocket::err($e->getMessage());
							$defer->fail($e);
						} finally {
						}
					}
				} catch (\Throwable $e) {
					Websocket::err($e->getMessage());
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

class Websocket
{
	static public $debug = false;
	protected $port = null;
	protected $host = null;
	protected $server = null;
	protected $callback = null;

	protected static $opcodes = array(
		'continuation' => 0,
		'text'		 => 1,
		'binary'	   => 2,
		'close'		=> 8,
		'ping'		 => 9,
		'pong'		 => 10,
	);

	public function __construct($host, int $port, callable $callback = NULL, int $concurrency = 1, int $capacity = 0) {
		$this->port = $port;
		$this->host = $host;
		$this->callback = $callback;
		$this->pool = new Pool($concurrency, $capacity);
	}

	public function start()
	{
		$callback = $this->callback;
		$ws = $this;
		$worker = static function ($socket) use ($ws, $callback) {
			// echo ('memory'.memory_get_usage().PHP_EOL);
			//$socket->setOption(TcpSocket::NODELAY, false);
			$socket->isHttp = false;
			$socket->parser = new \Phalcon\Http\Parser();
			$socket->isHandshake = false;
			$socket->is_closing = false;
			$socket->fragment_status = 0;
			$socket->fragment_length = 0;
			$socket->fragment_size = 4096;
			$socket->read_length = 0;
			$socket->huge_payload = '';
			$socket->payload = '';
			$socket->headers = NULL;
			$socket->request_path = NULL;
			try {
				$buffer = '';
				while (!$socket->is_closing && null !== ($chunk = $socket->read())) {

					if ($socket->isHandshake === false) {
						$buffer .= $chunk;
						$pos = strpos($buffer, "\r\n\r\n");
						if ($pos) {
							$header = substr($buffer, 0, $pos+4);
							$buffer = substr($buffer, $pos+4);
							if ($ws->handShake($socket, $header)) {
							}
						}
					} elseif ($socket->isHttp) {
						$buffer = $chunk;
					} else {
						$buffer .= $chunk;
					}
					if ($socket->isHttp) {
						$ret = $socket->parser->execute($buffer);
						if (!$ret) {
							throw new \Exception('HTTP parse failed');
						}
						if ($socket->parser->status() == \Phalcon\Http\Parser::STATUS_END) {
							$body = \Phalcon\Arr::get($ret, 'BODY');
							if ($callback && \is_callable($callback)) {
								$callback($socket, $socket->headers, $socket->request_path, $body);
							}
							$socket->is_closing = true;
							break;
						}
					} else if ($ws->process($socket, $buffer)) {
						$buffer = substr($buffer, $socket->read_length);
						if ($callback && \is_callable($callback)) {
							$callback($socket, $socket->headers, $socket->request_path, $socket->payload);
						}
						$socket->fragment_status = 0;
						$socket->fragment_length = 0;
						$socket->read_length = 0;
						$socket->huge_payload = '';
						$socket->payload = '';
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
				// \Phalcon\Async\Task::async($worker, $socket);
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

	/**
	 * 请求握手
	 * @return boolean
	 */
	static public function handShake($socket, $header)
	{
		self::info('recv:'.$header);
		$request = $socket->parser->execute($header, true);
		if (!$request || !isset($request['HEADERS'])) {
			throw new \Exception('Handshake failed, HEAD error');
		}
		$headers = $request['HEADERS'];
		$socket->headers = $headers;
		$socket->request_path = $request['QUERY_STRING'];

		if (!isset($headers['Sec-WebSocket-Key'])) {
			$socket->isHttp = true;
			return true;
		} else if ($request['REQUEST_METHOD'] != 'GET') {
			throw new \Exception('Handshake failed, No GET in HEAD');
		}
		$socket->isHandshake = true;

		$wsKey = trim($headers['Sec-WebSocket-Key']);

		// 根据客户端传递过来的 key 生成 accept key
		$acceptKey = base64_encode(sha1($wsKey . "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", true));

		// 拼接回复字符串
		$msg = "HTTP/1.1 101 Switching Protocols\r\n";
		$msg .= "Upgrade: websocket\r\n";
		$msg .= "Sec-WebSocket-Version: 13\r\n";
		$msg .= "Connection: Upgrade\r\n";
		$msg .= "Sec-WebSocket-Accept: " . $acceptKey . "\r\n\r\n";
		$socket->write($msg);
		return true;
	}

	static public function readFragment0($socket, $buffer) {
		if (strlen($buffer) < 2) {
			return false;
		}
		$data = substr($buffer, 0, 2);
		$socket->read_length = 2;
		// Is this the final fragment?	// Bit 0 in byte 0
		/// @todo Handle huge payloads with multiple fragments.
		$socket->final = (boolean) (ord($data[0]) & 1 << 7);

		// Should be unused, and must be false…	// Bits 1, 2, & 3
		$rsv1	= (boolean) (ord($data[0]) & 1 << 6);
		$rsv2	= (boolean) (ord($data[0]) & 1 << 5);
		$rsv3	= (boolean) (ord($data[0]) & 1 << 4);

		// Parse opcode
		$opcode_int = ord($data[0]) & 31; // Bits 4-7
		$opcode_ints = array_flip(self::$opcodes);
		if (!array_key_exists($opcode_int, $opcode_ints)) {
			throw new \Exception('Bad opcode in websocket frame: '.$opcode_int);
		}

		$opcode = $opcode_ints[$opcode_int];
		self::info('opcode '.$opcode);

		// record the opcode if we are not receiving a continutation fragment
		if ($opcode !== 'continuation') {
			$socket->last_opcode = $opcode;
		}

		// Masking?
		$socket->mask = (boolean) (ord($data[1]) >> 7);	// Bit 0 in byte 1

		$socket->payload = '';

		// Payload length
		$socket->payload_length = (integer) ord($data[1]) & 127; // Bits 1-7 in byte 1
		if ($socket->payload_length > 125) {
			$socket->fragment_status = 1;
		} else {
			$socket->fragment_status = 2;
		}
		return true;
	}

	static public function readFragment1($socket, $buffer) {
		if ($socket->payload_length === 126) {
			if ($socket->fragment_length - $socket->read_length < 2) {
				return false;
			}
			$data = substr($buffer, $socket->read_length, 2); // 126: Payload is a 16-bit unsigned int
			$socket->read_length += 2;
		} else {
			if ($socket->fragment_length - $socket->read_length < 8) {
				return false;
			}
			$data = substr($buffer, $socket->read_length, 8); // 127: Payload is a 64-bit unsigned int
			$socket->read_length += 8;
		}
		$socket->payload_length = bindec(self::sprintB($data));
		$socket->fragment_status = 2;
		return true;
	}

	static public function readFragment2($socket, $buffer) {

		// Get masking key.
		if ($socket->mask) {
			if ($socket->fragment_length - $socket->read_length < (4 + $socket->payload_length)) {
				return false;
			}
			$masking_key = substr($buffer, $socket->read_length, 4);
			$socket->read_length += 4;
		} elseif ($socket->fragment_length - $socket->read_length < $socket->payload_length) {
			return false;
		}

		// Get the actual payload, if any (might not be for e.g. close frames.
		if ($socket->payload_length > 0) {
			$data = substr($buffer, $socket->read_length, $socket->payload_length);
			$socket->read_length += $socket->payload_length;

			if ($socket->mask) {
				// Unmask payload.
				for ($i = 0; $i < $socket->payload_length; $i++) {
					$socket->payload .= ($data[$i] ^ $masking_key[$i % 4]);
				}
			} else {
				$socket->payload = $data;
			}
		}
		$socket->fragment_status = 3;
		return true;
	}

	static public function sendFragment($socket, $payload, $opcode = 'text', $masked = true) {
		if (!in_array($opcode, array_keys(self::$opcodes))) {
			throw new \Exception('Bad opcode '.$opcode.', try text or binary.');
		}

		// record the length of the payload
		$payload_length = strlen($payload);

		if ($payload_length <= 0) {
			self::send_frame($socket, 1, NULL, $opcode, $masked);
			return;
		}

		$fragment_cursor = 0;
		// while we have data to send
		while ($payload_length > $fragment_cursor) {
			// get a fragment of the payload
			$sub_payload = substr($payload, $fragment_cursor, $socket->fragment_size);

			// advance the cursor
			$fragment_cursor += $socket->fragment_size;

			// is this the final fragment to send?
			$final = $payload_length <= $fragment_cursor;

			// send the fragment
			self::send_frame($socket, $final, $sub_payload, $opcode, $masked);

			// all fragments after the first will be marked a continuation
			$opcode = 'continuation';
		}

	}

	static public function send_frame2($socket, $final, $payload, $opcode, $masked) {
		// Binary string for header.
		$frame_head_binstr = '';

		// Write FIN, final fragment bit.
		$frame_head_binstr .= (bool) $final ? '1' : '0';

		// RSV 1, 2, & 3 false and unused.
		$frame_head_binstr .= '000';

		// Opcode rest of the byte.
		$frame_head_binstr .= sprintf('%04b', self::$opcodes[$opcode]);

		// Use masking?
		$frame_head_binstr .= $masked ? '1' : '0';

		// 7 bits of payload length...
		$payload_length = strlen($payload);
		if ($payload_length > 65535) {
			$frame_head_binstr .= decbin(127);
			$frame_head_binstr .= sprintf('%064b', $payload_length);
		}
		elseif ($payload_length > 125) {
			$frame_head_binstr .= decbin(126);
			$frame_head_binstr .= sprintf('%016b', $payload_length);
		}
		else {
			$frame_head_binstr .= sprintf('%07b', $payload_length);
		}

		$frame = '';

		// Write frame head to frame.
		foreach (str_split($frame_head_binstr, 8) as $binstr) $frame .= chr(bindec($binstr));

		// Handle masking
		if ($masked) {
			// generate a random mask:
			$mask = '';
			for ($i = 0; $i < 4; $i++) $mask .= chr(rand(0, 255));
			$frame .= $mask;
		}

		// Append payload to frame:
		for ($i = 0; $i < $payload_length; $i++) {
			$frame .= ($masked === true) ? $payload[$i] ^ $mask[$i % 4] : $payload[$i];
		}

		$socket->write($frame);
	}

	static public function send_frame($socket, $final, $payload, $opcode, $masked) {
		$first_byte = ($final ? 0x80 : 0x00 ) | self::$opcodes[$opcode]; // echo sprintf('%b', 0x80); sprintf('%x', $first_byte)
		$frame_head_binstr = chr($first_byte);

		$length_flag = $payload_length = strlen($payload);
		$pack = '';
		if ($payload_length > 65535) {
			$length_flag = 127;
			$pack   = \pack('NN', ($payload_length & 0xFFFFFFFF00000000) >> 32, $payload_length & 0x00000000FFFFFFFF); // 大端
		} elseif ($payload_length > 125) {
			$length_flag = 126;
			$pack   = \pack('n*', $payload_length);
		}

		$frame_head_binstr .=  chr((($masked ? 1 : 0)  << 7) | $length_flag).$pack;

		$frame = $frame_head_binstr;

		if ($masked) {
			$mask = '';
			for ($i = 0; $i < 4; $i++) $mask .= chr(rand(0, 255));
			$frame .= $mask;

			if ($payload_length) {
				$mask_key = \str_repeat($mask, \floor($payload_length / 4)) . \substr($mask, 0, $payload_length % 4);
				$frame .= $payload ^ $mask_key;
			}
		} else {
			$frame .= $payload;
		}

		$socket->write($frame);
	}

	/**
	 * Helper to convert a binary to a string of '0' and '1'.
	 */
	protected static function sprintB($string) {
		$ret = '';
		for ($i = 0; $i < strlen($string); $i++) {
			$ret .= sprintf("%08b", ord($string[$i]));
		}
		return $ret;
	}

	/**
	 * 解析数据包
	 *
	 * @return string
	 */
	static public function process($socket, $buffer)
	{
startfragment:
		self::info('process fragment_status:'.$socket->fragment_status.' buffer length:'.strlen($buffer));
		$socket->fragment_length = strlen($buffer);

		switch ($socket->fragment_status) {
			case 0:
				// Just read the main fragment information first.
				if (!self::readFragment0($socket, $buffer)) {
					return false;
				}
				goto startfragment;
				break;
			case 1:
				if (!self::readFragment1($socket, $buffer)) {
					return false;
				}
				goto startfragment;
				break;
			case 2:
				if (!self::readFragment2($socket, $buffer)) {
					return false;
				}
				goto startfragment;
				break;
			case 3:
			{
				if ($socket->last_opcode === 'close') {
					// Get the close status.
					if ($socket->payload_length >= 2) {
						$status_bin = $socket->payload[0] . $socket->payload[1];
						$status = bindec(sprintf("%08b%08b", ord($socket->payload[0]), ord($socket->payload[1])));
						$socket->close_status = $status;
						$socket->payload = substr($socket->payload, 2);

						self::sendFragment($socket, $status_bin . 'Close acknowledged: ' . $status, 'close', true); // Respond.
					}

					$socket->is_closing = true; // A close response, all done.
				}

				// if this is not the last fragment, then we need to save the payload
				if (!$socket->final) {
					$socket->huge_payload .= $socket->payload;
					self::info('final:'.$socket->final.', payload:'.$socket->payload);
					return false;
				} else {
					// sp we need to retreive the whole payload
					$socket->huge_payload .= $socket->payload;
					$socket->payload = $socket->huge_payload;
					$socket->huge_payload = null;
					self::info('final:'.$socket->final.', payload:'.$socket->payload);
					return true;
				}
				break;
			}
		}
		return false;
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
 * 客户端测试
 * sudo apt install node-ws
 * wscat -c ws://localhost:10001
 * curl -v http://localhost:10001/hello
 * 运行 php websocket-server.php
 */
if (isset($vals['debug'])) {
	Websocket::$debug = true;
}
$ws = new Websocket(\Phalcon\Arr::get($vals, 'server', '0.0.0.0'), \Phalcon\Arr::get($vals, 'port', 10001), function($socket, $headers, $path, $data) {

	if ($socket->last_opcode == 'ping') {
		Websocket::sendFragment($socket, NULL, 'pong');
		return;
	} elseif ($socket->last_opcode == 'pong') {
		return;
	}
	if ($path) {
		$handlerName = 'Index';
		$actionName = 'Index';
		$params = NULL;
		if (preg_match("#^/([a-zA-Z0-9_-]++)/?+$#", $path, $matches)) {
			$handlerName = $matches[1];
		} else if (preg_match("#^/([a-zA-Z0-9_-]++)/([a-zA-Z0-9\\._]++)(/.*+)?+$#", $path, $matches)) {
			$handlerName = $matches[1];
			$actionName = $matches[2];
			$params = isset($matches[3]) ? $matches[3] : NULL;
		}
		$handlerName = Phalcon\Text::camelize($handlerName);
		$handlerName .= 'Plugin';
		$actionName .= 'Action';
		if (class_exists($handlerName) && method_exists($handlerName, $actionName)) {
			$data = call_user_func($handlerName.'::'.$actionName, $data); 
		}
	}
	if ($socket->isHttp) {
		$sendchunk = \sprintf("HTTP/1.1 200 OK\r\nServer: webserver\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n%x\r\n%s\r\n0\r\n\r\n", \strlen($data), $data);
		$socket->write($sendchunk);
	} else {
		Websocket::sendFragment($socket, $data);
	}
}, \Phalcon\Arr::get($vals, 'concurrency', 500), \Phalcon\Arr::get($vals, 'capacity', 1));
$ws->start();
