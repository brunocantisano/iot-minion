#!/bin/bash

arduino-cli compile --build-path $PWD/buildProdArduino --fqbn $1 $2 --output-dir $3