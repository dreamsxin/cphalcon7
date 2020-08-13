<?php

$file = __DIR__.'/img/butterfly.jpg';
$size = array(
	'width' => 256,
	'height' => 100,
);

$image = new Imagick($file);
$histogram = array_fill_keys(range(0, 255), 0);
foreach ($image->getImageHistogram() as $pixel) {
	$rgb = $pixel->getColor();
	$histogram[$rgb['r']] += $pixel->getColorCount();
	$histogram[$rgb['g']] += $pixel->getColorCount();
	$histogram[$rgb['b']] += $pixel->getColorCount();
}

$max = max($histogram);
$threshold = ($image->getImageWidth() * $image->getImageHeight()) / 256 * 12;
if ($max > $threshold) {
	$max = $threshold;
}

$image = new Imagick();
$draw = new ImagickDraw();
$image->newImage($size['width'], $size['height'], 'white');

foreach ($histogram as $x => $count) {
	if ($count == 0) {
		continue;
	}
	$draw->setStrokeColor('black');
	$height = min($count, $max) / $max * $size['height'];
	$draw->line($x, $size['height'], $x, $size['height'] - $height);
	$image->drawImage($draw);
	$draw->clear();
}

$image->setImageFormat('png');
$image->writeImage('histogram.png'); 