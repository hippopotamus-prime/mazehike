# Maze Hiker
> A raycasting demo and maze generator for PalmOS

![Screenshot 1](https://i.imgur.com/HKbq8Cd.gif)
![Screenshot 2](https://i.imgur.com/r6WNO4h.gif)

## About
Maze Hiker is a maze generator / solver with a 3D raycasting engine (think
Wolfenstein 3D). It's intended for older m68k-based PalmOS devices, but should
run on newer ones too.

The demo was written for CodeJedi's
[Old School PalmOS Coding Contest 2003](http://www.codejedi.com/shadowplan/contest.html).
There was never a plan to develop the raycasting engine into a game - it was
just a demo to show what was possible on low-end Palm hardware at the time.

The raycasting engine is written in 68k assembly while the maze generator and
other setup code is in C.

## Free Software
Maze Hiker is free software as described by the GNU General Public
License (v3). See LICENSE.md for details.

## System Requirements
A device that supports 8-bit color (OS 3.5+).

## Building
Maze Hiker was originally developed with [PRC Tools](http://prc-tools.sourceforge.net/), which
unfortunately has not been maintained for modern operating systems. An easy alternative to build
it is to use [prc-tools-remix](https://github.com/jichu4n/prc-tools-remix). The project offers pre-built
binaries for 64-bit Ubuntu/Debian systems and a convenient setup script to install the Palm SDK.

Run `make all` to build a .prc file. Transfer it to a device that meets the
system requirements or the [Palm OS Emulator](https://sourceforge.net/projects/pose/).

Note that the source contains support for high resolution (320x320) Sony Clie
devices that predate PalmOS 5's high resolution API. This has been disabled
since Sony's SDK is no longer publicly available, but if you somehow obtain a
a copy, use `make all -DSONY_CLIE=1` to activate it.
