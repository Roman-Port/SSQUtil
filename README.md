# SSQUtil

SSQUtil is a very simple utility program that reads status from a Source game server using the [Source Server Query](https://developer.valvesoftware.com/wiki/Server_queries).

It will return a return code from a configurable source and can dump strings (such as map name, server name, etc) to files.

It's been tested to work with Dino D-Day, but will likely work on all other Source 1 games.

## Build

This program uses CMake to build it. Run these commands to build:

```
git clone --recursive https://github.com/Roman-Port/SSQUtil
cd SSQUtil
mkdir build
cd build
cmake ..
make
```

Cloning recursively is important. If you forgot to do that, run the following command to grab the submodules:

```
git submodule update --init --recursive
```

## Usage

Invoke the program with no arguments to view help. Otherwise, these are the three main options:

* -h [hostname]
* -p [port]
* -c [return source]

Return source specifies what the status code of the program will be derived from. On error it will always be -1, otherwise the following numbers will pull from these sources:

1. Game ID (usually 0)
2. Current number of players
3. Max players
4. Current number of bots
5. Current number of players minus bots

Additionally, you can dump strings to files by specifying a filename with one of these arguments:

* --name-file [filename] (server name)
* --map-file [filename] (server map)
* --folder-file [filename] (server game folder's name)
* --game-file [filename] (server gamemode, at least for Dino D-Day)
* --version-file [filename] (server version, seems to always be 1.0.0.0 for Dino D-Day)
* --keywords-file [filename] (server keywords displayed in the server browser)

Finally, you can change the default timeout and retries with these arguments:

* -t [timeout in milliseconds]
* -r [retry count]
