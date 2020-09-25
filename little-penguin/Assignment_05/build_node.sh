#!/bin/sh

export MAJOR=$(grep fortytwo </proc/devices | cut -d ' ' -f1)

mknod /dev/fortytwo c $MAJOR 0
