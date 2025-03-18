#!/usr/bin/env bash

# cd into the script's directory
cd "$(dirname "$0")"

SDL_DIR=$(realpath "../lib/SDL")
BUILD_DIR="../build/lib/SDL"
mkdir -p $BUILD_DIR
BUILD_DIR=$(realpath "../build/lib/SDL")
INSTALL_DIR="$BUILD_DIR/install"

cd $BUILD_DIR

# command from libsdl2-dev:i386 source package build log
# libsdl2-dev 2.30.0+dfsg-1build3 (i386 binary) in ubuntu noble
# https://launchpad.net/ubuntu/+source/libsdl2/2.30.0+dfsg-1build3/+build/28030357/+files/buildlog_ubuntu-noble-i386.libsdl2_2.30.0+dfsg-1build3_BUILDING.txt.gz
# also: 32 bit addition with CFLAGS CXXFLAGS and LDFLAGS from https://discourse.libsdl.org/t/building-sdl2-for-32-bit-on-a-64-bit-linux/24331/4
$SDL_DIR/configure --build=i686-linux-gnu "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32" --prefix=$(realpath $INSTALL_DIR) --includedir=\${prefix}/include --mandir=\${prefix}/share/man --infodir=\${prefix}/share/info --sysconfdir=/etc --localstatedir=/var --disable-option-checking --disable-silent-rules --libdir=\${prefix}/lib/i386-linux-gnu --runstatedir=/run --disable-maintainer-mode --disable-dependency-tracking --disable-alsa-shared --disable-arts --disable-directx --disable-esd --disable-fusionsound --disable-jack --disable-joystick-mfi --disable-kmsdrm-shared --disable-libsamplerate-shared --disable-nas --disable-pulseaudio-shared --disable-render-d3d --disable-rpath --disable-video-cocoa --disable-video-directfb --disable-video-metal --disable-video-opengles1 --disable-video-rpi --disable-video-vivante --disable-wasapi --disable-wayland-shared --disable-x11-shared --disable-xinput --enable-alsa --enable-dbus --enable-fcitx --enable-hidapi --enable-hidapi-joystick --enable-ibus --enable-libsamplerate --enable-pulseaudio --enable-sdl2-config --enable-sndio "--enable-vendor-info=Ubuntu 2.30.0+dfsg-1build3" --enable-video-kmsdrm --enable-video-opengl --enable-video-opengles --enable-video-opengles2 --enable-video-x11 --enable-libdecor --enable-libudev --enable-pipewire --enable-video-vulkan --enable-video-wayland ac_cv_header_libunwind_h=no 

# build
BUILD_THREADS=$(($(nproc) + 2))
make -j $BUILD_THREADS

# install
# source: https://wiki.libsdl.org/SDL2/Installation
make install