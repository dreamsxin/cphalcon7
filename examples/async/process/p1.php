<?php

echo "START\n";
echo strtoupper(file_get_contents('php://stdin')), "\n";
echo "END!";

exit(1);
