#!/usr/bin/env bash

DIR=$(readlink -enq $(dirname $0))
CFLAGS="-O2 -g3 -fno-strict-aliasing -std=gnu90";

pecl channel-update pecl.php.net

enable_extension() {

    if [ -z "$1" ]; then
        return 1;
    fi

    if [ -z $(php -m | grep $1) ]; then
        phpenv config-add "$DIR/$1.ini"
    fi

    return 0;
}

install_extension() {

    if [ "$2" ]; then
        pecl config-set preferred_state beta;
    fi

    declare INSTALLED=$(pecl list $1 | grep 'not installed')

    if [ -z "$1" ]; then
        return 1;
    fi

    if [ -z "${INSTALLED}" ]; then
        printf "\n" | pecl upgrade $1 &> /dev/null
    else
        printf "\n" | pecl install $1 &> /dev/null
    fi

    enable_extension $1

   return 0;
}

install_igbinary_php7() {
	git clone https://github.com/igbinary/igbinary7.git /tmp/igbinary
	cd /tmp/igbinary;

	$PHPIZE_BIN &> /dev/null
	./configure CFLAGS="-O2 -g" --silent --enable-phalcon &> /dev/null

	make --silent -j4 &> /dev/null
	make --silent install

	if [ -z $(php -m | grep igbinary) ]; then
        phpenv config-add "$DIR/igbinary.ini"
    fi
}

phpenv config-rm xdebug.ini

install_extension imagick
enable_extension memcached

# See https://pear.php.net/bugs/bug.php?id=21007
# printf "\n" | pecl install apcu
# printf "\n" | pecl install apcu_bc-beta
# echo "apc.enable_cli=On" >> ~/.phpenv/versions/$(phpenv version-name)/etc/php.ini
# printf "\n" | pecl install yaml-2.0.0RC8
# install_igbinary_php7

wait
