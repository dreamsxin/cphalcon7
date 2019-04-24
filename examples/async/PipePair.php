<?php

error_reporting(-1);

$chunk = str_repeat('A', 7000);

list ($a, $b) = Phalcon\Async\Network\Pipe::pair();

Phalcon\Async\Task::async(function () use ($a) {
    try {
        $len = 0;

        while (null !== ($buf = $a->read())) {
            $len += strlen($buf);

            if ($buf !== str_repeat('A', strlen($buf))) {
                throw new \Error('Corrupted data received');
            }
        }

        var_dump($len);
    } catch (\Throwable $e) {
        echo $e, "\n\n";
    } finally {
        $a->close();
    }
});

try {
    for ($i = 0; $i < 1000; $i++) {
        $b->write($chunk);
    }
} finally {
    $b->close();
}
