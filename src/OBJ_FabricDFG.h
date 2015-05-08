
#ifndef __OBJ_FABRICSDFG_H__
#define __OBJ_FABRICSDFG_H__

#include <OBJ/OBJ_Geometry.h>
#include "core/FabricDFGOP.h"

namespace OpenSpliceHoudini
{

class OBJ_FabricDFG : public FabricDFGOP<OBJ_Geometry>
{
public:
    static OP_Node* myConstructor(OP_Network*, const char*, OP_Operator*);
    static OP_TemplatePair* buildTemplatePair(OP_TemplatePair* prevstuff);

protected:
    OBJ_FabricDFG(OP_Network* net, const char* name, OP_Operator* op);
    virtual ~OBJ_FabricDFG();
    // // Performs the calculation of the local and the world transformation.
    // virtual OP_ERROR cookMyObj(OP_Context& context);

    virtual int applyInputIndependentTransform(OP_Context& context, UT_DMatrix4& mat);
};

class OP_FabricDFG_OBJ : public OP_Operator
{
public:
    OP_FabricDFG_OBJ()
        : OP_Operator("fabricObject",                      // Internal name
                      "Fabric Object",                     // UI name
                      OBJ_FabricDFG::myConstructor,        // How to build the SOP
                      OBJ_FabricDFG::buildTemplatePair(0), // My parameters
                      0,                                   // Min # of node inputs
                      1,                                   // Max # of node inputs
                      0)                                   // Local variables
    {
    }
};

} // End namespace OpenSpliceHoudini
#endif // __OBJ_FABRICSDFG_H__
