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
				//var_dump($buf);
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
	for ($i = 0; $i < 100000; $i++) {
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
		$num = 0;
		$status = 0;
		$len = 0;
		$alldata = '';
		while (null !== ($data = $a->read())) {
			$alldata .= $data;
		}

		$reader = new Phalcon\Binary\Reader($alldata);
		while (!$reader->isEof()) {
			$len = $reader->readInt16();
			if($len != 10) {
				break;
			}
			$buf = $reader->readString($len+1); // $bin->read($len);
			//var_dump($buf);
			$num++;
		}
		var_dump($num);
	} catch (\Throwable $e) {
		echo $e, "\n\n";
	} finally {
		$a->close();
	}
});

try {
	for ($i = 0; $i < 100000; $i++) {
		$bin = new Phalcon\Binary\Writer;
		$bin->writeInt16(strlen($chunk));
		$bin->writeString($chunk); // $bin->write($chunk, strlen($chunk));
		$b->write($bin->getContent());
	}
} finally {
	$b->close();
}
Phalcon\Async\Task::await($t);
echo 'End time '.time().PHP_EOL;
