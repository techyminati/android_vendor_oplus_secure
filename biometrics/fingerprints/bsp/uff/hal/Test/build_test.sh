#!/bin/bash

prebuilt_usage() {
cat << EOF
*******************************prebuilt_usage*****************************
Get the build from gerrit:
    git clone ssh://yourname@gerrit.scm.adc.com:29418/oplus/secure/build
method for setting prebuilt:
    1 you can set bash environment: export PREBUILT_ROOT=your_prebult_dir
**************************************************************************
EOF
}

echo -e "\ncheck prebuilt"
if [ x${PREBUILT_ROOT} == x ] ; then
    prebuilt_usage
    exit 1
fi

NDK_VERSION=android-ndk-r21e
NDK_ROOT=${PREBUILT_ROOT}/ree/${NDK_VERSION}

$NDK_ROOT/ndk-build -B \
    V=1 \
    NDK_PROJECT_PATH=. \
    NDK_OUT=out \
    NDK_LIBS_OUT=out \
    NDK_APPLICATION_MK=Application.mk

if [ -d ~/out ] ; then
    cp -v out/arm64-v8a/fptest ~/out/
fi
