<?php

$n = 1000;
$x = [];
$y = [];
$z = [];

$plt = Phalcon\Py\Matplot::factory();
for($i=0; $i<$n; $i++) {
	$x[] = $i*$i;
	$y[] = sin(2*M_PI*$i/360.0);
	$z[] = log($i);

	if ($i % 10 == 0) {
		// Clear previous plot
		$plt->clf();
		// Plot line from given x and y data. Color is selected automatically.
		$plt->plot($x, $y);
		// Plot a line whose name will show up as "log(x)" in the legend.
		$plt->plot($x, $z, NULL, NULL, "log(x)");

		// Set x-axis to interval [0,1000000]
		$plt->xlim(0, $n*$n);

		// Add graph title
		$plt->title("Sample figure");
		// Enable legend.
		$plt->legend();
		// Display plot continuously
		$plt->pause(0.01);
	}
}