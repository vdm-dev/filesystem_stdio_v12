# File System Standard IO (Version 12)
This library is needed to run the old 2005 version of **Half-Life 2 Deathmatch**.
It is possible that other Source games will run with this library too.

## Screenshot
![Half-Life 2 Deathmatch 2005](https://github.com/vdm-dev/filesystem_stdio_v12/raw/main/screenshot.jpg)

## Build
For Visual Studio 2019 or newer, run the following commands:
```
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A Win32 ..
```

For older version of Visual Studio and CMake version below 3.9, run the following commands:
```
mkdir build
cd build
cmake -G "Visual Studio 7 .NET 2003" ..
```
