<?php

// 用到了 sleep 需要设置 phalcon.async.timer = 1
namespace Phalcon\Async;
class Redis {
}

class RedisPool
{
    private $channel;
    private $capacity;
    private $count = 0;

    public function __construct(int $capacity = 0)
    {
        $this->capacity = $capacity;
        $this->channel = new Channel($capacity);
    }

    public function close(?\Throwable $e = null): void
    {
        $this->count = \PHP_INT_MAX;
        $this->channel->close($e);
    }

    public function push($v)
    {	
		// 桶满了会阻塞，此处判断为了能立即返回。
        if ($this->count >= $this->capacity) {
			echo 'push '.$v.' failed'.PHP_EOL;
			// $v->close();
			return false;
		}
		$this->count++;

		$this->channel->send([
			$v
		]);
		//echo 'push '.get_class($v).' success'.PHP_EOL;

        return true;
    }

    public function pop()
    {
		if ($this->count <=0 ) {
			return new Redis;
		}
		$iter = $this->channel->getIterator();
		while ($iter->valid()) {
			$this->count--;
			$v = $iter->current();
			return $v[0];
		}
    }
}

$pool = new RedisPool(10);

$jobs[] = Task::async(static function (RedisPool $pool) {
	while(1) {
		$v = $pool->pop();
		echo 'job0 pop '.get_class($v).PHP_EOL;
		sleep(1);
		$pool->push($v);
	}
	echo 'job0 exit'.PHP_EOL;
}, $pool);

$jobs[] = Task::async(static function (RedisPool $pool) {
	while(1) {
		$v = $pool->pop();
		echo 'job1 pop '.get_class($v).PHP_EOL;
		sleep(1);
		$pool->push($v);
	}
	echo 'job1 exit'.PHP_EOL;
}, $pool);

//$signal = new Signal(Signal::SIGINT);
//$signal->awaitSignal();
//$pool->close();

foreach ($jobs as $i => $job) {
    printf("#%u => %s\n", $i + 1, Task::await($job));
}
echo "END!";