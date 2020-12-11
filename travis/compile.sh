#!/bin/sh
set -exv
phpize && ./configure && make clean && make