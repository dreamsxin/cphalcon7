<?php
/*
JWT简介

1.JWT(Json Web Token)是实现token技术的一种解决方案,JWT由三部分组成: header(头)、payload(载体)、signature(签名)。
2.头 HS384 HS512 RS256 RS384 RS512 ES256 ES384 ES512 PS256 PS384有这几种
3.载体

iss：Issuer，发行者
sub：Subject，主题
aud：Audience，观众
exp：Expiration time，过期时间
nbf：Not before
iat：Issued at，发行时间
jti：JWT ID

*/
$hmackey = "example-hmac-key";
$payload = array(
    "data" => [
        "name" => "ZiHang Gao",
        "admin" => true
    ],
    "iss" => "http://example.org",
    "sub" => "1234567890",
);

// Array
// (
//    [data] => Array
//        (
//            [name] => ZiHang Gao
//            [admin] => 1
//        )
//
//    [iss] => http://example.org
//    [sub] => 1234567890
// )

// or would you prefer to use a static method call
$token = \Phalcon\JWT::encode($payload, $hmackey);
print_r($token.PHP_EOL);
$decoded_token = \Phalcon\JWT::decode($token, $hmackey);
print_r($decoded_token);

$payload = ['data' => 'data', 'exp' => time() - 1];

$token = \Phalcon\JWT::encode($payload, $hmackey, 'HS256');

try {
	$decoded_token = \Phalcon\JWT::decode($token, $hmackey, ['algorithm' => 'HS256']);
	print_r($decoded_token);
} catch (\Exception $e) {
	print_r($e);
}

$payload = ['data' => 'data', 'nbf' => time() + 3600];

$token = \Phalcon\JWT::encode($payload, $hmackey, 'HS256');

try {
	$decoded_token = \Phalcon\JWT::decode($token, $hmackey, ['algorithm' => 'HS256']);
	print_r($decoded_token);
} catch (\Exception $e) {
	print_r($e);
}

$payload = ['data' => 'data', 'aud' => ['Young', 'Old']];

$token = \Phalcon\JWT::encode($payload, $hmackey, 'HS256');

try {
	$decoded_token = \Phalcon\JWT::decode($token, $hmackey, ['aud' => ['Young'], 'algorithm' => 'HS256']);
} catch (\Exception $e) {
	print_r($e);
}

$payload = ['data' => 'data', 'jti' => md5('id')];

$token = \Phalcon\JWT::encode($payload, $hmackey, 'HS256');

try {
	$decoded_token = \Phalcon\JWT::decode($token, $hmackey, ['jti' => md5('id'), 'algorithm' => 'HS256']);
} catch (\Exception $e) {
	print_r($e);
}

