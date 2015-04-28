# OpenSpliceHoudini

Open source version of Fabric Splice for SideFx Houdini

# Environment setup:

It is only tested on OSX for the moment but should work on Linux too.

You need Houdini version >= 14.

OpenSpliceHoudini.0.3.0 is tested using:
FabricEngine 2.0 build: FabricEngine-pablo-Darwin-x86_64-20150428-082016
FabricUI commit: 463994cbd31a53c262e00cc1c859e1a62a7e152b

I'm assuming both FabricEngine-2.0 and FabricUI are copied under a FABRIC_PARENT_DIR directory.

Launch Houdini shell to get the Houdini environment (on Mac, ctrl + space > Houdini Terminal)

Then run those lines:
> cd $FABRIC_PARENT_DIR/FabricEngine-2.0.0-beta-Darwin-x86_64
> source environment.sh
> export DYLD_LIBRARY_PATH=FABRIC_PARENT_DIR/FabricEngine-2.0.0-beta-Darwin-x86_64/lib

To build and install plugin:
make install

To clean:
make clean_all
