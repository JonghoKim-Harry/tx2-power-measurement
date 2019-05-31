#!/bin/bash
HOME=/home/jongho
HERE=$(realpath $(dirname $0))

# This script is for Ubuntu
sudo add-apt-repository "deb http://archive.ubuntu.com/ubuntu $(lsb_release -sc) main universe restricted multiverse"
sudo apt update
LIST="libboost-dev libboost-all-dev liblapack-dev libblas-dev libatlas-base-dev libatlas-dev libleveldb-dev liblmdb-dev libsnappy-dev libgflags-dev"
for PACKAGE in $LIST; do
    sudo apt install -y $PACKAGE
done

# gflags
git clone https://github.com/gflags/gflags.git $HOME/gflags
cd $HOME/gflags
mkdir build && cd build && cmake -DBUILD_SHARED_LIBS=ON .. && make
sudo make install
cd $HERE

# protobuf
git clone https://github.com/protocolbuffers/protobuf.git $HOME/protobuf
cd $HOME/protobuf
./autogen.sh && ./configure && make && make check
sudo make install
sudo ldconfig
cd $HERE


# hdf5
sudo ln -s /usr/bin/libtoolize /usr/bin/libtool
git clone https://github.com/live-clones/hdf5.git $HOME/hdf5
cd $HOME/hdf5
./autogen.sh && ./configure --prefix=/usr/local && make
##sudo cp -r $HOME/hdf5/ /usr/local
sudo make install
cd $HERE

# To resolve "fatal error: hdf5.h: no such file or directory"
cd /usr/local
sudo ln -s libhdf5_serial.so.8.0.2 libhdf5.so
sudo ln -s libhdf5_serial_hl.so.8.0.2 libhdf5_hl.so
cd $HERE

# glog
git clone https://github.com/google/glog.git $HOME/glog
cd $HOME/glog
./autogen.sh && ./configure && make
sudo make install
cd $HERE
