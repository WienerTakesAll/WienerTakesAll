# WienerTakesAll

## Compiling

### Linux

1. Run `installer.sh` to install runtime libraries.
2. Install Docker
3. Create the docker image using `./docker_build.sh init`
4. Compile the source using `./docker_build.sh all`

### Windows

1. Download all the libraries into the root of the project at http://enochtsang.com:9000/lib.zip.
	If that link doesn't work, try http://198.199.70.199:9000/lib.zip.
2. Unzip the file so that the directory structure is:
	- `WienerTakesAll\lib\include\`
	- `WienerTakesAll\lib\x86\`
	- `WienerTakesAll\lib\dlls\`
3. Optionally you can delete the `lib.zip` folder now.
3. Copy all the DLL files in  `WienerTakesAll\lib\dlls\` into `WienerTakesAll\Debug\`.
4. Copy the `WienerTakesAll\assets` folder into `WienerTakesAll\Debug\`.
4. Open the `WienerTakesAll.sln` solution and build for _Debug x86_.

NOTE: On VS2017 the Windows 8.1 SDK and the UCRT SD must be installed since this is a VS2015 project.
Also the target must be retargeted for the newest platform tools.
To do so, right click the solution file in Visual Studio and click _Retarget Project_.

image::vs2015-2017.png

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
