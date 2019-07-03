<?php

namespace Phalcon\Async;

use Phalcon\Async\Component;
use Phalcon\Async\DNS\Config;
use Phalcon\Async\DNS\Query;
use Phalcon\Async\DNS\Resolver;
use Phalcon\Async\Network\UdpDatagram;
use Phalcon\Async\Network\UdpSocket;

error_reporting(-1);

class UdpResolver implements Resolver, Component
{
    private $socket;

    private $requests = [];

    public function __construct(string $ip, int $port)
    {
        if (\filter_var($ip, \FILTER_VALIDATE_IP) === false) {
            throw new \InvalidArgumentException(\sprintf('Invalid IP address: "%s"', $ip));
        }
        
        if ($port < 0 || $port > 0xFFFF) {
            throw new \InvalidArgumentException(\sprintf('Invalid port number: %d', $port));
        }
        
        $this->socket = UdpSocket::connect($ip, $port);

        Task::asyncWithContext(Context::background(), function () {
            try {
                while (true) {
                    $datagram = $this->socket->receive();
                    $header = \unpack('nid/nflags/nqc/nac/nauth/nadd', $datagram->data);

                    if (isset($this->requests[$header['id']])) {
                        $this->requests[$header['id']]->resolve([
                            $header,
                            $datagram->data
                        ]);
                    }
                }
            } catch (\Throwable $e) {
                foreach ($this->requests as $defer) {
                    $defer->fail($e);
                }

                throw $e;
            }
        });
    }

    public function shutdown(): void
    {
        $this->socket->close();
    }

    public function search(Query $query): void
    {
        foreach ($query->getTypes() as $type) {
            switch ($type) {
                case Query::A:
                case Query::AAAA:
                case Query::MX:
                    $this->query($query, $type);
                    break;
            }
        }
    }

    private function query(Query $query, int $type): void
    {
        $id = random_int(1, 0xFFFF);

        $buffer = pack('n*', $id, 1 << 8, 1, 0, 0, 0);
        $buffer .= $this->encodeLabel($query->host);
        $buffer .= pack('nn', $type, 1);

        $this->requests[$id] = $defer = new Deferred();

        try {
            $this->socket->send(new UdpDatagram($buffer));

            list ($header, $buffer) = Task::await($defer->awaitable());
        } finally {
            unset($this->requests[$id]);
        }
        
        $cache = [];
        $pos = 12;

        for ($i = 0; $i < $header['qc']; $i++) {
            $this->readLabel($buffer, $pos, $cache);
            \unpack('ntype/nclass', $buffer, 4);
            $pos += 4;
        }
        
        for ($i = 0; $i < $header['ac']; $i++) {
            $this->readLabel($buffer, $pos, $cache);
            $answer = \unpack('ntype/nclass/Nttl/nlen', $buffer, $pos);
            $pos += 10;

            if ($answer['type'] == $type) {
                if ($type == Query::MX) {
                    $query->addRecord($type, $answer['ttl'], [
                        'pri' => 1,
                        'target' => 'mx.foo.bar'
                    ]);
                } else {
                    $ip = \inet_ntop(\substr($buffer, $pos, $answer['len']));

                    $query->addRecord($type, $answer['ttl'], [
                        (($type == Query::A) ? 'ip' : 'ipv6') => $ip
                    ]);
                }
            }
            
            $pos += $answer['len'];
        }
    }

    private function encodeLabel(string $label): string
    {
        $encoded = '';

        foreach (\explode('.', $label) as $part) {
            $encoded .= \chr(\strlen($part)) . $part;
        }

        return $encoded . "\x00";
    }

    private function readLabel(string $buffer, int & $pos, array & $cache): string
    {
        $labels = [];

        while (true) {
            $index = $pos;

            $b1 = $buffer[$pos++];
            $len = \ord($b1);

            if (0 === $len) {
                break;
            }

            $b2 = $buffer[$pos++];
            $offset = \unpack('n', $b1 . $b2)[1];

            if (($len & 0xC0) === 0xC0) {
                $offset &= 0x3FFF;

                if (!isset($cache[$offset])) {
                    throw new \RuntimeException(\sprintf('Invalid label compression offset: %u', $offset));
                }

                $labels[] = [
                    $index,
                    $cache[$offset]
                ];

                break;
            }

            if ($len > 63) {
                throw new \RuntimeException(\sprintf('Invalid DNS label length: %u', $len));
            }

            $labels[] = [
                $index,
                ($len > 1) ? ($b2 . \substr($buffer, $pos, $len - 1)) : $b2
            ];

            $pos += $len - 1;
        }

        for ($size = \count($labels), $i = 0; $i < $size; $i++) {
            $cache[$labels[$i][0]] = \implode('.', \array_column(\array_slice($labels, $i), 1));
        }

        return \implode('.', \array_column($labels, 1));
    }
}

print_r(Config::getHosts());
print_r(Config::getNameservers());

TaskScheduler::register(Resolver::class, function () {
    foreach (Config::getNameservers() as $ip => $port) {
        return new UdpResolver($ip, $port);
    }
});

if (get_included_files()[0] == __FILE__) {
    $mx = null;
    $weight = null;
    var_dump(dns_get_mx('goolge.com', $mx, $weight));
    print_r($mx);
    print_r($weight);
}

var_dump(checkdnsrr('google.com'));
