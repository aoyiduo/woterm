#!/bin/bash
path_current=`pwd`
path_script=$(cd "$(dirname "$0")"; pwd)
mode=$1
bundlenoqt=""

source ${path_script}/../../../init_qt_unix.sh
export PATH=$PATH:${path_script}
export PATH=${QT_TOOLCHAIN}/bin:$PATH
export PATH=$path_script/tools:$PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${QT_TOOLCHAIN}/lib

lrelease ${path_script}/woterm_en.ts -qm ${path_script}/woterm_en.qm
lrelease ${path_script}/woterm_zh.ts -qm ${path_script}/woterm_zh.qm

