# Installing Buzz

## Requirements

Regardless of the installation type, you need the following packages:

* A UNIX system (Linux or MacOSX; Microsoft Windows is not supported)
* _g++_ >= 4.3 (on Linux) or _clang_ >= 3.1 (on MacOSX)
* _cmake_ >= 2.8.12

## Download
You can download the development sources through git:

```bash
git clone https://github.com/buzz-lang/Buzz.git buzz
```

## Standalone Installation
To install Buzz by itself, execute the following:

```bash
cd buzz
mkdir build && cd build
cmake ../src
make
sudo make install
```

On Linux, run this command too:
```bash
sudo ldconfig
```
