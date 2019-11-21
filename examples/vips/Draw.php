<?php

$image = Phalcon\Image\Vips::black(150, 100, ['bands' => 3]);
$image = $image->drawRect([0, 0, 255], 0, 0, 50, 50, ['fill' => 1]);
$image = $image->drawRect([255, 255, 255], 50, 0, 50, 50, ['fill' => 1]);
$image = $image->drawRect([255, 0, 0], 100, 0, 50, 50, ['fill' => 1]);
$image = $image->drawRect([0, 255, 0], 50, 50, 50, 50, ['fill' => 1]);
$image->writeToFile(__DIR__.'/img/rect.png');

$circle = Phalcon\Image\Vips::black(100, 100, ['bands' => 3]);
$circle = $circle->drawCircle([255, 0, 0], 50, 50, 40, ['fill' => 1]);
$circle->writeToFile(__DIR__.'/img/circle.png');

$image2 = Phalcon\Image\Vips::black(200, 200, ['bands' => 3]);
$image2 = $image2->drawImage($image, 50, 50);
$floodimage->writeToFile(__DIR__.'/img/image2.png');

$floodimage = Phalcon\Image\Vips::black(100, 100, ['bands' => 3]);
$floodimage = $floodimage->drawFlood(0, 0, ['test' => $circle, 'equal' => true, 'left' => 0, 'top' => 0, 'width' => 50, 'height' => 50]);
$floodimage->writeToFile(__DIR__.'/img/circle.png');

$lineimage = $floodimage->drawLine([255, 0, 0], 0, 0, 100, 100);
$lineimage->writeToFile(__DIR__.'/img/line.png');

// 作为掩码的图片，颜色值决定了透明度，通道 bands 必须是 1
$mask = Phalcon\Image\Vips::black(51, 51, ['bands' => 1]);
$mask = $mask->drawCircle([128], 25, 25, 25, ['fill' => 1]);

$maskimage = Phalcon\Image\Vips::black(100, 100, ['bands' => 3]);
$maskimage = $maskimage->drawCircle([255, 0, 0], 50, 50, 40, ['fill' => 1]);
$maskimage = $maskimage->drawCircle([0, 0, 255], 50, 50, 30, ['fill' => 1]);
$maskimage = $maskimage->drawMask($mask, [0, 255, 0], 0, 0); // mask 图形绘制后的颜色
$maskimage->writeToFile(__DIR__.'/maskimage.png');

$smudgeimage = Phalcon\Image\Vips::black(150, 150, ['bands' => 3]);
$smudgeimage = $maskimage->drawCircle([255, 0, 0], 75, 75, 50, ['fill' => 1]);
$smudgeimage = $maskimage->drawCircle([0, 0, 255], 75, 75, 40, ['fill' => 1]);
$smudgeimage = $maskimage->drawSmudge(50, 50, 50, 50);
$maskimage->writeToFile(__DIR__.'/smudgeimage.png');

$text = Phalcon\Image\Vips::text('hello', ['font' => 'sans 120','width' => 100,'height' => 50]);
$text->writeToFile(__DIR__.'/text.png');

// Embed text in larger image
$bigtext = $text->embed(0, 0, 100, 100);
$text->writeToFile(__DIR__.'/bigtext.png');

// ifthenelse
$redimage = Phalcon\Image\Vips::black(300, 300, ['bands' => 3]);
$redimage = $redimage->drawRect([255, 0, 0], 0, 0, 300, 300, ['fill' => 1]);

$text = Phalcon\Image\Vips::text('hello', ['font' => 'sans 120', 'with' => 300, 'height' => 300]);
// 如果跟自己颜色相同就采用第一个图像的颜色，不同采用第二个图像的颜色
$out = $text->ifthenelse($redimage, $text, ['blend' => true]);
$out->writeToFile(__DIR__.'/ifthenelse.png');
