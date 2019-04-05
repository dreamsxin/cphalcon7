<?php

namespace Concurrent;

$signal = new SignalWatcher(SignalWatcher::SIGINT);
var_dump(getenv('PATH'));
echo "START: \"", getenv('MY_TITLE'), "\"\n";

$signal->awaitSignal();

echo "END!";

exit(7);
