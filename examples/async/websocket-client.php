<?php

class WebsocketClient
{
	static public $debug = false;
	protected $url;
	protected $options;
	protected $scheme;
	protected $host;
	protected $user;
	protected $pass;
	protected $port;
	protected $path;
	protected $fullpath;
	protected $query;
	protected $socket;
	protected $callback = null;

	protected static $opcodes = array(
		'continuation' => 0,
		'text'         => 1,
		'binary'       => 2,
		'close'        => 8,
		'ping'         => 9,
		'pong'         => 10,
	);

	public function __construct($url, $callback = NULL, array $options = NULL) {
		$this->url = $url;
		$this->options = $options;
		$url_parts = parse_url($this->url);
		if ($url_parts) {
			$this->scheme = \Phalcon\Arr::get($url_parts, 'scheme');
			$this->host = \Phalcon\Arr::get($url_parts, 'host');
			$this->user = isset($url_parts['user']) ? $url_parts['user'] : '';
			$this->pass = isset($url_parts['pass']) ? $url_parts['pass'] : '';
			$this->port = isset($url_parts['port']) ? $url_parts['port'] : ($this->scheme === 'wss' ? 443 : 80);
			$this->path = isset($url_parts['path']) ? $url_parts['path'] : '/';
			$this->query = isset($url_parts['query'])    ? $url_parts['query'] : '';
			$this->fragment = isset($url_parts['fragment']) ? $url_parts['fragment'] : '';
		}

		if (!in_array($this->scheme, array('ws', 'wss'))) {
			throw new \Exception(
				"Url should have scheme ws or wss, not '$scheme' from URI '$this->socket_uri' ."
			);
		}

		$this->fullpath = $this->path;
		if (!empty($this->query)) {
			$this->fullpath .= '?' . $this->query;
		}
		if (!empty($this->fragment)) {
			$this->fullpath .= '#' . $this->fragment;
		}
		if ($callback && \is_callable($callback)) {
			$callback = Closure::bind($callback, $this);
		}
		$this->callback = $callback;
	}

	public function __destruct() {
		if ($this->socket && !$this->socket->is_closing) {
			$this->socket->close();
			$this->socket = null;
		}
	}

	/**
	* Generate a random string for WebSocket key.
	* @return string Random string
	*/
	protected static function generateKey() {
		$chars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!"$&/()=[]{}0123456789';
		$key = '';
		$chars_length = strlen($chars);
		for ($i = 0; $i < 16; $i++) {
			$key .= $chars[mt_rand(0, $chars_length-1)];
		}
		return base64_encode($key);
	}

	public function sendHeader()
	{
		// Generate the WebSocket key.
		$this->socket->key = self::generateKey();

		// Default headers (using lowercase for simpler array_merge below).
		$headers = array(
			'Host'                  => $this->host . ":" . $this->port,
			'User-Agent'            => 'websocket-client-php',
			'Connection'            => 'Upgrade',
			'Upgrade'               => 'websocket',
			'Sec-WebSocket-Key'     => $this->socket->key,
			'Sec-Websocket-Version' => '13',
		);

		// Handle basic authentication.
		if ($this->user || $this->pass) {
			$headers['authorization'] = 'Basic ' . base64_encode($this->user . ':' . $this->pass) . "\r\n";
		}

		// Deprecated way of adding origin (use headers instead).
		if (isset($this->options['origin'])) {
			$headers['origin'] = $this->options['origin'];
		}

		// Add and override with headers from options.
		if (isset($this->options['headers'])) {
			$headers = array_merge($headers, array_change_key_case($this->options['headers']));
		}

		$header = "GET " . $this->fullpath . " HTTP/1.1\r\n";
		$headers = array_map(function($key, $value) {
			return $key.': '.$value;
		}, array_keys($headers), $headers);
		$header .= implode("\r\n", $headers) . "\r\n\r\n";

		// Send headers.
		$this->socket->write($header);
		return true;
	}

	public function connect()
	{
		$defer = new \Phalcon\Async\Deferred();
		try {
			$tls = NULL;
			if ($this->scheme == 'wss') {
				$tls = new \Phalcon\Async\TlsClientEncryption();
				$tls = $tls->withAllowSelfSigned(true);
			}
			$socket = \Phalcon\Async\Network\TcpSocket::connect($this->host, $this->port, $tls);
			$socket->is_closing = false;
			$socket->fragment_status = 0;
			$socket->fragment_length = 0;
			$socket->fragment_size = 4096;
			$socket->read_length = 0;
			$socket->huge_payload = '';
			$socket->payload = '';
			$socket->headers = NULL;
			$socket->isHandshake = false;
			$this->socket = $socket;
			$this->sendHeader();
			// 接收握手数据
			\Phalcon\Async\Task::async(function () use ($defer, $socket) {

				try {
					$buffer = '';
					while (!$socket->is_closing && null !== ($chunk = $socket->read())) {

						$buffer .= $chunk;
						if ($socket->isHandshake === false) {
							if (strpos($buffer, "\r\n\r\n")) {
								if ($this->handShake($socket, $buffer)) {
									$socket->isHandshake = true;
									$buffer = '';
									$defer->resolve('connected');
								}
							}
						} else {
							$defer->resolve('connected');
						}
					}
				} catch (\Throwable $e) {
					$defer->fail($e);
				}
				$socket->close();
				$socket->is_closing = true;
			});
		} catch (\Throwable $e) {
			self::err($e->getMessage());
			exit;
		}
		return $defer;
	}

	public function recv($stdin) {
		$socket = $this->socket;
		$callback = $this->callback;
		// 接收数据
		\Phalcon\Async\Task::async(function () use ($socket, $callback, $stdin) {

			try {
				$buffer = '';
				while (!$socket->is_closing && null !== ($chunk = $socket->read())) {

					$buffer .= $chunk;
					if ($this->process($socket, $buffer)) {
						$buffer = '';
						if ($callback && \is_callable($callback)) {
							$callback($socket->payload);
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
			}
			$socket->close();
			$socket->is_closing = true;

			echo PHP_EOL.\Phalcon\Cli\Color::success('disconnected');
			$stdin->close();
		});
	}
	public function isClose() {
		return !$this->socket || $this->socket->is_closing;
	}

	public function send($data)
	{
		if ($this->isClose()) {
			self::err("No connection established or connection closed");
			return false;
		}
		self::sendFragment($this->socket, $data);
	}

	public function handShake($socket, $buffer)
	{
		// Validate response.
		if (!preg_match('#Sec-WebSocket-Accept:(.*)$#mUi', $buffer, $matches)) {
			throw new \Exception('Connection failed: Server sent invalid upgrade response:'.PHP_EOL . $buffer);
		}

		$keyAccept = trim($matches[1]);
		$expectedResonse = base64_encode(pack('H*', sha1($this->socket->key . '258EAFA5-E914-47DA-95CA-C5AB0DC85B11')));

		if ($keyAccept !== $expectedResonse) {
		  throw new \Exception('Server sent bad upgrade response.');
		}

		$socket->isHandshake = true;
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

	static public function send_frame($socket, $final, $payload, $opcode, $masked) {
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

/**
 * 客户端测试
 * sudo apt install node-ws
 * php -d extension=phalcon.so websocket-client.php -c ws://localhost:10001
 */
error_reporting(-1);
ini_set('display_errors', (DIRECTORY_SEPARATOR == '\\') ? '0' : '1');

$opts = new \Phalcon\Cli\Options('Websocket CLI');
$opts->add([
    'type' => \Phalcon\Cli\Options::TYPE_STRING,
    'name' => 'connect',
    'shortName' => 'c',
    'required' => true,
	'help' => "-c, --connect <url>           connect to a websocket server",
]);
$vals = $opts->parse();
if (!isset($vals['connect'])) {
	return;
}

$stdin = Phalcon\Async\Stream\ReadablePipe::getStdin();
$stdout = Phalcon\Async\Stream\WritablePipe::getStdout();

// WebsocketClient::$debug = true;
$ws = new WebsocketClient($vals['connect'], function($data) use ($stdout) {
    $stdout->write(Phalcon\Cli\Color::colorize('< '.$data, Phalcon\Cli\Color::FG_BLUE, NULL, Phalcon\Cli\Color::BG_LIGHT_GRAY));
	$stdout->write(PHP_EOL.'> ');
});

$defer = $ws->connect();

try {

	\Phalcon\Async\Task::await(Phalcon\Async\Deferred::transform($defer->awaitable(), function (?\Throwable $e, ?string $v = null) use ($ws, $stdin, $stdout) {
		if ($e) {
			throw $e;
		}
		$ws->recv($stdin);
		$stdout->write('> ');
		while (null !== ($chunk = $stdin->read(100))) {

			$ws->send(trim($chunk));
		}
	}));
} catch (\Throwable $e) {
} finally {
    $stdout->close();
}
