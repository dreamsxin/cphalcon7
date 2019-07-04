<?php

namespace Phalcon\Async;

class Pool
{
    private $channel;

    private $concurrency;

    private $count = 0;
    
    private $context;

    public function __construct(int $concurrency = 1, int $capacity = 0)
    {
        $this->concurrency = max(1, $concurrency);
        $this->channel = new Channel($capacity);

        $this->context = Context::background();
    }

    public function close(?\Throwable $e = null): void
    {
        $this->count = \PHP_INT_MAX;
        $this->channel->close($e);
    }

    public function submit(callable $work, ...$args): Awaitable
    {
        if ($this->count < $this->concurrency) {
            $this->count++;

            Task::asyncWithContext($this->context, static function (iterable $it) {
                foreach ($it as list ($defer, $context, $work, $args)) {
                    try {
                        $defer->resolve($context->run($work, ...$args));
                    } catch (\Throwable $e) {
                        $defer->fail($e);
                    }
                }
            }, $this->channel->getIterator());
        }

        $this->channel->send([
            $defer = new Deferred(),
            Context::current(),
            $work,
            $args
        ]);

        return $defer->awaitable();
    }
}

$work = function (array $input) {
    echo "Starting work...\n";

    (new Timer(random_int(300, 700)))->awaitTimeout();

    return array_sum($input);
};

$pool = new Pool((int) ($_SERVER['argv'][1] ?? 1), (int) ($_SERVER['argv'][2] ?? 0));
$jobs = [];

for ($i = 1; $i <= 9; $i++) {
    $jobs[] = $pool->submit($work, range(1, $i));
}

foreach ($jobs as $i => $job) {
    printf("#%u => %s\n", $i + 1, Task::await($job));
}
