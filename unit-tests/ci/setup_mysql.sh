#! /bin/sh

DIR=$(readlink -enq $(dirname $0))

(mysql -uroot -p -e 'create database phalcon_test charset=utf8 collate=utf8_unicode_ci;' && mysql -uroot -p phalcon_test < "$DIR/../schemas/mysql/phalcon_test.sql") &
wait
