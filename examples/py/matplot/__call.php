<?php

$plt = Phalcon\Py\Matplot::factory();
$plt->bar(['args' => [[2,4,6,8,10], [2,4,6,8,10]]]);
$plt->show();
