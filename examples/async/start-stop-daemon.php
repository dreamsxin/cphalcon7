<?php

$name = 'TcpServer.php';
$runfile = __DIR__.'/TcpServer.php';
$pidfile = __DIR__.'/TcpServer.pid';

if ($argc < 2 || !in_array($argv[1], ['start', 'stop', 'restart'])) {
	echo Phalcon\Cli\Color::info("Usage: php start-top-daemon.php {start|stop|restart}");
	exit;
}

if ($argv[1] == 'stop' || $argv[1] == 'restart') {
	$pid = file_get_contents($pidfile);

	if (empty($pid)) {
		echo Phalcon\Cli\Color::error("Can't get pid");
		exit;
	}

	$builder = Phalcon\Async\Process\ProcessBuilder::shell();
	$builder2 = $builder->withStderrInherited();
	$builder = $builder2->withStdoutPipe();

	$reader = function (Phalcon\Async\Process\ReadablePipe $pipe, int $len) {
		$ret = '';
		try {
			while (null !== ($chunk = $pipe->read($len))) {
				$ret .= $chunk;
			}
		} finally {
			$pipe->close();
		}
		return $ret;
	};

	$process = $builder->start('ps -o pid,command --no-heading --pid '.$pid);

	$t = \Phalcon\Async\Task::async($reader, $process->getStdout(), 256);

	$code = $process->join();
	$out = \Phalcon\Async\Task::await($t);

	if (stripos($out, $run) === false) {
		echo Phalcon\Cli\Color::error("Error pid ".$pid);
		exit;
	}

	$builder = $builder2->withStdoutInherited();
	$builder->execute('kill -9 '.$pid.' && echo KILL SUCCESS');
}

if ($argv[1] == 'start' || $argv[1] == 'restart') {
	// 启动 web 服务
	$builder = new Phalcon\Async\Process\ProcessBuilder(PHP_BINARY);
	$builder = $builder->withStdoutInherited();
	$builder = $builder->withStderrInherited();

	$process = $builder->daemon($runfile);
}


