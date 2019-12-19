<?php

error_reporting(-1);

$process = Phalcon\Async\Process\ProcessBuilder::fork(__DIR__ . '/process/worker2.php')->start();
$ipc = $process->getIpc();

try {
    $tcp = Phalcon\Async\Network\TcpSocket::connect('httpbin.org', 80);
    $tcp->export($ipc);
    $tcp->close();

    $tcp = Phalcon\Async\Network\TcpSocket::connect('baidu.org', 80);
    $tcp->export($ipc);
    $tcp->close();    
} finally {
    $ipc->close();
}

var_dump('AWAIT DATA...');
printf("\nEXIT CODE: %u\n", $process->join());
