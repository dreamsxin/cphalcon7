<?php

$signal = new Phalcon\Async\SignalWatcher(Phalcon\Async\SignalWatcher::SIGINT);
var_dump(getenv('PATH'));
echo "START: \"", getenv('MY_TITLE'), "\"\n";

$signal->awaitSignal();

echo "END!";

exit(7);
