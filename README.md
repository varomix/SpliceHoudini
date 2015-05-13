# OpenSpliceHoudini

Open source version of Fabric Splice for SideFx Houdini
Only tested on OSX for now (but should work on Linux too).

For access to the Fabric 2.0 Beta, you’ll want to sign up to this group:
https://groups.google.com/forum/?hl=en#!forum/fabric-engine-beta

# Required applications and libraries

You will need Houdini, Fabric 2.0 and FabricUI

OpenSpliceHoudini.0.4.0 is tested using:
* Houdini 14.0.223
* FabricEngine 2.0 build "FabricEngine-pablo-Darwin-x86_64-20150513-150126"
* FabricUI commit number "91e27ea1f8124f1b95bd74cab7defa099dadddd1"

I'm assuming both FabricEngine-2.0 and FabricUI are copied under a FABRIC_PARENT_DIR directory.
You will need Qt 4.8 to build FabricUI.

# TEMPORARY

You will need to add additional DCC methods from SpliceHoudini/Ext/SpliceHoudiniUtils.kl into your FabricEngine-2.0/Exts/Builtin/Geometry/PolygonMesh/PolygonMeshDCCConversion.kl.
This is a very temporary thing until similar methods are distributed with Fabric 2.0!

# Environment setup

Launch Houdini shell to get the Houdini environment (on Mac, ctrl + space > Houdini Terminal)

Then run those lines:
> cd $FABRIC_PARENT_DIR/FabricEngine-2.0.0-beta-Darwin-x86_64
> source environment.sh
> export DYLD_LIBRARY_PATH=$FABRIC_PARENT_DIR/FabricEngine-2.0.0-beta-Darwin-x86_64/lib

To build and install plugin:
> make install

To clean:
> make clean_all

# Tests

First add the SpliceHoudini DFG_Presets to your FABRIC_DFG_PATH:  
export FABRIC_DFG_PATH=$SPLICE_HOUDINI_DIR/DFG_Presets:$FABRIC_DFG_PATH
Then from your SpliceHoudini directory run: 
> cd test; hython loadScenes.py ; cd -

Once tests are running well, you can start playing in Houdini !
