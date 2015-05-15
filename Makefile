VERSION = 0.4.0
DSONAME = OpenSpliceHoudini.${VERSION}.dylib

WIDGET := FabricDFGWidget

SOURCES = \
	src/core/FabricDFGView.cpp \
	src/core/MultiParams.cpp \
	src/core/FabricDFGOP.cpp \
	src/core/${WIDGET}.cpp \
	src/core/moc_${WIDGET}.cpp \
	src/core/CanvasUI.cpp \
	src/core/ParameterFactory.cpp \
	src/SOP_FabricGenerator.cpp \
	src/SOP_FabricDeformer.cpp \
	src/OBJ_FabricKinematic.cpp \
	src/plugin.cpp

INCDIRS = -I${FABRIC_DIR}/include/
INCDIRS += -I${FABRIC_DIR}/include/FabricServices

LIBDIRS = -L${FABRIC_DIR}/lib
LIBS = -lFabricCore.2.0 -lFabricServices -lFabricSplitSearch -ldl -lpthread

FABRIC_UI_PATH = ${FABRIC_DIR}/../FabricUI/stage
INCDIRS += -I${FABRIC_UI_PATH}/include/FabricUI
INCDIRS += -I${FABRIC_UI_PATH}/include
LIBDIRS += -L${FABRIC_UI_PATH}/lib
LIBS += -lFabricUI

INCDIRS += -I${HFS}/toolkit/include/OpenEXR/

QT_MOC	:= ${QT_DIR}/bin/moc

OPTIMIZER = -g

include ${HFS}/toolkit/makefiles/Makefile.gnu

# A simple Qt's moc preprocessor rule for our DFG widget 
src/core/moc_FabricDFGWidget.cpp: src/core/${WIDGET}.h
	${QT_MOC} src/core/${WIDGET}.h -o src/core/moc_${WIDGET}.cpp

clean_all:
	rm -f ${OBJECTS} ${APPNAME} ${DSONAME} src/core/moc_FabricDFGWidget.cpp	
