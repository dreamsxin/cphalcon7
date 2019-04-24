<?php

error_reporting(-1);

$process = Phalcon\Async\Process\Builder::fork(__DIR__ . '/process/worker.php')->start();
$ipc = $process->getIpc();

try {
    $tcp = Phalcon\Async\Network\TcpSocket::connect('httpbin.org', 80);
    $tcp->writeAsync("GET /json HTTP/1.0\r\nHost: httpbin.org\r\nConnection: close\r\n\r\n");
    
    var_dump('SEND HANDLE');
    $tcp->export($ipc);
    $tcp->close();    
} finally {
    $ipc->close();
}

var_dump('AWAIT DATA...');
printf("\nEXIT CODE: %u\n", $process->join());

