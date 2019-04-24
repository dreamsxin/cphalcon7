<?php

$builder = new Phalcon\Async\Process\Builder(PHP_BINARY);
$builder = $builder->withStdoutPipe();
$builder->withStderrInherited();

$proc = $builder->daemon(__DIR__.'/process/daemon.php');
if ($proc->isRunning()) {
	echo "Running".PHP_EOL;
	$stdout = $proc->getStdout();
	try {
		if (null !== ($chunk = $stdout->read())) {
			echo $chunk;
		}
	} finally {
		$stdout->close();
	}
	if (stripos($chunk, 'OK') !== FALSE || stripos($chunk, 'EXISTS') !== FALSE) {
		$event = new Phalcon\Sync\Event("newtask");
		$event->fire();
		echo 'Fire'.PHP_EOL;
	} else {
		echo 'Error'.PHP_EOL;
	}
}
