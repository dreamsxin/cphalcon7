<?php

$redimage = Phalcon\Image\Vips::black(100, 100, ['bands' => 3]);
$redimage = $redimage->drawRect([255, 0, 0], 0, 0, 100, 60, ['fill' => 1]);

$blurimage = Phalcon\Image\Vips::black(50, 50, ['bands' => 3]);
$blurimage = $blurimage->drawRect([255, 0, 255], 0, 0, 50, 50, ['fill' => 1]);

$x = Phalcon\Image\Vips::call("composite", NULL, [$redimage->getVipsImage(), $blurimage->getVipsImage()], 2, [Phalcon\Image\Vips::BLENDMODE_OVER]);
Phalcon\Image\Vips::write_to_file($x["out"], __DIR__.'/img/composite.png');
