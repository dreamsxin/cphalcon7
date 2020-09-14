# PHPSci CArray Extension

<p align="center">
  <img src="https://i.imgur.com/QoIbhqj.png" width="70%" />
</p>

PHPSci CArray is a high-performance scientific computing library for PHP developed in C and based on the original NumPy code. CArrays offer a solid alternative to PHP arrays as well as compatibility with codes developed using NumPy.

- High Performance Indexing and Data Access
- Low memory footprint compared to PHP Arrays
- Efficient shape, initializers, linear algebra and mathematical methods.
- Out of the box GPU integration (Cuda)

---

## Installing

It's really easy to compile this extension using Linux environments.

#### Requirements

- php-devel (php-dev)
- PHP 7.2
- OpenBLAS

#### Optional
- cuBLAS (For GPU Integration)

#### Ubuntu 16.04
```commandline
$ add-apt-repository -y ppa:ondrej/php
$ apt-get update
$ apt-get install libblas-dev libatlas-base-dev php7.2-dev
$ phpize
$ ./configure
$ make test
$ make install
```
#### Ubuntu 14.04
```commandline
$ add-apt-repository -y ppa:ondrej/php
$ apt-get update
$ apt-get install libopenblas-dev liblapacke-dev php7.2-dev
$ phpize
$ ./configure
$ make test
$ make install
```

> Don't forget to check if the extension is enabled in your php.ini file.

> **Apache/NGINX Users:** Don't forget to restart your services.
