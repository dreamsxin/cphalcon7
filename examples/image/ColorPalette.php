<?php

function ColorPalette($imageFile, $numColors, $granularity = 5) 
{ 
	$granularity = max(1, abs((int)$granularity)); 
	$colors = array(); 
	$size = @getimagesize($imageFile); 
	if($size === false) { 
		user_error("Unable to get image size data"); 
		return false; 
	} 
	$img = @imagecreatefromjpeg($imageFile);

	if(!$img) { 
		user_error("Unable to open image file"); 
		return false; 
	} 
	for($x = 0; $x < $size[0]; $x += $granularity) { 
		for($y = 0; $y < $size[1]; $y += $granularity) { 
			$thisColor = imagecolorat($img, $x, $y); 
			$rgb = imagecolorsforindex($img, $thisColor); 
			$red = round(round(($rgb['red'] / 0x33)) * 0x33); 
			$green = round(round(($rgb['green'] / 0x33)) * 0x33); 
			$blue = round(round(($rgb['blue'] / 0x33)) * 0x33); 
			$thisRGB = sprintf('%02X%02X%02X', $red, $green, $blue); 
			if(array_key_exists($thisRGB, $colors)) { 
				$colors[$thisRGB]++; 
			} else { 
				$colors[$thisRGB] = 1; 
			} 
		} 
	} 
	arsort($colors); 
	return array_slice(array_keys($colors), 0, $numColors); 
} 

$file = __DIR__.'/img/butterfly.jpg';
$palette = ColorPalette($file, 10, 4); 

foreach($palette as $color) {
	echo $color.PHP_EOL;
}
echo "<table>".PHP_EOL;
foreach($palette as $color) { 
	echo "<tr><td style='background-color:#$color;width:2em;'>&nbsp;</td><td>#$color</td></tr>".PHP_EOL;
}
echo "</table>".PHP_EOL;

echo '-------'.PHP_EOL;
$image = new \Imagick($file);
$image->quantizeImage(10, \Imagick::COLORSPACE_RGB, 0, false, false); // 减少到10种颜色
$image->uniqueImageColors();

foreach ($image->getPixelIterator() as $pixels) {

	foreach ($pixels as $pixel) {
		$color = $pixel->getColor();
		echo dechex($color['r']).dechex($color['g']).dechex($color['b']).PHP_EOL;
		echo 'rgba('.$color['r'].','.$color['g'].','.$color['b'].','.$color['a'].')'.PHP_EOL;
	}
}
echo "<table>".PHP_EOL;
foreach ($image->getPixelIterator() as $pixels) {

	foreach ($pixels as $pixel) {
		$color = $pixel->getColor();
		$color = dechex($color['r']).dechex($color['g']).dechex($color['b']).PHP_EOL;
		echo "<tr><td style='background-color:#$color;width:2em;'>&nbsp;</td><td>#$color</td></tr>".PHP_EOL;
	}
}
echo "</table>".PHP_EOL;