brew intstall wget

# Download libuiohook dependency
mkdir deps
export DEPS="libuiohook-osx-1.1-sl.0"
wget --quiet --retry-connrefused --waitretry=1 https://obs-studio-deployment.s3-us-west-2.amazonaws.com/libuiohook-osx-1.1-sl.0.tar.gz
tar -xf ./${DEPS}.tar.gz -C ./deps

mkdir build
cd build

# Configure
cmake .. \
-DUIOHOOKDIR=${PWD}/../deps/${DEPS} \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DCMAKE_INSTALL_PREFIX=${FULLDISTRIBUTEPATH}/node-libuiohook
-DNODEJS_NAME=${RUNTIMENAME} \
-DNODEJS_URL=${RUNTIMEURL} \
-DNODEJS_VERSION=${RUNTIMEVERSION}

cd ..

# Build
cmake --build build --target install --config RelWithDebInfo