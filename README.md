#[中文帮助](https://github.com/dreamsxin/cphalcon7/wiki)
[Phalcon 1.3](https://github.com/dreamsxin/cphalcon)

Phalcon7 Framework
=================

Phalcon is a web framework implemented as a C extension offering high performance and lower resource consumption.

Get Started
-----------

Phalcon is written in C with platform independence in mind. As a result, Phalcon is available on Microsoft Windows, GNU/Linux, and Mac OS X. You can either download a binary package for the system of your choice or build it from sources.

### Linux/Unix/Mac

On a Unix-based platform you can easily compile and install the extension from sources.

#### Requirements
Prerequisite packages are:

* PHP 7.0.x development resources
* GCC compiler (Linux/Solaris) or Xcode (Mac)

Ubuntu:

```bash
sudo add-apt-repository ppa:ondrej/php-7.0
sudo apt-get install php7-dev libpcre3-dev gcc make

# or compilation
----------------
cd php-src
 ./buildconf --force
./configure --prefix=/usr/local/php --with-config-file-path=/usr/local/php/etc --with-fpm-user=www-data --with-fpm-group=www-data --with-pdo-pgsql --with-pdo-mysql --with-pdo-sqlite  --with-iconv-dir --with-freetype-dir --with-jpeg-dir --with-png-dir --with-zlib --with-libxml-dir=/usr --enable-xml --disable-rpath --enable-bcmath --enable-shmop --enable-sysvsem --enable-inline-optimization --with-curl --enable-mbregex --enable-mbstring --with-mcrypt --enable-ftp --with-gd --enable-gd-native-ttf --with-openssl --with-mhash --enable-pcntl --enable-sockets --with-xmlrpc --enable-zip --enable-soap --without-pear --with-gettext --disable-fileinfo --enable-maintainer-zts --enable-phpdbg-debug --enable-debug
make -j4
sudo make install
```

Suse:

```bash
sudo zypper install php7-devel gcc make
```

CentOS/Fedora/RHEL

```bash
sudo yum install php-devel pcre-devel gcc make
```

Compilation
-----------

Follow these instructions to generate a binary extension for your platform:

```bash
git clone git://github.com/dreamsxin/cphalcon7.git
cd cphalcon/ext
phpize
make -j4
sudo make install
# or
/usr/local/php/bin/phpize
./configure CFLAGS="-g3 -O0 -std=gnu90 -Wall -Werror -Wno-error=uninitialized" --with-php-config=/usr/local/php/bin/php-config
```

Add the extension to your php.ini:

```bash
extension=phalcon.so
```

Finally, restart the webserver.


License
-------
Phalcon is open source software licensed under the New BSD License. See the docs/LICENSE.txt file for more information.
