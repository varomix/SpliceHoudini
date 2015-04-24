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
#include <boost/lexical_cast.hpp>

#define FEC_PROVIDE_STL_BINDINGS

using namespace OpenSpliceHoudini;

OP_Node* SOP_FabricDFGDeformer::myConstructor(OP_Network* net, const char* name, OP_Operator* op)
{
    return new SOP_FabricDFGDeformer(net, name, op);
}

SOP_FabricDFGDeformer::SOP_FabricDFGDeformer(OP_Network* net, const char* name, OP_Operator* op)
    : FabricDFGOP<SOP_Node>(net, name, op)
{
    mySopFlags.setManagesDataIDs(true);
    s_copyAttributesFunc = SOP_FabricDFGDeformer::OnUpdateGraphCopyAttributes;
}

SOP_FabricDFGDeformer::~SOP_FabricDFGDeformer()
{
}

FabricCore::RTVal SOP_FabricDFGDeformer::ConstructPositionsRTVal(const GU_Detail& gdpRef,
                                                                 SOP_FabricDFGDeformer& sopDeformerNode)
{
    FabricCore::RTVal positions;
    FabricCore::Client client = *(sopDeformerNode.getView().getClient());

    GA_ROHandleV3 handle(gdpRef.findAttribute(GA_ATTRIB_POINT, "P"));
    if (handle.isValid())
    {
        size_t bufferSize = static_cast<size_t>(gdpRef.getNumPoints());
        FabricCore::Client client = *(sopDeformerNode.getView().getClient());
        positions = FabricCore::RTVal::ConstructFixedArray(client, "Vec3", bufferSize);

        // Fill the RTVal array with P values
        GA_Offset ptoff;
        UT_Vector3 val;
        FabricCore::RTVal rtVec[3];
        GA_FOR_ALL_PTOFF(&gdpRef, ptoff)
        {
            val = handle.get(ptoff);
            rtVec[0] = FabricCore::RTVal::ConstructFloat32(client, val.x());
            rtVec[1] = FabricCore::RTVal::ConstructFloat32(client, val.y());
            rtVec[2] = FabricCore::RTVal::ConstructFloat32(client, val.z());
            FabricCore::RTVal rtVal = FabricCore::RTVal::Construct(client, "Vec3", 3, &rtVec[0]);
            positions.setArrayElement(ptoff, rtVal);
        }
    }
    return positions;
}

FabricCore::RTVal SOP_FabricDFGDeformer::ConstructPolygonMeshRTVal(const GU_Detail& gdpRef,
                                                                   SOP_FabricDFGDeformer& sopDeformerNode)
{
    FabricCore::RTVal polygonMesh;
    FabricCore::Client client = *(sopDeformerNode.getView().getClient());

    GA_ROHandleV3 handle(gdpRef.findAttribute(GA_ATTRIB_POINT, "P"));
    if (handle.isValid())
    {
        GA_Size nElements = static_cast<GA_Size>(gdpRef.getNumPoints());
        size_t bufferSize = static_cast<size_t>(nElements);
        std::vector<UT_Vector3F> posBuffer;
        posBuffer.resize(bufferSize);
        handle.getBlock(GA_Offset(), nElements, &posBuffer[0]);

        try
        {
            polygonMesh = FabricCore::RTVal::Construct(client, "PolygonMesh", 0, 0);
            std::vector<FabricCore::RTVal> args(2);
            args[0] = FabricCore::RTVal::ConstructExternalArray(client, "Float32", bufferSize * 3, &posBuffer[0]);
            args[1] = FabricCore::RTVal::ConstructUInt32(client, 3);
            polygonMesh.callMethod("", "setPointsFromExternalArray", 2, &args[0]);
        }
        catch (FabricCore::Exception e)
        {
            FabricCore::Exception::Throw(
                (std::string("[SOP_FabricDFGDeformer::ConstructPolygonMeshRTVal]: ") + e.getDesc_cstr()).c_str());
        }
    }

    return polygonMesh;
}

void SOP_FabricDFGDeformer::OnUpdateGraphCopyAttributes(OP_Network& node, DFGWrapper::Binding& binding)
{
    SOP_FabricDFGDeformer& sopDeformerNode = static_cast<SOP_FabricDFGDeformer&>(node);
    GU_Detail& gdpRef = *(sopDeformerNode.gdp);

    DFGWrapper::PortList ports = binding.getExecutable()->getPorts();
    for (DFGWrapper::PortList::const_iterator it = ports.begin(); it != ports.end(); it++)
    {
        DFGWrapper::PortPtr port = *it;
        if (port->getPortType() == FabricCore::DFGPortType_In)
        {
            std::string name(port->getName());
            std::string resolvedType(port->getResolvedType());

            if (resolvedType == "PolygonMesh")
            {
                FabricCore::RTVal polygonMesh = ConstructPolygonMeshRTVal(gdpRef, sopDeformerNode);

                size_t nbPoints = polygonMesh.callMethod("Size", "pointCount", 0, 0).getUInt32();

                std::cout << "PolygonMesh got: " << nbPoints << " points from Houdini gdp" << std::endl;
            }

            else if (name == "fromP" && resolvedType == "Vec3[]")
            {
                FabricCore::RTVal positions = ConstructPositionsRTVal(gdpRef, sopDeformerNode);

                sopDeformerNode.getView().getBinding()->setArgValue("fromP", positions);
            }
        }
    }
}

void SOP_FabricDFGDeformer::setPointsPositions(OP_Context& context)
{

    // GA_Size nelements = static_cast<GA_Size>(gdp->getNumPoints());
    // std::string resolvedTypeToMatch = "Vec3[" + boost::lexical_cast<std::string>(nelements) + "]";
    std::string resolvedTypeToMatch = "Vec3[]";

    DFGWrapper::PortList ports = getView().getBinding()->getExecutable()->getPorts();
    for (DFGWrapper::PortList::const_iterator it = ports.begin(); it != ports.end(); it++)
    {
        DFGWrapper::PortPtr port = *it;
        if (port->getPortType() == FabricCore::DFGPortType_Out)
        {
            std::string name(port->getName());
            std::string resolvedType(port->getResolvedType());
            if (name == "toP" && resolvedType == resolvedTypeToMatch)
            {
                FabricCore::RTVal rtValVec3Array = getView().getBinding()->getArgValue("toP");

                GA_RWHandleV3 handle(gdp->findAttribute(GA_ATTRIB_POINT, "P"));
                GA_Offset ptoff;
                float pos[3];
                FabricCore::RTVal rtValVec3;
                std::vector<FabricCore::RTVal> args(2);
                GA_FOR_ALL_PTOFF(gdp, ptoff)
                {
                    rtValVec3 = rtValVec3Array.getArrayElement(ptoff);
                    args[0] = FabricCore::RTVal::ConstructExternalArray(*(getView().getClient()), "Float32", 3, &pos);
                    args[1] = FabricCore::RTVal::ConstructUInt32(*(getView().getClient()), 0 /* offset */);
                    rtValVec3.callMethod("", "get", 2, &args[0]);

                    handle.set(ptoff, UT_Vector3(pos[0], pos[1], pos[2]));
                }
                handle.bumpDataId();
            }
        }
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
        updateGraph(now);
        executeGraph();
        setPointsPositions(context);
    }
    catch (FabricCore::Exception e)
    {
        printf("FabricCore::Exception from SOP_FabricDFGDeformer::cookMySop:\n %s\n", e.getDesc_cstr());
        return error();
    }

    return error();
}
