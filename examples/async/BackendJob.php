<?php

$phpcode = <<<PHP
class BackendJob {
	public static function run() {
		while(1){
			echo time().PHP_EOL;
			sleep(1);
		}
	}
}
BackendJob::run();
PHP;

$builder = new Phalcon\Async\Process\ProcessBuilder(PHP_BINARY);
$builder = $builder->withStdoutInherited();
$builder = $builder->withStderrInherited();


$process = $builder->start('-r', $phpcode);

// 3 秒后结束
Phalcon\Async\Task::async(function () use ($process) {
    (new Phalcon\Async\Timer(3000))->awaitTimeout();
    $process->signal(Phalcon\Async\Signal::SIGINT);
});

$code = $process->join();
echo "\nEXIT CODE: ", $code, "\n";