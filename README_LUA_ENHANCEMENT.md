Redis 6.0 with lua-enhancement
======================

## Features
  - support lua module script
  - add lua bigdecimal package

## Build from Source

  - **CentOS**

    **install dependencies**
    ```bash
    $ yum -y install git gcc tcl autoconf gcc-c++ glibc-static centos-release-scl
    $ yum -y install devtoolset-8
    ```

    **download redis source**
    ```bash
    $ curl -O -sSL https://github.com/FastHCA/redis/archive/refs/tags/6.0.16-lua-enhancement-xxxx.tar.gz
    $ tar zxvf 6.0.16-lua-enhancement-xxxx.tar.gz
    ```

    **compile**
    ```bash
    $ scl enable devtoolset-8 bash
    $ cd redis-6.0.16-lua-enhancement-xxxx
    $ make
    $ make install
    ```

