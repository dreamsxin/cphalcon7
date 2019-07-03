<?php

namespace Phalcon\Async\Stream;

error_reporting(-1);
ini_set('display_errors', (DIRECTORY_SEPARATOR == '\\') ? '0' : '1');

$stdin = ReadablePipe::getStdin();
$stdout = WritablePipe::getStdout();
$i = 0;

while (null !== ($chunk = $stdin->read(100))) {
    if (++$i > 10) {
        break;
    }

    $stdout->write(sprintf("CHUNK: %s\n", trim($chunk)));
}
