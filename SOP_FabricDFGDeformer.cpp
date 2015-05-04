#include "SOP_FabricDFGDeformer.h"
#include "AttributeTraits.h"

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
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#define FEC_PROVIDE_STL_BINDINGS

using namespace OpenSpliceHoudini;

static PRM_Name pointAttributesName("pointAttributes", "Points Attributes");
static PRM_Template pointAttributesTemplate(PRM_STRING, 1, &pointAttributesName);

namespace
{

std::vector<std::string> getAttributeNameTokens(SOP_FabricDFGDeformer& sopDeformerNode)
{
    std::string paramVal = sopDeformerNode.getStringValue("pointAttributes", 0).toStdString();
    std::vector<std::string> result;
    boost::split(result, paramVal, boost::is_any_of(",; "));
    return result;
}

} // end unnamed namespace

OP_TemplatePair* SOP_FabricDFGDeformer::buildTemplatePair(OP_TemplatePair* prevstuff)
{
    static PRM_Template* theTemplate = 0;
    theTemplate = new PRM_Template[2];

    theTemplate[0] = pointAttributesTemplate;
    theTemplate[1] = PRM_Template();

    OP_TemplatePair* dfg, *geo;
    dfg = new OP_TemplatePair(myTemplateList, prevstuff);
    geo = new OP_TemplatePair(theTemplate, dfg);

    return geo;
}

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
    FabricCore::Client client = *(sopDeformerNode.getView().getClient());
    FabricCore::RTVal polygonMesh = FabricCore::RTVal::Create(client, "PolygonMesh", 0, 0);

    // Setting P attribute is required before adding other point attributes
    GA_ROHandleV3 handle(gdpRef.findAttribute(GA_ATTRIB_POINT, "P"));
    if (!handle.isValid())
        return polygonMesh;

    size_t bufferSize = static_cast<size_t>(gdpRef.getNumPoints());
    std::vector<UT_Vector3F> posBuffer(bufferSize);
    handle.getBlock(GA_Offset(), gdpRef.getNumPoints(), &posBuffer[0]);

    try
    {
        std::vector<FabricCore::RTVal> args(1);
        args[0] = FabricCore::RTVal::ConstructExternalArray(client, "Vec3", bufferSize, &posBuffer[0]);
        polygonMesh.callMethod("", "setPointPositionFromHoudiniArray", 1, &args[0]);
    }
    catch (FabricCore::Exception e)
    {
        FabricCore::Exception::Throw(
            (std::string("[SOP_FabricDFGDeformer::CreatePolygonMeshRTVal]: ") + e.getDesc_cstr()).c_str());
    }

    // Get other per-point attributes
    BOOST_FOREACH (const std::string& attrName, getAttributeNameTokens(sopDeformerNode))
    {
        if (attrName == "P")
            continue;

        const GA_Attribute* attrib = gdpRef.findAttribute(GA_ATTRIB_POINT, attrName.c_str());
        if(attrib == 0)
            continue;

        if (attrib->getTupleSize() == 1)
        {
            if (attrib->getStorageClass() == GA_STORECLASS_INT)
            {
                HouToFabAttributeTraits<int32>::setAttribute(GA_ROHandleI(attrib),
                                                             attrib->getTypeInfo(),
                                                             gdpRef.getNumPoints(),
                                                             client,
                                                             polygonMesh,
                                                             attrName.c_str());
            }
            if (attrib->getStorageClass() == GA_STORECLASS_REAL)
            {
                HouToFabAttributeTraits<fpreal32>::setAttribute(GA_ROHandleF(attrib),
                                                                attrib->getTypeInfo(),
                                                                gdpRef.getNumPoints(),
                                                                client,
                                                                polygonMesh,
                                                                attrName.c_str());
            }
        }
        else if (attrib->getTupleSize() == 3)
        {
            HouToFabAttributeTraits<UT_Vector3F>::setAttribute(GA_ROHandleV3(attrib),
                                                               attrib->getTypeInfo(),
                                                               gdpRef.getNumPoints(),
                                                               client,
                                                               polygonMesh,
                                                               attrName.c_str());
        }
        else if (attrib->getTupleSize() == 4)
        {
            HouToFabAttributeTraits<UT_Vector4F>::setAttribute(GA_ROHandleV4(attrib),
                                                               attrib->getTypeInfo(),
                                                               gdpRef.getNumPoints(),
                                                               client,
                                                               polygonMesh,
                                                               attrName.c_str());
        }
    }

    return polygonMesh;
}

void SOP_FabricDFGDeformer::OnUpdateGraphCopyAttributes(OP_Network& node, DFGWrapper::Binding& binding)
{
    SOP_FabricDFGDeformer& sopDeformerNode = static_cast<SOP_FabricDFGDeformer&>(node);
    GU_Detail& gdpRef = *(sopDeformerNode.gdp);

    DFGWrapper::PortList ports = binding.getExecutable()->getPorts();
    BOOST_FOREACH (const DFGWrapper::PortPtr& port, ports)
    {
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
                        std::cout << "Point Count Mismatch ! gdp is: " << inPtsCount << " and output port '"
                                  << port->getName() << "' is: " << nbPoints << std::endl;
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
        std::string msg = "FabricCore::Exception from SOP_FabricDFGDeformer::cookMySop:\n";
        msg += e.getDesc_cstr();
        std::cerr << msg << std::endl;
        addError(SOP_MESSAGE, msg.c_str());
        return error();
    }

    if (!getView().hasOuputPort())
    {
        addWarning(SOP_MESSAGE, "No Canvas output ports, Fabric Graph will not be executed");
    }

    return error();
}
