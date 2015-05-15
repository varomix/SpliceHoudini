# OpenSpliceHoudini

Open source version of Fabric Splice for SideFx Houdini
Only tested on OSX for now (but should work on Linux too).

Here is a short video showing it in action: https://vimeo.com/127362431

For access to the Fabric 2.0 Beta, you’ll want to sign up to this group:
https://groups.google.com/forum/?hl=en#!forum/fabric-engine-beta

# Required applications and libraries

You will need Houdini, Fabric 2.0 and FabricUI

OpenSpliceHoudini.0.4.0 is tested using:
* Houdini 14.0.223
* FabricEngine 2.0 build "FabricEngine-pablo-Darwin-x86_64-20150514-182019"
* FabricUI commit number "d84cc214d9f0cb6cba874be427e06bb8d8c7c2c0"

I'm assuming both FabricEngine-2.0 and FabricUI are copied under a FABRIC_PARENT_DIR directory.
You will need Qt 4.8 to build FabricUI.

# FabricUI add -fPIC flag in the Linux section
env.Append(CXXFLAGS = ['-fPIC'])

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
