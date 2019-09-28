<?php

$sock = '/tmp/test.sock';
@unlink($sock);

$server = stream_socket_server("unix://".$sock, $errno, $errstr);;
if ($server) {
	while ($conn = stream_socket_accept($server)) {
		echo $msg = 'The local time is ' . date('n/j/Y g:i a') . "\n";
		fwrite($conn, $msg);
		fclose($conn);
	}
}