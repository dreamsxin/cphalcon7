<?php

function DrawRoundedRect($im, $x, $y, $width, $height, $col, $bg) {

	$radius = $width > $height ? $width/4 : $height/4;
	$diam = $radius*2;
	
	$im = $im->drawRect($col, $x, $y, $width, $height, ['fill' => true]);

	$im = $im->drawRect($bg, $x, $y, $radius, $radius, ['fill' => true]);
	$im = $im->drawRect($bg, $x+$width-$radius, $y, $radius, $radius, ['fill' => true]);
	$im = $im->drawRect($bg, $x, $y+$height-$radius, $radius, $radius, ['fill' => true]);
	$im = $im->drawRect($bg, $x+$width-$radius, $y+$height-$radius, $radius, $radius, ['fill' => true]);

	$im = $im->drawCircle($col, $x + $radius, $y + $radius, $radius, ['fill' => true]);
	$im = $im->drawCircle($col, $x + $width - $radius - 1, $y + $radius, $radius, ['fill' => true]);
	$im = $im->drawCircle($col, $x + $radius, $y + $height - $radius - 1, $radius, ['fill' => true]);
	$im = $im->drawCircle($col, $x + $width - $radius - 1, $y + $height - $radius - 1, $radius, ['fill' => true]);
	return $im;
}

$bg = Phalcon\Image::splitHexColor('000000');
$fg = Phalcon\Image::splitHexColor('FFCC00');
$image = Phalcon\Image\Vips::black(200, 200, ['bands' => 3]);
$image = DrawRoundedRect($image, 0, 0, 200, 200, $fg, $bg);
$image->writeToFile(__DIR__.'/img/roundedrect.png');
