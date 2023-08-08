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

lupdate -recursive ${path_script}/../../woterm ${path_script}/../../kxftp ${path_script}/../../kxvnc ${path_script}/../../kxutil ${path_script}/../../kxver  ${path_script}/../../kxterm -ts ${path_script}/woterm_zh.ts
