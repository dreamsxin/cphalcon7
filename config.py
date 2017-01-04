#!/usr/bin/env python


import os
import stat
from subprocess import check_call, Popen


REPO = "https://github.com/dreamsxin/cphalcon7.git"


HOME_PATH = os.environ['HOME']
GIT_PATH = "/usr/bin/git"
APT_GET_PATH = "/usr/bin/apt-get"
PHP_PATH = "/usr/bin/php"


PHALCON_DIR = HOME_PATH + "/.phalcon"
PHALCON_COMMAND = PHALCON_DIR + "/cphalcon7/devtools/phalcon.php"
PHALCON_SCRIPT_BIN_PATH = "/usr/bin/phalcon"
CPHALCON_BUILD_DIR = PHALCON_DIR + "/cphalcon"


LIBS = ("git-core gcc autoconf make php7.0 php7.0-dev php7.0-cli")


PHP_INIT_FILE_PATH = ("/etc/php/7.0/cli/php.ini")


def devtools():
    print("Installing DevTools ... \n")
    os.chdir(PHALCON_DIR)
    os.chmod(PHALCON_COMMAND, stat.S_IXUSR | stat.S_IRUSR | stat.S_IXGRP |
             stat.S_IRGRP | stat.S_IXOTH | stat.S_IROTH)
    os.symlink(PHALCON_COMMAND, PHALCON_SCRIPT_BIN_PATH)
    print("Finish Installing DevTools \n")


def install_phalcon():
    print("Installing Phalcon7 ... \n")
    os.mkdir(PHALCON_DIR)
    os.chdir(PHALCON_DIR)
    check_call([GIT_PATH, "clone", REPO])
    os.chdir(CPHALCON_BUILD_DIR)
    proc = Popen("./debug.sh", shell=True, stdin=None, executable="/bin/bash")
    proc.wait()
    for _file in PHP_INIT_FILE_PATH:
        with open(_file, "a") as fd:
            fd.write("extension=phalcon.so")
            print("Writing in "+_file)
    print("Finish Installing Phalcon \n")


def install_dependencies():
    print("Installing Dependencies ... \n")
    proc = Popen(APT_GET_PATH + " -y install " + LIBS, shell=True, stdin=None,
                 executable="/bin/bash")
    proc.wait()
    print("Finish Installing Dependencies \n")


if __name__ == '__main__':
    install_dependencies()
    install_phalcon()
    devtools()
