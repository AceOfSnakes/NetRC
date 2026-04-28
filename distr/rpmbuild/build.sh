#!/bin/bash -e

CURRENT_DIR=$(pwd)

cd ../..
NAME=$(basename "$PWD")
DIR=$PWD
RPM_ROOT=/tmp/$NAME/rpm

cd $CURRENT_DIR
# 1. 
echo ========================= $RPM_ROOT
rm -rf $RPM_ROOT
mkdir -p "$RPM_ROOT"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# 2. 

cp netrc.spec "$RPM_ROOT/.."

cp -r ../../src $RPM_ROOT/SOURCES
cp -r ../../*.pro $RPM_ROOT/SOURCES
cp -r ../../settings $RPM_ROOT/SOURCES
cp -r ../../src/commons/style $RPM_ROOT/SOURCES
cp -r ../debian/*.desktop $RPM_ROOT/SOURCES
export APP_VERSION_VALUE=26.04


cd "$RPM_ROOT/.."

# 3. 
rpmbuild -ba \
  --define "_topdir $RPM_ROOT" \
  --build-in-place \
  --define "debug_package %{nil}" \
  netrc.spec

echo "========================================"
echo "Complete. Find your packages in: $RPM_ROOT/RPMS/"
cp $RPM_ROOT/RPMS/*.rpm $RPM_ROOT/../../