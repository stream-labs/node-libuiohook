# Download libuiohook dependency
mkdir deps
wget --quiet --retry-connrefused --waitretry=1 https://obs-studio-deployment.s3-us-west-2.amazonaws.com/libuiohook-osx-1.1-sl.0.tar.gz
tar -xf ./osx-deps-2018-08-09.tar.gz -C ./deps

mkdir build
cd build

# Configure
cmake .. \
-DUIOHOOKDIR=${PWD}/../deps \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DCMAKE_INSTALL_PREFIX=${FULLDISTRIBUTEPATH}/node-libuiohook
-DNODEJS_NAME=${RUNTIMENAME} \
-DNODEJS_URL=${RUNTIMEURL} \
-DNODEJS_VERSION=${RUNTIMEVERSION}

cd ..

# Build
cmake --build build --target install --config RelWithDebInfo