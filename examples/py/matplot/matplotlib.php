<?php

$plt = Phalcon\Py\Matplot::factory();
$plt->plot([1,2,3,4], NULL, "r--");
$plt->plot([1,2,3,4], [0.2,0.5,1,2], NULL, NULL, 'name2');
// Add graph title
$plt->title('Phalcon7');
// Enable legend.
$plt->legend();
$plt->show();
// $plt->save('filepath');