#! /bin/sh

DIR=$(readlink -enq $(dirname $0))
sqlite3 /tmp/phalcon_test.sqlite < "$DIR/../schemas/sqlite/phalcon_test.sql"
wait
