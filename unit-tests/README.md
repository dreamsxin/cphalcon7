
Phalcon7 Unit Tests
==================

Thanks for reading this page, this folder includes all the unit tests we used to be sure that Phalcon7 will run properly and have a stable state. Some tests are specifically designed to test development version, so it's important to compile Phalcon7 extension from "dev/" directory instead of "release/".

You could run the test suite in your own machine. In order to run these tests, it's necessary to install phpunit (https://github.com/sebastianbergmann/phpunit/).

A MySQL database is also bundled, this is required for several tests. Follow these instructions to create the database:

    mysql -uroot -e 'create database phalcon_test charset=utf8 collate=utf8_unicode_ci;'
    mysql -uroot phalcon_test < unit-tests/schemas/mysql/phalcon_test.sql

Once the database is created, run the tests on console mode:

    cd cphalcon7/
    phpunit --debug -c unit-tests/phpunit.xml

Also, if you don't want to install phpunit or you can't. You can use this primitive command to run the test-suite. You'll get a exception if something fails.

    cd cphalcon7/
    php unit-tests/manual-unit.php

Additionally, the file cphalcon7/.travis.yml contains full instructions to test Phalcon7 on Ubuntu 11.10

Please report any issue if you find out bugs or memory leaks. Thanks!

Note: Cache unit-tests are slower than others tests because they make waits (sleeps) to expire generated caches.

Note: For testing on Windows copy folder php-tests/tests/app/configs to unit-tests/config as Windows doesn't recognize symbolic links
