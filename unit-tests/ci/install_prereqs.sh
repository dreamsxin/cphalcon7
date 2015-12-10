#! /bin/sh

DIR=$(readlink -enq $(dirname $0))

( pecl install apcu < /dev/null || ( pecl config-set preferred_state beta; pecl install apcu < /dev/null ) && phpenv config-add "$DIR/apcu.ini" ) &

CFLAGS="-O1 -g3 -fno-strict-aliasing" pecl install memcached < /dev/null &
CFLAGS="-O1 -g3 -fno-strict-aliasing" pecl install mongo < /dev/null &
CFLAGS="-O1 -g3 -fno-strict-aliasing" pecl install igbinary < /dev/null &
CFLAGS="-O1 -g3 -fno-strict-aliasing" pecl install imagick < /dev/null &
CFLAGS="-O1 -g3 -fno-strict-aliasing" pecl install yaml < /dev/null &

phpenv config-add "$DIR/memcached.ini"
phpenv config-add "$DIR/mongo.ini"
phpenv config-add "$DIR/redis.ini"
phpenv config-rm xdebug.ini
wait
