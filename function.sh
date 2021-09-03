#!/bin/bash
curdir=$(pwd)
builddir=${curdir}/build
if [ ! -d $builddir ]; then
  mkdir $builddir
fi

pv=$(python3 -c 'import sys; print(sys.version_info[0])')
pip_cmd=pip
if [ "$pv" == '3' ]; then
  pip_cmd=pip3
fi

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
PINK='\033[0;35m'
NC='\033[0m' # No Color

function run_compile() {
    echo -e "${GREEN}Start compiling.${NC}"
    cd ${builddir}
    cmake .. -DCMAKE_INSTALL_PREFIX=.install -DCMAKE_PREFIX_PATH=${builddir}  \
        -DTHREAD_COUNT=${thread_count} \
        -DSSL_TYPE=${ssl_type} \
        -DUSE_ALONE=${use_alone} \
        -DCMAKE_BUILD_TYPE=${build_type} \
        -DSERVER_TYPE=${server_type} \
        -DUSE_BUFFER=${use_buffer} \
        -DUSE_CACHE=${use_cache} -DMULTI_LOCKS=${multi_locks} \
        -DCLIENT_TYPE=${client_type}
        # -DPYBIND11_PYTHON_VERSION=${python_version}

    if [ -z ${verbose} ];then
        make -j8
    else
        make -j8 VERBOSE=${verbose}
    fi

    cd ${curdir}
    echo -e "${GREEN}Compile executed successfully.${NC}"
}

function run_compile_proto() {
    echo -e "${GREEN}start compile proto file.${NC}"
    proto_file_dir=./protos
    core_dir=./core/src/
    include_dir=./core/include/

    protoc -I protos --grpc_out=$proto_file_dir --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` \
      $proto_file_dir/*.proto
    protoc -I protos --cpp_out=$proto_file_dir $proto_file_dir/*.proto

    mv $proto_file_dir/*.cc $core_dir
    mv $proto_file_dir/*.h $include_dir
    echo -e "${GREEN}compile proto file successfully.${NC}"
}

function run_clean() {
    echo -e "${GREEN}Start cleaning.${NC}"
    ./clean.sh
    echo -e "${GREEN}Cleaned up successfully.${NC}"
}

function run_compile_python() {
  echo -e "stage run_compile_python."
  cd ${curdir}
  python_cmd=python${3}
  package_name="channel_sdk"

  echo -e "python_cmd: ${python_cmd}"

  if [ "$USER" == "root" ]; then
    ${pip_cmd} uninstall $package_name -y
  else
    ${python_cmd} -m pip uninstall $package_name -y # for the current user
  fi

  export USE_ALONE=$1 SSL_TYPE=$2 && ${python_cmd} setup.py build_ext
  export USE_ALONE=$1 SSL_TYPE=$2 && ${python_cmd} setup.py bdist_wheel

  cd ${curdir}
  echo -e "${GREEN}run stage run_compile_python ok.${NC}"
}

function run_install_python() {
    echo -e "${GREEN}Start install.${NC}"
    cd ${curdir}

    if [ "$USER" == "root" ]; then
        ${pip_cmd} install dist/*.whl
    else
        ${pip_cmd} install dist/*.whl --user
    fi

    cd ${curdir}
    echo -e "${GREEN}Install successfully.${NC}"
}