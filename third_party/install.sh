#!/bin/sh

sudo apt-get install -y liblua5.4-dev
sudo apt-get -y install lua-dkjson
sudo cp /usr/include/lua5.4/** /usr/include/
cd third_party/luacpp
mkdir -p build
cd build
cmake ../Source -DCMAKE_INSTALL_LIBDIR=/usr/lib -DCMAKE_INSTALL_INCLUDEDIR=/usr/include && make -j$(nproc)
sudo make install && cd ../../..
