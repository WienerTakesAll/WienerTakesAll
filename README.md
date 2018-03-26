# WienerTakesAll

## Controls

All controls can be controlled by an Xbox controller.

- Right trigger to go forward
- Left trigger to brake
- Left stick to turn
- Left shoulder button to use powerups on yourself
- Right shoulder button to use powerups on others

Alternatively, they can also be controlled by keyboard controls as well.

 - Player 1 WASD; Q to powerup self, E to use powerups on others
 - Player 2 TFGH; R self, Y others
 - Player 3 IJKL; U self, O others
 - Player 4 Arrow keys; RSHIFT self, RETURN others

Menu selection on the keyboard is down with the enter key.
Escape will end the game early.
To toggle full-screen mode, press F11.

## Powerups

There are currently three powerups implemented:

- Ketchup: Give yourself a handy boost, or cause others to go speeding out of control!
- Mustard: Hop over other players, or make them hop over you!
- Relish: Mess around with your enemies' steering, but take care not to mess yourself up!

## Known issues

There are a number of issues that we are aware of and are making efforts to address:

- On a Windows machine, the physics may take some time to load after starting the game from the menu.
- The physics simulation lags sometimes when certain conditions are met
- Resolution differences between windowed and fullscreen may stretch some textures
- For Xbox controllers, the controller number shown may not match the player number in-game

## Compiling

### Linux

1. Run `installer.sh` to install runtime libraries.
2. Install Docker
3. Create the docker image using `./docker_build.sh init`
4. Compile the source using `./docker_build.sh all`

### Mac OS X

1. Run `installer.sh` to install libraries
2. Build using `./build.sh all`

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

Overall the required folder structure for dependencies (with version implied) is:
```
WienerTakesAll
├── Debug
│   ├── assimp-vc140-mt.dll
│   ├── glew32.dll
│   ├── glut32.dll
│   ├── libjpeg-9.dll
│   ├── libpng16-16.dll
│   ├── libtiff-5.dll
│   ├── libwebp-7.dll
│   ├── nvToolsExt32_1.dll
│   ├── PhysX3CharacterKinematicDEBUG_x86.dll
│   ├── PhysX3CommonDEBUG_x86.dll
│   ├── PhysX3CookingDEBUG_x86.dll
│   ├── PhysX3DEBUG_x86.dll
│   ├── PhysX3Gpu_x86.dll
│   ├── PhysX3GpuCHECKED_x86.dll
│   ├── PhysX3GpuDEBUG_x86.dll
│   ├── PhysX3GpuPROFILE_x86.dll
│   ├── PhysXDevice.dll
│   ├── PxFoundationDEBUG_x86.dll
│   ├── PxPvdSDKDEBUG_x86.dll
│   ├── SDL2.dll
│   ├── SDL2_image.dll
│   ├── SDL2_mixer.dll
│   └── zlib1.dll
├── README.md
├── WienerTakesAll.sln
├── WienerTakesAll.vcxproj
├── WienerTakesAll.vcxproj.filters
├── assets
├── lib
│   ├── include
│   │   ├── Px*.h
│   │   ├── <physx headers directories>
│   │   ├── SDL*.h
│   │   ├── GL
│   │   │   └── <glew-headers>
│   │   ├── glm
│   │   │   └── <glm-headers>
│   │   └── assimp
│   │       └── <assimp-headers>
│   └── x86
|       ├── assimp-vc140-mt.lib
|       ├── glew32.lib
|       ├── glew32s.lib
|       ├── LowLevelAABBDEBUG.lib
|       ├── LowLevelClothDEBUG.lib
|       ├── LowLevelDEBUG.lib
|       ├── LowLevelDynamicsDEBUG.lib
|       ├── LowLevelParticlesDEBUG.lib
|       ├── PhysX3CharacterKinematicDEBUG_x86.lib
|       ├── PhysX3CommonDEBUG_x86.lib
|       ├── PhysX3CookingDEBUG_x86.lib
|       ├── PhysX3DEBUG_x86.lib
|       ├── PhysX3ExtensionsDEBUG.lib
|       ├── PhysX3VehicleDEBUG.lib
|       ├── PsFastXmlDEBUG_x86.lib
|       ├── PxFoundationDEBUG_x86.lib
|       ├── PxPvdSDKDEBUG_x86.lib
|       ├── PxTaskDEBUG_x86.lib
|       ├── SceneQueryDEBUG.lib
|       ├── SDL2.lib
|       ├── SDL2_image.lib
|       ├── SDL2_mixer.lib
|       ├── SDL2main.lib
|       ├── SDL2test.lib
|       └── SimulationControllerDEBUG.lib
└── src
```
