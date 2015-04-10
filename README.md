# OpenSpliceHoudini

Open source version of Fabric Splice for SideFx Houdini

# Environment setup:

It is only tested on OSX for the moment but should work on Linux too.

You need Houdini version >= 14.
You need FabricEngine-2.0 and FabricUI (assuming both are copied under a FABRIC_PARENT_DIR directory) 
Launch Houdini shell to get the Houdini environment (on Mac, ctrl + space > Houdini Terminal)

Then run those lines:
> cd $FABRIC_PARENT_DIR/FabricEngine-2.0.0-beta-Darwin-x86_64
> source environment.sh
> export DYLD_LIBRARY_PATH=FABRIC_PARENT_DIR/FabricEngine-2.0.0-beta-Darwin-x86_64/lib

To build and install plugin:
make install

To clean:
make clean_all
