<?php

/**
 * Handles communication with a FastCGI application
 *
 * @author      Pierrick Charron <pierrick@adoy.net>
 * @version     1.0
 */
class FastCGIClient
{
    const VERSION_1            = 1;

    const BEGIN_REQUEST        = 1;
    const ABORT_REQUEST        = 2;
    const END_REQUEST          = 3;
    const PARAMS               = 4;
    const STDIN                = 5;
    const STDOUT               = 6;
    const STDERR               = 7;
    const DATA                 = 8;
    const GET_VALUES           = 9;
    const GET_VALUES_RESULT    = 10;
    const UNKNOWN_TYPE         = 11;
    const MAXTYPE              = self::UNKNOWN_TYPE;

    const RESPONDER            = 1;
    const AUTHORIZER           = 2;
    const FILTER               = 3;

    const REQUEST_COMPLETE     = 0;
    const CANT_MPX_CONN        = 1;
    const OVERLOADED           = 2;
    const UNKNOWN_ROLE         = 3;

    const HEADER_LEN           = 8;

    const REQ_STATE_WRITTEN    = 1;
    const REQ_STATE_OK         = 2;
    const REQ_STATE_ERR        = 3;
    const REQ_STATE_TIMED_OUT  = 4;

    const FCGI_MAX_LENGTH  = 0xffff;

    /**
     * Socket
     * @var resource
     */
    private $_sock = null;

    /**
     * Host
     * @var string
     */
    private $_host = null;

    /**
     * Port
     * @var int
     */
    private $_port = null;

    /**
     * Keep Alive
     * @var bool
     */
    private $_keepAlive = false;

    /**
     * Outstanding request statuses keyed by request id
     *
     * Each request is an array with following form:
     *
     *  array(
     *    'state' => REQ_STATE_*
     *    'response' => null | string
     *  )
     *
     * @var array
     */
    private $_requests = array();

    /**
     * Connect timeout in milliseconds
     * @var int
     */
    private $_connectTimeout = 5000;

    /**
     * Read/Write timeout in milliseconds
     * @var int
     */
    private $_readWriteTimeout = 5000;

    /**
     * Constructor
     *
     * @param string $host Host of the FastCGI application
     * @param int $port Port of the FastCGI application
     */
    public function __construct($host, $port)
    {
        $this->_sock = Phalcon\Async\Network\TcpSocket::connect($host, $port ? $port : NULL);

        $this->_host = $host;
        $this->_port = $port;
    }


    /**
     * Build a FastCGI packet
     *
     * @param int $type Type of the packet
     * @param string $content Content of the packet
     * @param int $requestId RequestId
     * @return string
     */
    private function buildPacket($type, $content, $requestId = 1)
    {
        $clen = strlen($content);
        return chr(self::VERSION_1)         /* version */
            . chr($type)                    /* type */
            . chr(($requestId >> 8) & 0xFF) /* requestIdB1 */
            . chr($requestId & 0xFF)        /* requestIdB0 */
            . chr(($clen >> 8 ) & 0xFF)     /* contentLengthB1 */
            . chr($clen & 0xFF)             /* contentLengthB0 */
            . chr(0)                        /* paddingLength */
            . chr(0)                        /* reserved */
            . $content;                     /* content */
    }

    /**
     * Build an FastCGI Name value pair
     *
     * @param string $name Name
     * @param string $value Value
     * @return string FastCGI Name value pair
     */
    private function buildNvpair($name, $value)
    {
        $nlen = strlen($name);
        $vlen = strlen($value);
        if ($nlen < 128) {
            /* nameLengthB0 */
            $nvpair = chr($nlen);
        } else {
            /* nameLengthB3 & nameLengthB2 & nameLengthB1 & nameLengthB0 */
            $nvpair = chr(($nlen >> 24) | 0x80) . chr(($nlen >> 16) & 0xFF) . chr(($nlen >> 8) & 0xFF) . chr($nlen & 0xFF);
        }
        if ($vlen < 128) {
            /* valueLengthB0 */
            $nvpair .= chr($vlen);
        } else {
            /* valueLengthB3 & valueLengthB2 & valueLengthB1 & valueLengthB0 */
            $nvpair .= chr(($vlen >> 24) | 0x80) . chr(($vlen >> 16) & 0xFF) . chr(($vlen >> 8) & 0xFF) . chr($vlen & 0xFF);
        }
        /* nameData & valueData */
        return $nvpair . $name . $value;
    }

    /**
     * Read a set of FastCGI Name value pairs
     *
     * @param string $data Data containing the set of FastCGI NVPair
     * @return array of NVPair
     */
    private function readNvpair($data, $length = null)
    {
        $array = array();

        if ($length === null) {
            $length = strlen($data);
        }

        $p = 0;

        while ($p != $length) {

            $nlen = ord($data[$p++]);
            if ($nlen >= 128) {
                $nlen = ($nlen & 0x7F << 24);
                $nlen |= (ord($data[$p++]) << 16);
                $nlen |= (ord($data[$p++]) << 8);
                $nlen |= (ord($data[$p++]));
            }
            $vlen = ord($data[$p++]);
            if ($vlen >= 128) {
                $vlen = ($nlen & 0x7F << 24);
                $vlen |= (ord($data[$p++]) << 16);
                $vlen |= (ord($data[$p++]) << 8);
                $vlen |= (ord($data[$p++]));
            }
            $array[substr($data, $p, $nlen)] = substr($data, $p+$nlen, $vlen);
            $p += ($nlen + $vlen);
        }

        return $array;
    }

    /**
     * Decode a FastCGI Packet
     *
     * @param string $data string containing all the packet
     * @return array
     */
    private function decodePacketHeader($data)
    {
        $ret = array();
        $ret['version']       = ord($data[0]);
        $ret['type']          = ord($data[1]);
        $ret['requestId']     = (ord($data[2]) << 8) + ord($data[3]);
        $ret['contentLength'] = (ord($data[4]) << 8) + ord($data[5]);
        $ret['paddingLength'] = ord($data[6]);
        $ret['reserved']      = ord($data[7]);
        return $ret;
    }

    /**
     * Read a FastCGI Packet
     *
     * @return array
     */
    private function readPacket()
    {
    if ($packet = $this->_sock->read(self::HEADER_LEN)) {
            $resp = $this->decodePacketHeader($packet);
            $resp['content'] = '';
            if ($resp['contentLength']) {
                $len  = $resp['contentLength'];
                while ($len && ($buf=$this->_sock->read($len)) !== false) {
                    $len -= strlen($buf);
                    $resp['content'] .= $buf;
                }
            }
            if ($resp['paddingLength']) {
                $buf = $this->_sock->read($resp['paddingLength']);
            }
            return $resp;
        } else {
            return false;
        }
    }

    /**
     * Get Information on the FastCGI application
     *
     * @param array $requestedInfo information to retrieve
     * @return array
     * @throws \Exception
     */
    public function getValues(array $requestedInfo)
    {
        $this->connect();

        $request = '';
        foreach ($requestedInfo as $info) {
            $request .= $this->buildNvpair($info, '');
        }
        fwrite($this->_sock, $this->buildPacket(self::GET_VALUES, $request, 0));

        $resp = $this->readPacket();
        if ($resp['type'] == self::GET_VALUES_RESULT) {
            return $this->readNvpair($resp['content'], $resp['length']);
        } else {
            throw new \Exception('Unexpected response type, expecting GET_VALUES_RESULT');
        }
    }

    /**
     * Execute a request to the FastCGI application
     *
     * @param array $params Array of parameters
     * @param string $stdin Content
     * @return string
     * @throws \Exception
     */
    public function request(array $params, $stdin = FALSE)
    {
        // Pick random number between 1 and max 16 bit unsigned int 65535
        $requestId = mt_rand(1, (1 << 16) - 1);

        // Using persistent sockets implies you want them keept alive by server!
        $keepAlive = intval($this->_keepAlive);

        $request = $this->buildPacket(self::BEGIN_REQUEST, chr(0) . chr(self::RESPONDER) . chr($keepAlive) . str_repeat(chr(0), 5), $requestId);

        $paramsRequest = '';
        foreach ($params as $key => $value) {
            $paramsRequest .= $this->buildNvpair($key, $value, $requestId);
        }
        if ($paramsRequest) {
            $request .= $this->buildPacket(self::PARAMS, $paramsRequest, $requestId);
        }
        $request .= $this->buildPacket(self::PARAMS, '', $requestId);

        if ($stdin) {
            while (strlen($stdin) > self::FCGI_MAX_LENGTH) {
                $chunkStdin = substr($stdin, 0, self::FCGI_MAX_LENGTH);
        $socket->write($request . $this->buildPacket(self::STDIN, $chunkStdin, $requestId));
                $stdin = substr($stdin, self::FCGI_MAX_LENGTH);
            }
            $request .= $this->buildPacket(self::STDIN, $stdin, $requestId);
        } else {
    	$request .= $this->buildPacket(self::STDIN, '', $requestId);
    }

        $this->_sock->write($request);

        $this->_requests[$requestId] = array(
            'state' => self::REQ_STATE_WRITTEN,
            'response' => null
        );

        while ($resp = $this->readPacket()) {
            if ($resp['type'] == self::STDOUT || $resp['type'] == self::STDERR) {
                if ($resp['type'] == self::STDERR) {
                    $this->_requests[$resp['requestId']]['state'] = self::REQ_STATE_ERR;
                }
                $this->_requests[$resp['requestId']]['response'] .= $resp['content'];
            }
            if ($resp['type'] == self::END_REQUEST) {
                $this->_requests[$resp['requestId']]['state'] = self::REQ_STATE_OK;
                if ($resp['requestId'] == $requestId) {
                    break;
                }
            }
        }

        if (!is_array($resp)) {
            throw new \Exception('Read failed');
        }

        switch (ord($resp['content'][4])) {
            case self::CANT_MPX_CONN:
                throw new \Exception('This app can\'t multiplex [CANT_MPX_CONN]');
                break;
            case self::OVERLOADED:
                throw new \Exception('New request rejected; too busy [OVERLOADED]');
                break;
            case self::UNKNOWN_ROLE:
                throw new \Exception('Role value not known [UNKNOWN_ROLE]');
                break;
            case self::REQUEST_COMPLETE:
                return $this->_requests[$requestId];
        }
    }
}

// php FastCGIClient.php --url=/home/server/work/test.php

$opts = new \Phalcon\Cli\Options('FastCGI CLI');
$opts->add([
    'type' => \Phalcon\Cli\Options::TYPE_STRING,
    'name' => 'host',
    'required' => false,
	'defaultValue' => 'localhost',
	'help' => "Default localhost"
]);
$opts->add([
    'type' => \Phalcon\Cli\Options::TYPE_INT,
    'name' => 'port',
    'required' => false,
	'defaultValue' => 9000,
	'help' => "Default 9000"
]);
$opts->add([
    'type' => \Phalcon\Cli\Options::TYPE_STRING,
    'name' => 'url',
    'required' => true,
	'help' => "Ex: /home/server/work/test.php"
]);
$vals = $opts->parse();
if ($vals === false ) {
	exit;
}

$url = parse_url(\Phalcon\Arr::get($vals, 'url'));
$file = '/'.basename($url['path']);
$uri = '/'.basename($url['path']) .'?'.$url['query'];
$params = array(
    'GATEWAY_INTERFACE' => 'FastCGI/1.0',
    'REQUEST_METHOD'    => 'GET',
    'SCRIPT_FILENAME'   => $url['path'],
    'SCRIPT_NAME'       => $file,
    'QUERY_STRING'      => $url['query'],
    'REQUEST_URI'       => $uri,
    'DOCUMENT_URI'      => $file,
    'SERVER_SOFTWARE'   => 'Phalcon7',
    'SERVER_ADDR'       => '127.0.0.1',
    'SERVER_PORT'       => '80',
    'SERVER_NAME'       => php_uname('n'),
    'SERVER_PROTOCOL'   => 'HTTP/1.1',
    'CONTENT_TYPE'      => '',
    'CONTENT_LENGTH'    => 0
);

$client = new FastCGIClient(\Phalcon\Arr::get($vals, 'host'), \Phalcon\Arr::get($vals, 'port'));
echo $client->request($params, false)['response'];