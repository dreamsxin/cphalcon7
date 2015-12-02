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
sudo apt-get install php5-dev libpcre3-dev gcc make
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
```

Add the extension to your php.ini:

```bash
extension=phalcon.so
```

Finally, restart the webserver.


License
-------
Phalcon is open source software licensed under the New BSD License. See the docs/LICENSE.txt file for more information.
