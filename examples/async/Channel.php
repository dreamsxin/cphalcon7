<?php

// Pass an int arg via CLI to set the channel buffer size to the given number.

namespace Phalcon\Async;

class Wrap
{
    private $val;

    public function __construct($val)
    {
        $this->val = $val;
    }

    public function __toString()
    {
        return (string) $this->val;
    }
}

$work = function (string $label, int $delay, iterable $it) {
    $timer = new Timer($delay);

    try {
        $timer->awaitTimeout();

        foreach ($it as $v) {
            printf("%s -> %s\n", $label, $v);

            $timer->awaitTimeout();
        }
    } catch (ChannelClosedException $e) {
        printf("[%s] %s\n", get_class($e), $e->getMessage());
        printf("-> [%s] %s\n", get_class($e->getPrevious()), $e->getPrevious()->getMessage());
    } catch (\Throwable $e) {
        echo $e, "\n\n";
    }
};

$channel = new Channel((int) ($_SERVER['argv'][1] ?? 0));

Task::async($work, 'A', 50, $channel);
Task::async($work, 'B', 140, $channel->getIterator());

for ($i = 0; $i <= 20; $i++) {
    $channel->send(new Wrap($i));
    printf("C <- %u\n", $i);
}

$channel->close();
