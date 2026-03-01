#!/bin/bash 

PWDFROM=.

. ./prepare.sh
echo "--->" $PWDFROM
cd $PWDFROM

echo "==============================================================="
echo $0 PWD: $(pwd)
echo "==============================================================="
export APP_VERSION_VALUE=26.03
dpkg-buildpackage -B -uc -us --hook-done='for f in ../*amd64.deb; do mv "$f" "${f%%_amd64.deb}_$(uname -m).deb"; done'
