#include "SOP_FabricDFGDeformer.h"

#include <GU/GU_Detail.h>
#include <GEO/GEO_PrimPoly.h>
#include <PRM/PRM_Include.h>
#include <CH/CH_LocalVariable.h>
#include <UT/UT_Interrupt.h>
#include <SYS/SYS_Math.h>
#include <limits.h>
#include <GU/GU_PrimPoly.h>
#include <GEO/GEO_PolyCounts.h>
#include <OP/OP_AutoLockInputs.h>

#include <ImathVec.h>

#define FEC_PROVIDE_STL_BINDINGS

using namespace OpenSpliceHoudini;

OP_Node* SOP_FabricDFGDeformer::myConstructor(OP_Network* net, const char* name, OP_Operator* op)
{
    return new SOP_FabricDFGDeformer(net, name, op);
}

SOP_FabricDFGDeformer::SOP_FabricDFGDeformer(OP_Network* net, const char* name, OP_Operator* op)
    : FabricDFGOP<SOP_Node>(net, name, op)
{
    addExternalArrayGraph();
}

SOP_FabricDFGDeformer::~SOP_FabricDFGDeformer()
{
}

void SOP_FabricDFGDeformer::addExternalArrayGraph()
{
    // External arrays
    DFGWrapper::GraphExecutablePtr graph = getView().getGraph();
    try
    {
        FabricServices::DFGWrapper::PortPtr port = graph->getPort("P");
        if (std::string(port->getResolvedType()) != "Float32<>")
        {
            std::cout << "User defined Port P used. Can't access Houdini geometry !" << std::endl;
        }
    }
    catch (FabricCore::Exception e)
    {
        FabricCore::Client client = *(getView().getClient());
        DFGWrapper::Binding binding = *(getView().getBinding());

        graph->addPort("P", FabricCore::DFGPortType_IO);

        // graph->addPort("factor", FabricCore::DFGPortType_In);
        // FabricCore::RTVal value = FabricCore::RTVal::ConstructFloat32(client, 2.3);
        // binding.setArgValue("factor", value);


        DFGWrapper::NodePtr getNode = graph->addNodeFromPreset("Fabric.Core.Array.Get");
        DFGWrapper::NodePtr addNode = graph->addNodeFromPreset("Fabric.Core.Math.Add");
        graph->getPort("P")->connectTo(getNode->getPin("array"));
        getNode->getPin("element")->connectTo(addNode->getPin("lhs"));

        // graph->getPort("factor")->connectTo(addNode->getPin("rhs"));

        DFGWrapper::NodePtr setNode = graph->addNodeFromPreset("Fabric.Core.Array.Set");
        graph->getPort("P")->connectTo(setNode->getPin("array"));
        addNode->getPin("result")->connectTo(setNode->getPin("element"));

        setNode->getPin("array")->connectTo(graph->getPort("P"));
    }
}

void SOP_FabricDFGDeformer::setExternalArrayPoint(OP_Context& context, const char* name)
{
    GA_RWHandleV3 handle(gdp->findAttribute(GA_ATTRIB_POINT, name));
    if(!handle.isValid())
        return;
    
    GA_Size nelements = static_cast<GA_Size>(gdp->getNumPoints());
    size_t arraySize =  static_cast<size_t>(gdp->getNumPoints());
    m_array.resize(arraySize);
    handle.getBlock(GA_Offset(), nelements, &m_array[0]);


    FabricCore::Client client = *(getView().getClient());
    DFGWrapper::Binding binding = *(getView().getBinding());
    FabricCore::RTVal extArrayValue;
    try
    {
        extArrayValue = FabricCore::RTVal::ConstructExternalArray(client, "Vec3", arraySize, &m_array[0]);
        binding.setArgValue(name, extArrayValue);
    }
    catch (FabricCore::Exception e)
    {
        FabricCore::Exception::Throw("extArrayValue not contructed");
    }
}

OP_ERROR SOP_FabricDFGDeformer::cookMySop(OP_Context& context)
{
    OP_AutoLockInputs inputs(this);
    if (inputs.lock(context) >= UT_ERROR_ABORT)
        return error();

    // Duplicate input geometry
    duplicateSource(0, context);

    try
    {
        fpreal now = context.getTime();

        setExternalArrayPoint(context, "P");
        updateGraph(now);
        executeGraph(); 

        if(m_array.size() > 0)
        {
            for(size_t i=0; i< 3; i++)       
                std::cout << "EXTERNAL ARRAY VALUE " << i << ": " << m_array[i].x() << std::endl;
            
        }

    }
    catch (FabricCore::Exception e)
    {
        printf("FabricCore::Exception from SOP_FabricDFGDeformer::cookMySop:\n %s\n", e.getDesc_cstr());
        return error();
    }

    return error();
}
