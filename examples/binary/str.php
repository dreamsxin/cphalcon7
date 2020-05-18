<?php

function str2bin($input) {
	if (!is_string($input)) return false;
	$value = unpack('H*', $input);
	return base_convert($value[1], 16, 2);
}
$str = "\x01\x01";
echo str2bin($str).PHP_EOL;
var_dump(Phalcon\Binary::toBitstring($str, true));

var_dump(Phalcon\Binary::getbit($str, 0));
var_dump(Phalcon\Binary::setbit($str, 0, 1));
var_dump(Phalcon\Binary::toBitstring($str, true));

var_dump(Phalcon\Binary::getbit($str, 0));
var_dump(Phalcon\Binary::setbit($str, 0, 0));
var_dump(Phalcon\Binary::toBitstring($str, true));

var_dump(Phalcon\Binary::getbit($str, 0));
