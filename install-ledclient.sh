#!/usr/bin/env bash

#   Copyright (c) 2020 AnimatedLEDStrip
#
#   Permission is hereby granted, free of charge, to any person obtaining a copy
#   of this software and associated documentation files (the "Software"), to deal
#   in the Software without restriction, including without limitation the rights
#   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#   copies of the Software, and to permit persons to whom the Software is
#   furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included in
#   all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#   THE SOFTWARE.


if [ "$EUID" -ne 0 ]
then
  echo "\e[31mPlease run as root\e[0m"
  exit 1
fi

echo -n "Checking for cmake..."

if ! command -v cmake &>/dev/null
then
  echo -e "\e[31mnot found\e[0m"
  echo "Please install cmake"
  exit 1
else
  echo -e "\e[32mfound\e[0m"
fi

echo -n "Creating /usr/local/leds..."

if [[ -d /usr/local/leds ]]
then
  echo -e "\e[32mexists\e[0m"
else
  install -d /usr/local/leds
  echo -e "\e[32mdone\e[0m"
fi


echo "Cloning ledclient..."

rm -rf /tmp/ledclient-build

git clone https://github.com/AnimatedLEDStrip/ledclient.git /tmp/ledclient-build

mkdir /tmp/ledclient-build/build

echo -e "\e[2mCloning ledclient...\e[22;32mdone\e[0m"

echo "Building ledclient..."

cd /tmp/ledclient-build/build

cmake .. || exit

make || exit

echo -e "\e[2mBuilding ledclient...\e[22;32mdone\e[0m"

echo -n "Installing ledclient..."

install -m 755 ledclient /usr/local/leds/ledclient

ln -f -s /usr/local/leds/ledclient /usr/bin/ledclient

echo -e "\e[32mdone\e[0m"

rm -rf /tmp/ledclient-build

echo "AnimatedLEDStrip ledclient program installed successfully"
