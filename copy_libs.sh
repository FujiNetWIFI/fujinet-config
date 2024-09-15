#!/bin/bash

# This script will copy the generated FUJINET-LIB zips from a local development directory
# set by FUJI_LIB_DIR, allowing using non-published versions of the libraries

# specify platforms to copy, eventually will include pet, vic20 etc.
PLATFORMS="apple2 apple2enh c64 atari"
FUJI_LIB_DIR=${FUJI_LIB_DIR:-/d/dev/atari/fujinet/fujinet-lib}

ROOT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
#VERSION=$(grep '^FUJINET_LIB_VERSION :=' makefiles/fujinet-lib.mk | awk '{print $3}')
VERSION=4.6.2

#rm -rf ${ROOT_DIR}/fujinet-lib
mkdir ${ROOT_DIR}/fujinet-lib > /dev/null 2>&1

for platform in ${PLATFORMS}; do
  ZIP_FILE=${FUJI_LIB_DIR}/dist/fujinet-lib-${platform}-${VERSION}.zip

  if [ ! -f ${ZIP_FILE} ]; then
    echo "Couldn't find $ZIP_FILE"
    exit 1
  fi

  cp ${ZIP_FILE} ${ROOT_DIR}/fujinet-lib
  rm -rf ${ROOT_DIR}/fujinet-lib/${VERSION}-${platform}
  unzip ${ZIP_FILE} -d ${ROOT_DIR}/fujinet-lib/${VERSION}-${platform}
done
