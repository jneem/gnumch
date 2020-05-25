#!/bin/sh
make MainTest.o
g++ -g MainTest.o ../game/FileSys.o ../game/ConfigFile.o libGui.a -lSDL -lSDL_ttf -lSDL_image -lSDL_gfx
