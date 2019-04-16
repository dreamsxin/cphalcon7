<?php

// kill -9 `cat daemon.run`
$pidfile = __DIR__.'/daemon.run';
$fp = fopen($pidfile, 'w+');
if (!$fp) {
	echo 'File '.$pidfile.' fopen fail'.PHP_EOL;
	return;
}

if (!flock($fp, LOCK_EX | LOCK_NB)) {
	echo 'OK'.PHP_EOL;
	fclose($fp);
	return;
}
echo 'OK'.PHP_EOL;

fwrite($fp, getmypid());

$n = 0;
while (TRUE) {
	$event = new Phalcon\Sync\Event("newtask");
	$event->wait();

	file_put_contents(__DIR__.'/daemon.log', 'task'.$n.PHP_EOL, FILE_APPEND);
	$n++;
	if ($n > 10) {
		break;
	}
}

flock($fp, LOCK_UN);
fclose($fp);
