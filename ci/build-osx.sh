brew install wget

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
-DCMAKE_INSTALL_PREFIX=${DISTRIBUTEDIRECTORY}/node-libuiohook
-DNODEJS_NAME=${RUNTIMENAME} \
-DNODEJS_URL=${RUNTIMEURL} \
-DNODEJS_VERSION=${RUNTIMEVERSION}

cd ..

# Build
cmake --build build --target install --config RelWithDebInfo

# Configure loading path
sudo install_name_tool -change \
/usr/local/lib/libuiohook.0.dylib \
./node_modules/node-libuiohook/libuiohook.0.dylib \
./build/${DISTRIBUTEDIRECTORY}/node-libuiohook/node_libuiohook.node

#Upload debug files
curl -sL https://sentry.io/get-cli/ | bash
dsymutil $PWD/${BUILDDIRECTORY}/RelWithDebInfo/node_libuiohook.node
sentry-cli --auth-token ${SENTRY_AUTH_TOKEN} upload-dif --org streamlabs-obs --project obs-client $PWD/${BUILDDIRECTORY}/RelWithDebInfo/node_libuiohook.node.dSYM/Contents/Resources/DWARF/node_libuiohook.node.node
