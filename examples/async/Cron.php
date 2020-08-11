<?php
/**
*    *    *    *    *    *
-    -    -    -    -    -
|    |    |    |    |    |
|    |    |    |    |    + year [optional]
|    |    |    |    +----- day of week (0 - 7) (Sunday=0 or 7)
|    |    |    +---------- month (1 - 12)
|    |    +--------------- day of month (1 - 31)
|    +-------------------- hour (0 - 23)
+------------------------- min (0 - 59)
*/
$loader = new Phalcon\Loader();

$loader->registerNamespaces(array(
	"Cron" => __DIR__."/Cron",
));

$loader->register();

$now = new \DateTime();

$schedule = new \Cron\Schedule('* * * * *');
var_dump($schedule->valid($now));

$schedule = new \Cron\Schedule('* * * * *');
var_dump($schedule->valid($now));

$schedule = new \Cron\Schedule('0 */1 * * *');
var_dump($schedule->valid($now));


$builder = new Phalcon\Async\Process\ProcessBuilder(PHP_BINARY);
$builder = $builder->withStdoutInherited();
$builder = $builder->withStderrInherited();

$process = $builder->start(__DIR__ . '/task.php');
$code = $process->join();
var_dump($code, $process);