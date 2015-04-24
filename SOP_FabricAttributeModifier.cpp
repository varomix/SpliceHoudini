#include "SOP_FabricAttributeModifier.h"

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

static PRM_Name bindPointPositionsButton("bindPointPositions", "Bind Points Positions");
static PRM_Template bindPointPositionsTemplate(
    PRM_CALLBACK, 1, &bindPointPositionsButton, 0, 0, 0, &SOP_FabricAttributeModifier::bindPointPositionsCallback);

int
SOP_FabricAttributeModifier::bindPointPositionsCallback(void* data, int index, float time, const PRM_Template* tplate)
{
    SOP_FabricAttributeModifier* op = reinterpret_cast<SOP_FabricAttributeModifier*>(data);
    op->addExternalArrayGraphPointPositions();
    return 1;
}

OP_TemplatePair* SOP_FabricAttributeModifier::buildTemplatePair(OP_TemplatePair* prevstuff)
{
    static PRM_Template* theTemplate = 0;
    theTemplate = new PRM_Template[2];

    theTemplate[0] = bindPointPositionsTemplate;
    theTemplate[1] = PRM_Template();

    OP_TemplatePair* dfg, *geo;
    dfg = new OP_TemplatePair(myTemplateList, prevstuff);
    geo = new OP_TemplatePair(theTemplate, dfg);

    return geo;
}

OP_Node* SOP_FabricAttributeModifier::myConstructor(OP_Network* net, const char* name, OP_Operator* op)
{
    return new SOP_FabricAttributeModifier(net, name, op);
}

SOP_FabricAttributeModifier::SOP_FabricAttributeModifier(OP_Network* net, const char* name, OP_Operator* op)
    : FabricDFGOP<SOP_Node>(net, name, op)
{
    mySopFlags.setManagesDataIDs(true);
}

SOP_FabricAttributeModifier::~SOP_FabricAttributeModifier()
{
}

void SOP_FabricAttributeModifier::addExternalArrayGraphPointPositions()
{
    // External arrays
    DFGWrapper::GraphExecutablePtr graph = getView().getGraph();
    try
    {
        FabricServices::DFGWrapper::PortPtr port = graph->getPort("P");
        if (std::string(port->getResolvedType()) != "Vec3<>")
        {
            std::cout << "User defined Port P used. Can't access Houdini geometry !" << std::endl;
        }
    }
    catch (FabricCore::Exception e)
    {
        FabricCore::Client client = *(getView().getClient());
        DFGWrapper::Binding binding = *(getView().getBinding());

        graph->addPort("P", FabricCore::DFGPortType_IO);

        DFGWrapper::NodePtr getNode = graph->addNodeFromPreset("Fabric.Core.Array.Get");
        DFGWrapper::NodePtr addNode = graph->addNodeFromPreset("Fabric.Core.Math.Add");
        graph->getPort("P")->connectTo(getNode->getPin("array"));
        getNode->getPin("element")->connectTo(addNode->getPin("lhs"));

        DFGWrapper::NodePtr setNode = graph->addNodeFromPreset("Fabric.Core.Array.Set");
        graph->getPort("P")->connectTo(setNode->getPin("array"));
        addNode->getPin("result")->connectTo(setNode->getPin("element"));

        setNode->getPin("array")->connectTo(graph->getPort("P"));
    }
}

void SOP_FabricAttributeModifier::setExternalArrayPoint(OP_Context& context, const char* name)
{
    GA_RWHandleV3 handle(gdp->findAttribute(GA_ATTRIB_POINT, name));
    if (!handle.isValid())
        return;

    GA_Size nelements = static_cast<GA_Size>(gdp->getNumPoints());
    // FabricCore::RTVal::ConstructExternalArray is expecting a size_t type !
    size_t arraySize = static_cast<size_t>(gdp->getNumPoints());
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

void SOP_FabricAttributeModifier::setPointPositions(OP_Context& context)
{
    GA_RWHandleV3 handle(gdp->findAttribute(GA_ATTRIB_POINT, "P"));
    GA_Size nelements = static_cast<GA_Size>(gdp->getNumPoints());
    handle.setBlock(GA_Offset(), nelements, &m_array[0]);
    handle.bumpDataId();
}

OP_ERROR SOP_FabricAttributeModifier::cookMySop(OP_Context& context)
{
    OP_AutoLockInputs inputs(this);
    if (inputs.lock(context) >= UT_ERROR_ABORT)
        return error();

    // Duplicate input geometry
    duplicateSource(0, context);

    try
    {
        fpreal now = context.getTime();

        updateGraph(now);
        setExternalArrayPoint(context, "P");
        executeGraph();

        if (m_array.size() > 0)
        {
            for (size_t i = 0; i < 3; i++)
                std::cout << "EXTERNAL ARRAY VALUE " << i << ": " << m_array[i].x() << std::endl;

            setPointPositions(context);
        }
    }
    catch (FabricCore::Exception e)
    {
        printf("FabricCore::Exception from SOP_FabricAttributeModifier::cookMySop:\n %s\n", e.getDesc_cstr());
        return error();
    }

    return error();
}
