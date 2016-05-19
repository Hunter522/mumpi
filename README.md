# mumpi

*mumpi* is a simple mumble client daemon for the RaspberryPi. Mumpi aims to have
the following features:
 * simple mumble client that auto-connects to server
 * allows user to use a Push-To-Talk (PTT) button to speak and receive audio
 through speakers
 * uses external PTT button wired to GPIO pins
 * plug-and-play functionality by using default audio input and output devices
 * configurable using a simple properties file
 * runs as daemon (headless)
 * runs at startup

Currently this project has only been tested on Raspbian Jessie.

## Future

 * allow user to switch channels using buttons/switches
 * LCD display support


## Dependencies

* [mumlib](https://github.com/slomkowski/mumlib)
 * Boost libraries
 * OpenSSL
 * log4cpp
 * Opus library
 * Google Protobuf: libraries and compiler
* [PortAudio](http://www.portaudio.com/)
* [CMake](https://cmake.org/)

All of these dependencies (except mumlib) can be found in most popular Linux distro
package manager repositories. Mumlib is used in this project as a Git submodule and is
therefor built and linked when this project is built.

To download and install these dependencies on latest Raspbian Jessie:

```
apt-get install libboost-all-dev libssl-dev liblog4cpp5-dev libopus-dev protobuf-compiler libprotobuf-dev portaudio19-dev cmake
```

## Build

To build, run the following commands:
```
mkdir build && cd build
cmake ..
make
```
And to install (by default, installs into /usr/local/bin)
```
sudo make install
```
The *install* target should also install mumpi as a daemon service.

TODO: Cross compile

## Usage

##### Configuration

##### Running

TODO: add usage

## License

TODO: add license information
