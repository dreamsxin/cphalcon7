<?php

function rc4($pwd, $data)
{
    $cipher      = '';
    $key[]       = "";
    $box[]       = "";
    $pwd_length  = strlen($pwd);
    $data_length = strlen($data);
    for ($i = 0; $i < 256; $i++) {
        $key[$i] = ord($pwd[$i % $pwd_length]);
        $box[$i] = $i;
    }
    for ($j = $i = 0; $i < 256; $i++) {
        $j       = ($j + $box[$i] + $key[$i]) % 256;
        $tmp     = $box[$i];
        $box[$i] = $box[$j];
        $box[$j] = $tmp;
    }
    for ($a = $j = $i = 0; $i < $data_length; $i++) {
        $a       = ($a + 1) % 256;
        $j       = ($j + $box[$a]) % 256;
        $tmp     = $box[$a];
        $box[$a] = $box[$j];
        $box[$j] = $tmp;
        $k       = $box[(($box[$a] + $box[$j]) % 256)];
        $cipher .= chr(ord($data[$i]) ^ $k);
    }
    return $cipher;
}
$key = 'f63dfeafe6bd2f74fedcf754c89d25ad';
$data = '{"name": "crypt"}';

$bin = rc4($key, $data);
var_dump($data, base64_encode($bin));

$crypt = new Phalcon\Crypt;
$crypt->setOptions(OPENSSL_RAW_DATA | OPENSSL_NO_PADDING);
$crypt->setMethod('rc4');
$data = $crypt->decrypt($bin, $key); // use Using encrypt method, the result is the same
var_dump($data);
$bin = $crypt->encrypt($data, $key); // use Using encrypt method, the result is the same
var_dump(base64_encode($bin));
