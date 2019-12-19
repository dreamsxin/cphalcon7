<?php

var_dump(Phalcon\Async\Process\Process::isWorker());
var_dump($ipc = Phalcon\Async\Process\Process::connect());

$num = 0;
$tasks = [];
while ($tcp = Phalcon\Async\Network\TcpSocket::import($ipc)) {
	$num++;
	var_dump('SEND HANDLE '.$num);
	$tasks[] = Phalcon\Async\Task::async([
		$tcp,
		'write'
	], "GET /json HTTP/1.0\r\nHost: localhost\r\nConnection: close\r\n\r\n");

	$tasks[] = Phalcon\Async\Task::async(function() use ($tcp) {
		try {
			while (null !== ($chunk = $tcp->read())) {
				echo $chunk;
			}
		} finally {
			$tcp->close();
		}
	});
	if ($num >= 2) {
		break;
	}
}

foreach ($tasks as $t) {
	var_dump(Phalcon\Async\Task::await($t));
}
