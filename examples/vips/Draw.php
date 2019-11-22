<?php

$image = Phalcon\Image\Vips::black(150, 100, ['bands' => 3]);
$image = $image->drawRect([0, 0, 255], 0, 0, 50, 50, ['fill' => 1]);
$image = $image->drawRect([255, 255, 255], 50, 0, 50, 50, ['fill' => 1]);
$image = $image->drawRect([255, 0, 0], 100, 0, 50, 50, ['fill' => 1]);
$image = $image->drawRect([0, 255, 0], 50, 50, 50, 50, ['fill' => 1]);

$image->writeToFile(__DIR__.'/img/rect.png');
$buf = $image->writeToBuffer('.png');
$mem = $image->writeToMemory();

$crop = $image->crop(0, 0, 50, 50);
$crop->writeToFile(__DIR__.'/img/crop.png');

$smartcrop = $image->smartcrop(0, 0, 50, 50);
$smartcrop->writeToFile(__DIR__.'/img/smartcrop.png');

$resize = $image->resize(0.5);
$resize->writeToFile(__DIR__.'/img/resize.png');

$rotate = $image->rotate(30);
$rotate->writeToFile(__DIR__.'/img/rotate.png');

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
$maskimage->writeToFile(__DIR__.'/img/maskimage.png');

$smudgeimage = Phalcon\Image\Vips::black(150, 150, ['bands' => 3]);
$smudgeimage = $maskimage->drawCircle([255, 0, 0], 75, 75, 50, ['fill' => 1]);
$smudgeimage = $maskimage->drawCircle([0, 0, 255], 75, 75, 40, ['fill' => 1]);
$smudgeimage = $maskimage->drawSmudge(50, 50, 50, 50);
$maskimage->writeToFile(__DIR__.'/img/smudgeimage.png');

$text = Phalcon\Image\Vips::text('hello', ['font' => 'sans 120','width' => 100,'height' => 50]);
$text->writeToFile(__DIR__.'/img/text.png');

$text = Phalcon\Image\Vips::newFromFile(__DIR__.'/text.png');
$text = $text->drawCircle([255, 0, 0], 10, 10, 10, ['fill' => 1]);

// Embed text in larger image
$bigtext = $text->embed(0, 0, 100, 100);
$text->writeToFile(__DIR__.'/img/bigtext.png');

// ifthenelse
$redimage = Phalcon\Image\Vips::black(100, 50, ['bands' => 3]);
$redimage = $redimage->drawRect([255, 0, 0], 0, 0, 100, 50, ['fill' => 1]);

$text = Phalcon\Image\Vips::text('hello', ['width' => 100, 'height' => 50]);
$text->writeToFile(__DIR__.'/img/text.png');

/**
 * ifthenelse
 * 当 blend 为 true 时，使用公式（ out = (cond / 255) * in1 + (1 - cond / 255) * in2 ）生成新的颜色值
 * 否则，如果本身的颜色值为0，则输出 in2，非 0 输出 in1
 */
$text = $text->ifthenelse($redimage, $text);
$text->writeToFile(__DIR__.'/img/ifthenelse.png');

/**
 * relational
 * 通过比较颜色值的大小生成新的颜色值，颜色值比较如果结果为 true 设置为 255，false 设置为 0
 */
$redimage = Phalcon\Image\Vips::black(100, 100, ['bands' => 3]);
$redimage = $redimage->drawRect([255, 0, 0], 0, 0, 100, 60, ['fill' => 1]);
$redimage->writeToFile(__DIR__.'/img/redimage.png');
//var_dump($redimage->getpoint(0, 0));

$blurimage = Phalcon\Image\Vips::black(100, 100, ['bands' => 3]);
$blurimage = $blurimage->drawRect([0, 0, 255], 0, 40, 100, 60, ['fill' => 1]);
$blurimage->writeToFile(__DIR__.'/img/blurimage.png');
//var_dump($blurimage->getpoint(0, 0));

$relational = $redimage->relational($blurimage, Phalcon\Image\Vips::OP_RELATIONAL_LESSEQ);
$relational->writeToFile(__DIR__.'/img/relational.png');
//var_dump($relational->getpoint(0, 0));

$relational = $redimage->relational($blurimage, Phalcon\Image\Vips::OP_RELATIONAL_MOREEQ);
$relational->writeToFile(__DIR__.'/img/relational2.png');
//var_dump($relational->getpoint(0, 0));

$relational = $redimage->relationalConst(255, Phalcon\Image\Vips::OP_RELATIONAL_LESSEQ);
$relational->writeToFile(__DIR__.'/img/relational.png');
//var_dump($relational->getpoint(0, 0));

$relational = $redimage->relationalConst(255, Phalcon\Image\Vips::OP_RELATIONAL_MOREEQ);
$relational->writeToFile(__DIR__.'/img/relational2.png');
//var_dump($relational->getpoint(0, 0));

$add = $redimage->add($blurimage);
$subtract = $redimage->subtract($blurimage);
$multiply = $redimage->multiply($blurimage);
$divide = $redimage->divide($blurimage);

$linear = $redimage->linear(0.5, 1);
$linear->writeToFile(__DIR__.'/img/linear.png');

$lut = Phalcon\Image\Vips::identity();
$linear = $redimage->linear($lut->max(), 0);

$math = $blurimage->math(Phalcon\Image\Vips::OP_MATH_SIN);
$math2 = $blurimage->math2($redimage, Phalcon\Image\Vips::OP_MATH2_WOP);

$boolean = $blurimage->boolean($redimage, Phalcon\Image\Vips::OP_BOOLEAN_OR);
$boolean->writeToFile(__DIR__.'/boolean.png');
