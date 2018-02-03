#!/bin/bash -e

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     MACHINE=Linux;;
    Darwin*)    MACHINE=Mac;;
    CYGWIN*)    MACHINE=Cygwin;;
    MINGW*)     MACHINE=MinGw;;
    *)          MACHINE="UNKNOWN:${unameOut}"
esac
echo Machine identified as "$MACHINE"

if [[ "$MACHINE" = "Linux" ]]; then
    USER=`whoami`
    if [ $USER = "root" ]
        then
        echo "Running as ROOT"
    else
        echo "***********YOU ARE NOT ROOT************"
        exit 1
    fi
fi

DIR=`pwd`

function finish {
    echo "Cleaning up..."
    rm $DIR/SDL2-2.0.7.tar.gz || true
    rm $DIR/SDL2_mixer-2.0.2.tar.gz || true
    rm $DIR/SDL2_image-2.0.0.tar.gz || true
    rm -rf $DIR/SDL2-2.0.7 || true
    rm -rf $DIR/SDL2_mixer-2.0.2 || true
    rm -rf $DIR/SDL2_image-2.0.0 || true
}

trap finish EXIT

#SDL2.0
wget https://www.libsdl.org/release/SDL2-2.0.7.tar.gz
tar xzf SDL2-2.0.7.tar.gz
cd SDL2-2.0.7
./configure
make -j4
make install -j4
cd $DIR

#SDL2_mixer
wget https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.2.tar.gz
tar xzf SDL2_mixer-2.0.2.tar.gz
cd SDL2_mixer-2.0.2

./configure
make -j4
make install -j4
cd $DIR

#SDL2_image
wget https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.0.tar.gz
tar xzf SDL2_image-2.0.0.tar.gz
cd SDL2_image-2.0.0
./configure
make -j4
make install -j4
cd $DIR

#GLM, ASSIMP, GLEW
if [[ "$MACHINE" = "Linux" ]]; then
    apt-get install -y libglm-dev libglew-dev libassimp-dev
elif [[ "$MACHINE" = "Mac" ]]; then
    brew install glm glew assimp cmake astyle
fi
