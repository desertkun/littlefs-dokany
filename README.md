# The little filesystem in user-space for Windows

Similar to [littlefs-fuse](https://github.com/ARMmbed/littlefs-fuse) but for Windows using [dokany](https://github.com/dokan-dev/dokany).

## Command line arguments

`littlefs_dokany.exe <mount point> <block device>`

Example:
`littlefs_dokany.exe K:\ E:`

This will mount a new device `K:` using existing device (`E:`, e.g. an SD card) as block storage.

* `<mount point>` A folder to mount the filesystem to. Can be a new drive or existing folder (`C:/...`)
* `<block device>` block storage for the filesystem. Can be either drive letter (`E:`) 
  or `PhysicalDriveN` (you can see your devices using `wmic diskdrive list brief`)
* `-d` Enable debug mode
* `--block-size <size>` Specify block size (default 8192)
* `--unit-size <size>` Specify read/prog size (default 512)

## How to build
* Init the git repo submodules properly
* Install the [dokany driver](https://github.com/dokan-dev/dokany/wiki/Installation)
* Install [CMake](https://cmake.org/download/)
* Install Visual Studio (or CLion)
* run `generate-project.bat`
* Open the `win/littlefs_dokany.sln`
* Build
* Run it: `\win\Debug>littlefs_dokany.exe K:\ E:`

## Current issues
* The mounted drives shows up as `0 bytes free of 0 bytes`
* It's very slow
* Creating a new file in place leads to an error but later when refresh the folder, it's there.
  Other than that the file manipulations work fine.