
#ifndef __SOP_FABRICSDFG_H__
#define __SOP_FABRICSDFG_H__

#include <SOP/SOP_Node.h>
#include "FabricDFGOP.h"

namespace OpenSpliceHoudini
{

class SOP_FabricDFG : public FabricDFGOP<SOP_Node>
{
public:
    static OP_Node* myConstructor(OP_Network*, const char*, OP_Operator*);

protected:
    SOP_FabricDFG(OP_Network* net, const char* name, OP_Operator* op);
    virtual ~SOP_FabricDFG();
    virtual OP_ERROR cookMySop(OP_Context& context);

    static void copyAttributes(OP_Network& node, DFGWrapper::Binding& binding);
};

class OP_FabricDFG : public OP_Operator
{
public:
    OP_FabricDFG()
        : OP_Operator("fabricDFG",                   // Internal name
                      "Fabric Generator",            // UI name
                      SOP_FabricDFG::myConstructor,  // How to build the SOP
                      SOP_FabricDFG::myTemplateList, // My parameters
                      0,                             // Min # of node inputs
                      0,                             // Max # of node inputs
                      0,                             // Local variables
                      OP_FLAG_GENERATOR)             // Flag it as generator
    {
    }
};

} // End namespace OpenSpliceHoudini
#endif // __SOP_FABRICSDFG_H__
