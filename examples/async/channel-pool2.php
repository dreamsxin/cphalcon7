<?php

namespace Phalcon\Async;

class Pool
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
			return false;
		}
		$this->count++;

		$this->channel->send([
			$v
		]);
		echo 'push '.$v.' success'.PHP_EOL;

        return true;
    }

    public function pop()
    {
		$iter = $this->channel->getIterator();

		while ($iter->valid()) {
			$this->count--;
			$v = $iter->current();
			return $v[0];
		}
    }
}

$pool = new Pool(10);

$job = Task::async(static function (Pool $pool) {
	while(($v = $pool->pop())) {
		echo 'pop '.$v.PHP_EOL;	
		sleep(1);
	}
}, $pool);

$job1 = Task::async(static function (Pool $pool) {
	for ($i = 1; $i <= 11; $i++) {
		$pool->push($i);
	}
}, $pool);

$job2 = Task::async(static function (Pool $pool) {
	for ($i = 12; $i <= 22; $i++) {
		$pool->push($i);
	}
}, $pool);
