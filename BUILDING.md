# Building from sources

I'm creating this document as an outside contributor.  As such, only my
current build environment (Ubuntu) is explained.  The core authors can 
probably fill it out further.

## Bootstrapping your environment

### Install Python 2.7

    $ sudo apt install python2.7

### Install node 7 or better (time of writing: 9.7.1)

    $ wget -qO- https://raw.githubusercontent.com/creationix/nvm/v0.33.8/install.sh | bash
    $ export NVM_DIR="$HOME/.nvm"
    $ [ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"
    $ nvm install 9.7.1

### Install library deps

    $ sudo apt install libwebkit2gtk-4.0-dev

### Run the bootstrapping script to get a toolchain and make the `out` directories

    $ node scripts/bootstrap.js

## Running the build script

    $ node scripts/build.js

Now take a nap while ninja does its thing.
