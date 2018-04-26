Phalcon7(Dao7) Framework
========================

Phalcon7 is a web framework implemented as a C extension offering high performance and lower resource consumption.

Phalcon7 是什么？
-----------------

Phalcon7 是继承自 Phalcon 1.3.x，开源、全功能栈、使用 C 编写、针对 PHP 7 优化的高性能框架。
开发者不需要学习和使用 C 语言的功能， 因为所有的功能都以 PHP 类的方式暴露出来，可以直接使用。
Phalcon7 源自 Phalcon 所以具备了 Phalcon 所有与生俱来的特性，是松耦合的，可以根据项目的需要任意使用其他对象。

Phalcon7 不只是为了卓越的性能, 我们的目标是让它更加健壮，拥有更加丰富的功能以及更加简单易于使用！

Phalcon7 版权申明
------------------
Phalcon7 作为 Phalcon 1.3 系列的继承者，所以版权理所当然的属于 Phalcon 官方团队所有。

Get Started
-----------

Phalcon7 is written in C with platform independence in mind. As a result, Phalcon7 is available on GNU/Linux, and Mac OS X. You can build it from sources.

### Linux/Unix/Mac

On a Unix-based platform you can easily compile and install the extension from sources.

#### Requirements
Prerequisite packages are:

* PHP 7.0.x development resources
* GCC compiler (Linux/Solaris) or Xcode (Mac)

Ubuntu:

```bash
# if enable qrcode
sudo apt-get install libqrencode-dev libzbar-dev imagemagick libmagick++-dev libmagickwand-dev libmagickcore-dev libpng12-dev
sudo ln -s /usr/include/ImageMagick-6/ /usr/include/ImageMagick

sudo add-apt-repository ppa:ondrej/php
sudo apt-get install php7.1-dev libpcre3-dev gcc make

# or compilation
----------------
cd php-src
 ./buildconf --force
./configure --prefix=/usr/local/php --with-config-file-path=/usr/local/php/etc --with-fpm-user=www-data --with-fpm-group=www-data --with-pdo-pgsql --with-pdo-mysql --with-pdo-sqlite  --with-iconv-dir --with-freetype-dir --with-jpeg-dir --with-png-dir --with-zlib --with-libxml-dir=/usr --enable-xml --disable-rpath --enable-bcmath --enable-shmop --enable-sysvsem --enable-inline-optimization --with-curl --enable-mbregex --enable-mbstring --with-mcrypt --enable-ftp --with-gd --enable-gd-native-ttf --with-openssl --with-mhash --enable-pcntl --enable-sockets --enable-zip --without-pear --with-gettext --disable-fileinfo --enable-maintainer-zts --enable-phpdbg-debug --enable-debug
make -j4
sudo make install

# class Phalcon\Async : --enable-sysvsem --enable-pcntl
```

Mac OS:

```bash
brew install pkg-config
brew install imagemagick
ln -s /usr/local/Cellar/imagemagick/6.9.7-0/include/ImageMagick-6/ /usr/local/Cellar/imagemagick/6.9.7-0/include/ImageMagick

brew install php70
```

Compilation
-----------

Follow these instructions to generate a binary extension for your platform:

```bash
git clone git://github.com/dreamsxin/cphalcon7.git
cd cphalcon7/ext
phpize

./configure

# or custom php path
/usr/local/php/bin/phpize
./configure --with-php-config=/usr/local/php/bin/php-config

# or debug
./configure CFLAGS="-g3 -O0 -std=gnu90 -Wall -Werror -Wno-error=uninitialized"

make -j4
sudo make install
```

Other options:
```shell
./configure --enable-chart=yes --enable-qrcode=no --enable-process=yes \
--enable-intrusive=yes --enable-cache-yac=yes \
--enable-storage-btree=yes --enable-storage-wiredtiger=yes \
--enable-storage-bloomfilter=yes --enable-storage-datrie=yes \
--enable-storage-lmdb=yes --enable-storage-libmdbx=yes \
--enable-storage-leveldb=yes --enable-websocket=yes \
--enable-server=yes --enable-python=yes
make -j4
sudo make install
```

Add the extension to your php.ini:

```ini
extension=phalcon.so
```

Test:

```shell
php --ri phalcon7

cd cphalcon7
composer install
vendor/bin/phpunit unit-tests
```

Finally, restart the webserver.

Current Build Status
--------------------

Phalcon7 Framework is built under the Travis CI service. Every commit pushed to this repository will queue a build into the continuous integration service and will run all PHPUnit tests to ensure that everything is going well and the project is stable. The current build status is:

[![Build Status](https://secure.travis-ci.org/dreamsxin/cphalcon7.png?branch=master)](http://travis-ci.org/dreamsxin/cphalcon7)

External Links
--------------

* [中文帮助](https://github.com/dreamsxin/cphalcon7/wiki)
* [捐贈名單（Donation）](https://github.com/dreamsxin/cphalcon7/blob/master/DONATE.md)
* PHP5 系列 [Phalcon 1.3](https://github.com/dreamsxin/cphalcon)
* Zephir 系列 [Phalcon](https://github.com/phalcon/cphalcon)

License
-------
Phalcon7 is open source software licensed under the New BSD License. See the docs/LICENSE.txt file for more information.
> Phalcon7 is the successor to the Phalcon 1.3 series, so the copyright belongs to the Phalcon team.
