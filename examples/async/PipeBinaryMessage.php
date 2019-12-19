<?php

error_reporting(-1);

$chunk = str_repeat('A', 10);

echo 'Start time '.time().PHP_EOL;
list ($a, $b) = Phalcon\Async\Network\Pipe::pair();

$t = Phalcon\Async\Task::async(function () use ($a) {
	try {
		$buf = '';
		$num = 0;

		while (null !== ($chr = $a->read(1))) {
			if ($chr == chr(0x0)) {
				var_dump($buf);
				$buf = '';
				$num++;
				continue;
			}
			$buf .= $chr;
		}

		var_dump($num);
	} catch (\Throwable $e) {
		echo $e, "\n\n";
	} finally {
		$a->close();
	}
});

try {
	for ($i = 0; $i < 100; $i++) {
		$b->write($chunk.chr(0x0));
	}
} finally {
	$b->close();
}
Phalcon\Async\Task::await($t);
echo '--- time '.time().PHP_EOL;

list ($a, $b) = Phalcon\Async\Network\Pipe::pair();

$t = Phalcon\Async\Task::async(function () use ($a) {
	try {
		$buf = '';
		$num = 0;
		$status = 0;
		$len = 0;
		$reader = new Phalcon\Binary\Reader;
		while (null !== ($data = $a->read())) {
			$reader->append($data);
			while (!$reader->isEof() && $status == 0) {
				if ($status == 0) {
					if ($reader->getEofPosition() - $reader->getPosition() >= 2) {
						$len = $reader->readInt16();
						$status = 1;
					}
				}
				if ($status == 1 && $reader->getEofPosition() - $reader->getPosition() >= $len) {
					$status = 0;	
					$num++;
					
					$buf = $reader->readString();
					var_dump($buf);
				}
			}
		}

		var_dump($num);
	} catch (\Throwable $e) {
		echo $e, "\n\n";
	} finally {
		$a->close();
	}
});

try {
	for ($i = 0; $i < 100; $i++) {
		$bin = new Phalcon\Binary\Writer;
		$bin->writeInt16(strlen($chunk));
		$bin->writeString($chunk);
		$b->write($bin->getContent());
	}
} finally {
	$b->close();
}
Phalcon\Async\Task::await($t);
echo 'End time '.time().PHP_EOL;
