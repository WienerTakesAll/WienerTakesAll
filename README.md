# WienerTakesAll

## Compiling

### Linux

1. Run `installer.sh` to install runtime libraries.
2. Install Docker. If you are on x64 ubuntu xenial package here you can download the package for it here https://download.docker.com/linux/ubuntu/dists/xenial/pool/stable/amd64/docker-ce_17.12.0~ce-0~ubuntu_amd64.deb. Just double click the file after downlodaing and follow the prompts.
3. Create the docker image using `./docker_build.sh init`
4. Compile the source using `./docker_build.sh all`
5. Run `./WienerTakesAll` in the root directory to run the game.

### Mac OSX

1. Install Homebrew using `/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`.
2. Run `installer.sh` to install development libraries.
3. Run `./build.sh all` to compile.
4. Run `./WienerTakesAll` in the root directory to run the game.

### Windows

1. Download all the libraries into the root of the project at http://echtsang.com:9000/lib.zip.
	If that link doesn't work, try http://198.199.70.199:9000/lib.zip.
2. Unzip the file so that the directory structure is:
	- `WienerTakesAll\lib\include\`
	- `WienerTakesAll\lib\x86\`
	- `WienerTakesAll\lib\dlls\`
3. Optionally you can delete the `lib.zip` folder now.
3. Copy all the DLL files in  `WienerTakesAll\lib\dlls\` into `WienerTakesAll\Debug\`.
4. Copy the `WienerTakesAll\assets` folder into `WienerTakesAll\Debug\`.
4. Open the `WienerTakesAll.sln` solution and build for _Debug x86_.

Overall the required folder structure for dependencies (with version implied) is:
```
WienerTakesAll
├── Debug
│   ├── SDL2.dll
│   ├── SDL2_image.dll
│   ├── SDL2_mixer.dll
│   ├── assimp-vc140-mt.dll
│   ├── glew32.dll
│   ├── libjpeg-9.dll
│   ├── libpng16-16.dll
│   ├── libtiff-5.dll
│   ├── libwebp-7.dll
│   └── zlib1.dll
├── README.md
├── WienerTakesAll.sln
├── WienerTakesAll.vcxproj
├── WienerTakesAll.vcxproj.filters
├── assets
├── lib
│   ├── include
│   │   ├── SDL*.h
│   │   ├── GL
│   │   │   └── <glew-headers>
│   │   ├── glm
│   │   │   └── <glm-headers>
│   │   └── assimp
│   │       └── <assimp-headers>
│   └── x86
│       ├── SDL2.lib
│       ├── SDL2_image.lib
│       ├── SDL2_mixer.lib
│       ├── SDL2main.lib
│       ├── SDL2test.lib
│       ├── assimp-vc140-mt.lib
│       ├── glew32.lib
│       └── glew32s.lib
└── src
```