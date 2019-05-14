<?php

namespace Phalcon\Async;

$domain = (\DIRECTORY_SEPARATOR == '\\') ? \STREAM_PF_INET : \STREAM_PF_UNIX;

list ($a, $b) = stream_socket_pair($domain, \STREAM_SOCK_STREAM, \STREAM_IPPROTO_IP);

foreach ([$a, $b] as $r) {
    stream_set_blocking($r, false);
    stream_set_read_buffer($r, 0);
    stream_set_write_buffer($r, 0);
}

$poll = new Poll($b);

task::async(function () use ($a, $poll) {
    (new Timer(500))->awaitTimeout();
    
    fwrite($a, 'Hello Socket :)');
    fclose($a);
    var_dump('EOF!');

//     $poll->close(new \LogicException('Nope!'));
});

var_dump('WAIT FOR IO...');

while (\is_resource($b) && !\feof($b)) {
    $poll->awaitReadable();
    
    var_dump(fread($b, 4));
}

fclose($b);

var_dump('DONE!');
