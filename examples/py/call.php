<?php

function test() {
	return 'Test';
}

$py = <<<EOT
import phalcon

print phalcon.call('test')
print phalcon.call('sha1', ('tuple',))
print phalcon.call('sha1', ['list'])
EOT;

Phalcon\Py::exec($py);