<?php

$sock = '/tmp/test.sock';

$client = stream_socket_client("unix://".$sock);
if ($client) {
	echo "ok\n";
	while (!feof($client)) {
		echo fgets($client, 1024);
    }
}
