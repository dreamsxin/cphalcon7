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

$py = <<<EOT
def Test(n):
    l = [1, 2, 3]
    l.append(n)
    return l
EOT;

# Execute the python code (in the __main__ module)
var_dump(Phalcon\Py::exec($py));

# Execute it the simple way.
var_dump(Phalcon\Py::exec('Test(4)'));

# Execute it the more interesting way.
var_dump(Phalcon\Py::callFunction('__main__', 'Test', 4));