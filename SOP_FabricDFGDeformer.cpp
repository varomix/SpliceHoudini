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
    mySopFlags.setManagesDataIDs(true);
    getView().setCopyAttributesFunc(SOP_FabricDFGDeformer::OnUpdateGraphCopyAttributes);
}

SOP_FabricDFGDeformer::~SOP_FabricDFGDeformer()
{
}

FabricCore::RTVal SOP_FabricDFGDeformer::CreatePolygonMeshRTVal(const GU_Detail& gdpRef,
                                                                SOP_FabricDFGDeformer& sopDeformerNode)
{
    FabricCore::RTVal polygonMesh;
    FabricCore::Client client = *(sopDeformerNode.getView().getClient());

    GA_ROHandleV3 handle(gdpRef.findAttribute(GA_ATTRIB_POINT, "P"));
    if (handle.isValid())
    {
        GA_Size nElements = static_cast<GA_Size>(gdpRef.getNumPoints());
        size_t bufferSize = static_cast<size_t>(nElements);
        std::vector<UT_Vector3F> posBuffer(bufferSize);
        handle.getBlock(GA_Offset(), nElements, &posBuffer[0]);

        try
        {
            polygonMesh = FabricCore::RTVal::Create(client, "PolygonMesh", 0, 0);
            std::vector<FabricCore::RTVal> args(2);
            args[0] = FabricCore::RTVal::ConstructExternalArray(client, "Float32", bufferSize * 3, &posBuffer[0]);
            args[1] = FabricCore::RTVal::ConstructUInt32(client, 3);
            polygonMesh.callMethod("", "setPointsFromExternalArray", 2, &args[0]);
        }
        catch (FabricCore::Exception e)
        {
            FabricCore::Exception::Throw(
                (std::string("[SOP_FabricDFGDeformer::CreatePolygonMeshRTVal]: ") + e.getDesc_cstr()).c_str());
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
                FabricCore::RTVal polygonMesh = CreatePolygonMeshRTVal(gdpRef, sopDeformerNode);
                sopDeformerNode.getView().getBinding()->setArgValue(port->getName(), polygonMesh);
                break;
            }
        }
    }
}

void SOP_FabricDFGDeformer::setPointsPositions(OP_Context& context)
{
    DFGWrapper::PortList ports = getView().getBinding()->getExecutable()->getPorts();
    for (DFGWrapper::PortList::const_iterator it = ports.begin(); it != ports.end(); it++)
    {
        DFGWrapper::PortPtr port = *it;
        if (port->getPortType() == FabricCore::DFGPortType_Out)
        {
            std::string name(port->getName());
            std::string resolvedType(port->getResolvedType());
            if (resolvedType == "PolygonMesh")
            {
                FabricCore::RTVal polygonMesh = getView().getBinding()->getArgValue(port->getName());
                FabricCore::Client client = *(getView().getClient());

                size_t nbPoints = 0;
                if (polygonMesh.isValid() && !polygonMesh.isNullObject())
                {
                    try
                    {
                        FabricCore::RTVal rtValPointCount;
                        rtValPointCount = polygonMesh.callMethod("Size", "pointCount", 0, 0);
                        nbPoints = rtValPointCount.getUInt32();
                    }
                    catch (FabricCore::Exception e)
                    {
                        (std::string("[SOP_FabricDFGDeformer::setPointsPositions]: ") + e.getDesc_cstr()).c_str();
                    }

                    size_t inPtsCount = static_cast<size_t>(gdp->getNumPoints());
                    if (nbPoints != inPtsCount)
                    {
                        std::cout << "Point Count Mismatch !" << std::endl;
                        break;
                    }

                    std::vector<UT_Vector3F> posBuffer(nbPoints);
                    try
                    {
                        std::vector<FabricCore::RTVal> args(2);
                        args[0] =
                            FabricCore::RTVal::ConstructExternalArray(client, "Float32", nbPoints * 3, &posBuffer[0]);
                        args[1] = FabricCore::RTVal::ConstructUInt32(client, 3); // components

                        polygonMesh.callMethod("", "getPointsAsExternalArray", 2, &args[0]);
                    }
                    catch (FabricCore::Exception e)
                    {
                        (std::string("[SOP_FabricDFGDeformer::setPointsPositions]: ") + e.getDesc_cstr()).c_str();
                    }

                    GA_RWHandleV3 handle(gdp->findAttribute(GA_ATTRIB_POINT, "P"));
                    handle.setBlock(GA_Offset(), gdp->getNumPoints(), &posBuffer[0]);
                    handle.bumpDataId();
                }

                break;
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
