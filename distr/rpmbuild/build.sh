#!/bin/bash -e

CURRENT_DIR=$(pwd)
RPM_ROOT="$CURRENT_DIR/rpm"

rm -rf $RPM_ROOT

# 1. 
mkdir -p "$RPM_ROOT"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# 2. 
cp netrc.spec "$RPM_ROOT/SPECS/"

mkdir -p $RPM_ROOT/SOURCES
cp -r ../../src $RPM_ROOT/SOURCES
cp -r ../../*.pro $RPM_ROOT/SOURCES
cp -r ../../settings $RPM_ROOT/SOURCES
cp -r ../debian/*.desktop $RPM_ROOT/SOURCES
export APP_VERSION_VALUE=26.03
# 3. 
rpmbuild -ba \
  --define "_topdir $RPM_ROOT" \
  --build-in-place \
  netrc.spec

echo "========================================"
echo "Complete. Find your packages in: $RPM_ROOT/RPMS/"