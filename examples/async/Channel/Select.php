<?php

// Pass any truthy arg via cli to enable blocking select behavior.

namespace Phalcon\Async;

$producer = function (int $delay, Channel $channel) {
    try {
        $timer = new Timer($delay);
        $max = random_int(3, 5);

        for ($i = 0; $i < $max; $i++) {
            $timer->awaitTimeout();

            $channel->send($i);
        }
    } finally {
        $channel->close();
    }
};

$channel1 = new Channel();
$channel2 = new Channel();

Task::async($producer, 80, $channel1);
Task::async($producer, 110, $channel2);

$block = ((int) ($_SERVER['argv'][1] ?? '0')) ? true : false;

$group = new ChannelGroup([
    'A' => $channel1,
    'B' => $channel2->getIterator()
]);

do {
    $select = $group->select($block ? null : 25);
    
    switch ($select->key ?? null) {
        case 'A':
        case 'B':
            printf("%s >> %s\n", $select->key, $select->value);
            break;
        default:
            if (!$block) {
                echo "Await next poll...\n";
            }
    }
} while ($group->count());
