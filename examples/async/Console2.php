<?php

error_reporting(-1);
ini_set('display_errors', (DIRECTORY_SEPARATOR == '\\') ? '0' : '1');

$stdin = \Phalcon\Async\Stream\ReadablePipe::getStdin();

$builder = \Phalcon\Async\Process\ProcessBuilder::shell(true);
$builder = $builder->withStdoutPipe();
$builder = $builder->withStderrPipe();
//$builder = $builder->withCwd(__DIR__);
//$builder = $builder->withStdoutInherited();
//$builder = $builder->withStderrInherited();
$process = $builder->start();

$shellstdin = $process->getStdin();

$channel = new Phalcon\Async\Channel;

$reader = function (Phalcon\Async\Process\ReadablePipe $pipe) use ($channel) {
    try {
        try {
			while (($out = $pipe->read()) !== NULL) {
				echo $out;
				if ($out[-1] == chr(0x0a)) { //enter
					$channel->send('ok');
				}
			}
		} catch (\Exception $e) {
		}
    } finally {
        $pipe->close();
    }
};

$defer = new \Phalcon\Async\Deferred();

Phalcon\Async\Task::async($reader, $process->getStdout());
Phalcon\Async\Task::async($reader, $process->getStderr());

while (1) {
	echo 'php>';
	if (null !== ($chunk = $stdin->read()) && $process->isRunning()) {
		$shellstdin->write($chunk);
		$iter = $channel->getIterator();
		$iter->current();

		echo PHP_EOL;
	} else {
		echo Phalcon\Cli\Color::info('shell exit').PHP_EOL;
	}
}
echo 'close';

var_dump($process->join());
