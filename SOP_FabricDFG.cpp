#include "SOP_FabricDFG.h"

#include <GU/GU_Detail.h>
#include <GEO/GEO_PrimPoly.h>
#include <PRM/PRM_Include.h>
#include <CH/CH_LocalVariable.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <SYS/SYS_Math.h>
#include <limits.h>
#include <GU/GU_PrimPoly.h>
#include <GEO/GEO_PolyCounts.h>

#include <ImathVec.h>

#define FEC_PROVIDE_STL_BINDINGS

using namespace OpenSpliceHoudini;

OP_Node* SOP_FabricDFG::myConstructor(OP_Network* net, const char* name, OP_Operator* op)
{
    return new SOP_FabricDFG(net, name, op);
}

SOP_FabricDFG::SOP_FabricDFG(OP_Network* net, const char* name, OP_Operator* op)
    : FabricDFGOP<SOP_Node>(net, name, op)
{
    getView().setCopyAttributesFunc(SOP_FabricDFG::copyAttributes);
}

SOP_FabricDFG::~SOP_FabricDFG()
{
}

void SOP_FabricDFG::copyAttributes(OP_Network& node, DFGWrapper::Binding& binding)
{
    std::cout << " from SOP_FabricDFG!" << std::endl;
}

OP_ERROR SOP_FabricDFG::cookMySop(OP_Context& context)
{

    // Start the interrupt server
    UT_AutoInterrupt boss("Evaluating FabricDFG");
    if (boss.wasInterrupted())
    {
        return error();
    }

    try
    {
        fpreal now = context.getTime();
        updateGraph(now);
        executeGraph();
        
        gdp->clearAndDestroy();

        FabricServices::DFGWrapper::PortList polyMeshOutputPorts = getView().getPolygonMeshOutputPorts();
        if (polyMeshOutputPorts.size() > 0)
        {
            FabricCore::RTVal rtMesh = getView().getBinding()->getArgValue(polyMeshOutputPorts[0]->getName());

            size_t nbPoints = 0;
            size_t nbPolygons = 0;
            size_t nbSamples = 0;
            if (rtMesh.isValid() && !rtMesh.isNullObject())
            {
                nbPoints = rtMesh.callMethod("UInt64", "pointCount", 0, 0).getUInt64();
                nbPolygons = rtMesh.callMethod("UInt64", "polygonCount", 0, 0).getUInt64();
                nbSamples = rtMesh.callMethod("UInt64", "polygonPointsCount", 0, 0).getUInt64();

                if (nbPoints == 0 || nbPolygons == 0 || nbSamples == 0)
                {
                    return error();
                }

                std::vector<Imath::Vec3<float> > positions(nbPoints);
                {
                    std::vector<FabricCore::RTVal> args(2);
                    args[0] = FabricCore::RTVal::ConstructExternalArray(
                        *(getView().getClient()), "Float32", positions.size() * 3, &positions[0]);
                    args[1] = FabricCore::RTVal::ConstructUInt32(*(getView().getClient()), 3); // components

                    try
                    {
                        rtMesh.callMethod("", "getPointsAsExternalArray", 2, &args[0]);
                    }
                    catch (FabricCore::Exception e)
                    {
                        FabricCore::Exception::Throw(e.getDesc_cstr());
                    }
                }

                std::vector<int> faceCounts(nbPolygons);
                std::vector<int> elementsIndices(nbSamples);
                {
                    std::vector<FabricCore::RTVal> args(2);
                    args[0] = FabricCore::RTVal::ConstructExternalArray(
                        *(getView().getClient()), "UInt32", faceCounts.size(), &faceCounts[0]);
                    args[1] = FabricCore::RTVal::ConstructExternalArray(
                        *(getView().getClient()), "UInt32", elementsIndices.size(), &elementsIndices[0]);

                    try
                    {
                        rtMesh.callMethod("", "getTopologyAsCountsIndicesExternalArrays", 2, &args[0]);
                    }
                    catch (FabricCore::Exception e)
                    {
                        FabricCore::Exception::Throw(e.getDesc_cstr());
                    }
                }

                // Create points
                GA_Offset ptoff = gdp->appendPointBlock(nbPoints);

                // Set points positions
                size_t i = 0;
                GA_FOR_ALL_PTOFF(gdp, ptoff)
                {
                    UT_Vector3 p(positions[i].x, positions[i].y, positions[i].z);
                    gdp->setPos3(ptoff, p);
                    ++i;
                }

                // Build polygons using buildBlock function
                exint startpoint = 0;
                exint nfaces = static_cast<exint>(faceCounts.size());
                GEO_PolyCounts polyCounts;

                for (exint i = 0; i < nfaces; ++i)
                {
                    polyCounts.append(faceCounts[i]);
                }

                GU_PrimPoly::buildBlock(gdp, GA_Offset(startpoint), nbPoints, polyCounts, &elementsIndices[0]);

                select(GU_SPrimitive);
            }
        }
    }
    catch (FabricCore::Exception e)
    {
        printf("FabricCore::Exception from SOP_FabricDFG::cookMySop:\n %s\n", e.getDesc_cstr());
        return error();
    }

    return error();
}
