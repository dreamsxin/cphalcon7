<?php

$domain = (\DIRECTORY_SEPARATOR == '\\') ? \STREAM_PF_INET : \STREAM_PF_UNIX;

list ($a, $b) = stream_socket_pair($domain, \STREAM_SOCK_STREAM, \STREAM_IPPROTO_IP);

foreach ([$a, $b] as $r) {
    stream_set_blocking($r, false);
    stream_set_read_buffer($r, 0);
    stream_set_write_buffer($r, 0);
}
$watcher = new Phalcon\Async\StreamWatcher($b);

Phalcon\Async\Task::async(function () use ($a, $watcher) {
    (new Phalcon\Async\Timer(500))->awaitTimeout();

    fwrite($a, 'Hello Socket :)');
    echo 'EOF!'.PHP_EOL;
});

Phalcon\Async\Task::async(function () use ($a, $watcher) {
    (new Phalcon\Async\Timer(700))->awaitTimeout();

    fwrite($a, 'Second Hello Socket :)');
    fclose($a);
    echo 'EOF!'.PHP_EOL;
});


while (\is_resource($b) && !\feof($b)) {
    $watcher->awaitReadable();

    var_dump(fread($b, 30));
}

fclose($b);

echo 'DONE!'.PHP_EOL;
