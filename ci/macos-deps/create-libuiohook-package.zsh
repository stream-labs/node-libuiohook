#!/bin/zsh

# 1. Put the script to an empty folder
# 2. ./create-libuiohook-package.zsh --architecture=arm64
# 3. ./create-libuiohook-package.zsh --architecture=x86_64

download_libuihook() {
    # Check if the webrtc folder exists
    if [ -d "${GIT_FOLDER}" ]
    then
        echo "### The '${GIT_FOLDER}' folder exists. It will be reused. Remove it if you want to clone the git repositoty again."
        return
    fi

    echo "### Downloading sources ..."

    # Clone
    git clone --recurse-submodules https://github.com/kwhat/libuiohook ${GIT_FOLDER}
    if [ $? -ne 0 ]
    then
        echo "### Could not clone libuihook!"
        exit 1
    fi

    # Checkout correct tag
    cd "${GIT_FOLDER_NAME}"
    git checkout -b branch-for-tag-${GIT_TAG} ${GIT_TAG}
    if [ $? -ne 0 ]
    then
        echo "### Could not checkout the appropriate branch."
        cd "${INITIAL_WORKING_FOLDER}"
        exit 1
    fi

    # Check if the source folder name is correct and the folder exists
    if [ ! -d "${GIT_FOLDER}" ]
    then
        echo "### The source folder '${GIT_FOLDER}' could not be found. Probably the 'git clone' command failed. Please check manually!"
        exit 1
    fi
}

build_libuihook() {

    echo "### Building ..."

    # Prepare build parameters
    case "${ARCHITECTURE}" in
        arm64)
            MIN_MACOS_VERSION=11.0
            ;;
        x86_64)
            MIN_MACOS_VERSION=10.15
            ;;
        *)
            echo "### Unknown architecture! Only 'arm64' or 'x86_64' is supported."
            exit 1
            ;;
    esac

    cd "${GIT_FOLDER}"

    # Check if the build folder exists
    if [ -d "${BUILD_FOLDER}" ]
    then
        echo "### The '${BUILD_FOLDER}' folder exists. Configuring will be skiped. Remove the folder if you want to configure from scratch."
        cd "${BUILD_FOLDER}"
    else
        echo "### Configuring ..."

        mkdir -p "${BUILD_FOLDER}"
        if [ $? -ne 0 ]
        then
            echo "### Could not create the build folder: ${BUILD_FOLDER}!"
            exit 1
        fi
        cd "${BUILD_FOLDER}"
        cmake -S .. -DBUILD_SHARED_LIBS=ON -DCMAKE_OSX_ARCHITECTURES="${ARCHITECTURE}" -DCMAKE_OSX_DEPLOYMENT_TARGET=${MIN_MACOS_VERSION} -DCMAKE_INSTALL_PREFIX=${PACKAGE_FOLDER}
        if [ $? -ne 0 ]
        then
            echo "### Could not configure!"
            cd "${INITIAL_WORKING_FOLDER}"
            exit 1
        fi
    fi    

    echo "### Building ..."

    cmake --build . --target install
    if [ $? -ne 0 ]
    then
        echo "### Build failed!"
        cd "${INITIAL_WORKING_FOLDER}"
        exit 1
    fi
}

package_libuihook() {
    # Copy the script
    echo "### Copying this script ..."
    cp "${SCRIPT_PATH}" "${PACKAGE_FOLDER}" 

    cd ${INITIAL_WORKING_FOLDER}

    # Compress
    echo "### Compressing ..."
    tar cvzf ${PACKAGE_FOLDER_NAME}.tar.gz ${PACKAGE_FOLDER_NAME}    
}

# SCRIPT START

ARCHITECTURE=$(uname -m)

while [ $# -gt 0 ]; do
    case "$1" in
        --architecture=*)
            ARCHITECTURE="${1#*=}"
            ;;
        *)
            echo "### Error: Invalid command line parameter. Please use --architecture=arm64 or --architecture=x86_64"
            exit 1
    esac
    shift
done

INITIAL_WORKING_FOLDER=${PWD}
SCRIPT_PATH=$(realpath $0)
GIT_FOLDER_NAME=libuiohook
GIT_FOLDER=${INITIAL_WORKING_FOLDER}/${GIT_FOLDER_NAME}
GIT_TAG=1.2.2
BUILD_FOLDER_NAME=build-${ARCHITECTURE}
BUILD_FOLDER=${GIT_FOLDER}/${BUILD_FOLDER_NAME}
PACKAGE_FOLDER_NAME=libuiohook-osx-${GIT_TAG}-b$(date +'%y%m%d')-${ARCHITECTURE}
PACKAGE_FOLDER=${INITIAL_WORKING_FOLDER}/${PACKAGE_FOLDER_NAME}

# Check if git is available
if ! command -v git &> /dev/null
then
    echo "'git' could not be found. Please install 'git' and start the script again."
    exit 1
fi

download_libuihook
build_libuihook
package_libuihook
