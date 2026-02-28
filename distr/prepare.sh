#!/bin/bash -e

DIR=$PWD
cd ..
NAME=$(basename "$PWD")
cd $DIR

TARGET=/tmp/${NAME}
SOURCE=./
rm -rf $TARGET
mkdir -p $TARGET/debian
cp ${SOURCE}/debian/ -r $TARGET/
cp -r ../src/ $TARGET/src/
cp  ../*.pro $TARGET/
cp -r ../settings $TARGET/settings
cd $TARGET
echo "==============================================================="
echo $0 PWD: $(pwd)
echo "==============================================================="

PWDFROM=$(pwd)
