<?php

namespace Phalcon\Async\Stream;

$in = new ReadableMemoryStream('Hello World');
$out = new WritableMemoryStream();

try {
    while (null !== ($chunk = $in->read(8))) {
        var_dump($chunk);
        $out->write($chunk);
    }
} finally {
    $in->close();
    $out->close();
}

var_dump($out->getContents());
