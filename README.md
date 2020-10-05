# Super Mario 64 Port

- This repo contains a full decompilation of Super Mario 64 (J), (U), and (E) with minor exceptions in the audio subsystem.
- Naming and documentation of the source code and data structures are in progress.
- Efforts to decompile the Shindou ROM steadily advance toward a matching build.
- Beyond Nintendo 64, it can also target Linux and Windows natively.

This repo does not include all assets necessary for compiling the game.
A prior copy of the game is required to extract the assets.

## Building for the PS4

### Prerequisites
1. For running this on the PS4 you will first need to extract the OpenGL module libScePigletv2VSH.sprx and the shader compiler module libSceShaccVSH.sprx from RetroArch_PS4_r4.pkg. You can search and find this package online.

  Once extracted, transfer the modules to your PS4 in the location below. Create the subdirs if they are not there.

  `/data/self/system/common/lib/libScePigletVSH.sprx`

  `/data/self/system/common/lib/libSceShaccVSH.sprx`

2. You will also need the Super Mario 64 ROM file baserom.VERSION.z64 where VERSION can be **us**, **jp**, or **eu**.

### Build Instructions

1. Install and setup [OpenOrbis-PS4-Toolchain](https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain/releases) (Version 0.5 for Linux was used at the time of this build)

2. Clone the repo:

  `git clone https://github.com/OsirizX/sm64-port.git -b ps4 --single-branch --recurse-submodules`

  This will create a directory `sm64-port` and then **enter** it `cd sm64-port`.

3. Place baserom.VERSION.z64 into the repository's root directory for asset extraction.

4. Run `make TARGET_PS4=1 pkg` to build the game. Add `-j4` to improve build time.

5. The installable pkg will be located at `build/UP0001-CUSA64001_00-0000000000000001.pkg`

## Note
Savedata is stored at `/data/sm64_save_file.bin`.

## Credits

* The OpenOrbis team for their PS4 toolchain.
* fgsfds for the PS3 port.
* bythos14 for the Vita port.
* flatz for making OpenGL possible on the PS4.
