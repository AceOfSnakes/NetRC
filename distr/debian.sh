TARGET=/tmp/xxx
SOURCE=./
mkdir -p $TARGET
rm -rf $TARGET
mkdir -p $TARGET/debian
cp ${SOURCE}/debian/ -r $TARGET/
cp -r ../src/ $TARGET/src/
cp -r ../settings/ $TARGET/settings/
cp  ../* $TARGET/
cd $TARGET
ln -s /usr/bin/qmake6 ./qmake
pwd
export __FORCED_APP_VER=25.25
dpkg-buildpackage -b -uc -us -d