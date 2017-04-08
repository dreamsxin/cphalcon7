# Phalcon7 Devtools

## What are Devtools?

This tools provide you useful scripts to generate code helping to develop faster and easy applications that use
with Phalcon framework.

## Requirements

* PHP >= 7.0
* Phalcon7 >= 1.0.0

## Installation via Git

Phalcon Devtools can be installed by using Git.

Just clone the repo and checkout the current branch:

```bash
cd ~
git clone https://github.com/dreamsxin/cphalcon7.git
cd cphalcon7/devtools
```

This method requires a little bit more of setup. Probably the best way would be to symlink
the `phalcon.php` to a directory in your `PATH`, so you can issue phalcon commands in each directory
where a phalcon project resides.

```bash
ln -s ~/phalcon-devtools/phalcon.php /usr/bin/phalcon
chmod ugo+x /usr/bin/phalcon
```

## Usage

To get a list of available commands just execute following:

```bash
phalcon commands help
```

This command should display something similar to:

```sh
$ phalcon list ?

Phalcon7 DevTools (1.0.0)

Help:
  Lists the commands available in Phalcon devtools

Available commands:
  commands         (alias of: list, enumerate)
  controller       (alias of: create-controller)
  model            (alias of: create-model)
  all-models       (alias of: create-all-models)
  project          (alias of: create-project)
  migration        (alias of: create-migration)
```

## Database adapter

Should add `adapter` parameter in your `db` config file (if you use not Mysql database).

For PostgreSql it will be something like:

```php
$config = [
  'host'     => 'localhost',
  'dbname'   => 'my_db_name',
  'username' => 'my_db_user',
  'password' => 'my_db_user_password',
  'adapter'  => 'Postgresql'
];
```
