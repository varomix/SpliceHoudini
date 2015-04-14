VERSION = 0.2.0
DSONAME = OpenSpliceHoudini.${VERSION}.dylib

WIDGET := FabricDFGWidget

SOURCES = \
	plugin.cpp \
	FabricDFGView.cpp \
	MultiParams.cpp \
	FabricDFGOP.cpp \
	SOP_FabricDFG.cpp \
	OBJ_FabricDFG.cpp \
	${WIDGET}.cpp \
	moc_${WIDGET}.cpp \
	CanvasUI.cpp

FABRIC_PATH = ${FABRIC_DIR}
INCDIRS = -I${FABRIC_PATH}/include/
INCDIRS += -I${FABRIC_PATH}/include/FabricServices

LIBDIRS = -L${FABRIC_PATH}/lib
LIBS = -lFabricCore.2.0 -lFabricServices -lFabricSplitSearch -ldl -lpthread

FABRIC_UI_PATH = ${FABRIC_DIR}/../FabricUI/stage
INCDIRS += -I${FABRIC_UI_PATH}/include/FabricUI
INCDIRS += -I${FABRIC_UI_PATH}/include
LIBDIRS += -L${FABRIC_UI_PATH}/lib
LIBS += -lFabricUI

INCDIRS += -I$(HFS)/toolkit/include/OpenEXR/

# C++ Standard Library for C++11 does not work with Houdini and/or Fabric,
# at least on my mac :-/. So we are stucked with c++03.
CXXFLAGS = -std=c++03 -stdlib=libstdc++ -Wno-deprecated -Winit-self
LDFLAGS = -stdlib=libstdc++

QT_DIR=/usr/local/
QT_MOC	:= ${QT_DIR}/bin/moc

OPTIMIZER = -g

include ${HFS}/toolkit/makefiles/Makefile.gnu

# A simple Qt's moc preprocessor rule for our DFG widget 
moc_FabricDFGWidget.cpp: ${WIDGET}.h
	${QT_MOC} ${WIDGET}.h -o moc_${WIDGET}.cpp

clean_all:
	rm -f $(OBJECTS) $(APPNAME) $(DSONAME) moc_FabricDFGWidget.cpp	
