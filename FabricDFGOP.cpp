#include "FabricDFGOP.h"
#include "MultiParams.h"

#include <PRM/PRM_Include.h>

#include <SOP/SOP_Node.h>
#include <OBJ/OBJ_Node.h>
#include <ROP/ROP_Node.h>
#include <VOP/VOP_Node.h>
#include <OBJ/OBJ_Geometry.h>

namespace OpenSpliceHoudini
{

template <typename OP>
FabricDFGOP<OP>::FabricDFGOP(OP_Network* net, const char* name, OP_Operator* op)
    : OP(net, name, op)
    // , m_multiparms(this)
    , m_graphLoaded(false)
    , m_view(this)
    , m_ui(this, m_view)
{
    OP::getParm("__portsChanged").getTemplatePtr()->setInvisible(true);
    OP::getParm("jsonData").getTemplatePtr()->setInvisible(true);
    OP::getParm("currentFrame").setExpression(0, "$F", CH_OLD_EXPR_LANGUAGE, 0);
    OP::getParm("currentFrame").setLockedFlag(0, 1);
    OP::getParm("currentFrame").getTemplatePtr()->setInvisible(true);
}

template <typename OP>
PRM_Template FabricDFGOP<OP>::groupTemplate(
    PRM_STRING, 1, &PRMgroupName, 0, &SOP_Node::pointGroupMenu, 0, 0, SOP_Node::getGroupSelectButton(GA_GROUP_POINT));

template <typename OP>
PRM_Name FabricDFGOP<OP>::jsonFilePath("jsonFilePath", "DFG File");

template <typename OP>
PRM_Default FabricDFGOP<OP>::jsonFilePathDefault(0, "$FABRIC_DIR/Samples/DFG");

template <typename OP>
PRM_Template FabricDFGOP<OP>::jsonFilePathTemplate(PRM_FILE, 1, &jsonFilePath, &jsonFilePathDefault);

template <typename OP>
PRM_Name FabricDFGOP<OP>::jsonData("jsonData", "JSON Data");

template <typename OP>
PRM_Default FabricDFGOP<OP>::jsonDataDefault(0, "");

template <typename OP>
PRM_Template FabricDFGOP<OP>::jsonDataTemplate(PRM_STRING, 1, &jsonData, &jsonDataDefault);

template <typename OP>
PRM_Name FabricDFGOP<OP>::portsChanged("__portsChanged");
template <typename OP>
PRM_Template FabricDFGOP<OP>::portsChangedTemplate(PRM_INT, 1, &portsChanged);

template <typename OP>
PRM_Name FabricDFGOP<OP>::createGraphButton("createGraph", "Create Graph and Connect");
template <typename OP>
PRM_Template FabricDFGOP<OP>::createGraphTemplate(
    PRM_CALLBACK, 1, &createGraphButton, 0, 0, 0, &FabricDFGOP<OP>::createGraphCallback);

template <typename OP>
PRM_Name FabricDFGOP<OP>::openGraphButton("openGraph", "Open Graph");
template <typename OP>
PRM_Template FabricDFGOP<OP>::openGraphButtonTemplate(
    PRM_CALLBACK, 1, &openGraphButton, 0, 0, 0, &FabricDFGOP<OP>::openGraphButtonCallback);

template <typename OP>
PRM_Name FabricDFGOP<OP>::currentFrame("currentFrame", "Current Frame");
template <typename OP>
PRM_Template FabricDFGOP<OP>::currentFrameTemplate(PRM_INT, 1, &currentFrame);

template <typename OP>
int FabricDFGOP<OP>::createGraphCallback(void* data, int index, float time, const PRM_Template* tplate)
{
    FabricDFGOP<OP>* op = reinterpret_cast<FabricDFGOP<OP>*>(data);

    std::ifstream ifs(op->getStringValue("jsonFilePath").buffer(), std::ifstream::in);
    std::stringstream buffer;
    buffer << ifs.rdbuf();

    if (buffer.str() != "")
    {
        // MultiParams::clear(op); // Does not work
        op->setStringValue(UT_String(buffer.str()), "jsonData");

        UT_String jsonData = op->getStringValue("jsonData");

        op->m_view.setFromJSON(jsonData.buffer());

        const FabricDFGView::ParameterPortsNames& intInputs = op->m_view.getInputPortsSInt32Names();
        for (FabricDFGView::ParameterPortsNames::const_iterator it = intInputs.begin(); it != intInputs.end(); it++)
        {
            MultiParams::addIntParameterInst(op, *it, 1);
        }

        const FabricDFGView::ParameterPortsNames& floatInputs = op->m_view.getInputPortsFloat32Names();
        for (FabricDFGView::ParameterPortsNames::const_iterator it = floatInputs.begin(); it != floatInputs.end(); it++)
        {
            MultiParams::addFloatParameterInst(op, *it, 1.0);
        }

        const FabricDFGView::ParameterPortsNames& stringInputs = op->m_view.getInputPortsStringNames();
        for (FabricDFGView::ParameterPortsNames::const_iterator it = stringInputs.begin(); it != stringInputs.end(); it++)
        {
            MultiParams::addStringParameterInst(op, *it);
        }

        const FabricDFGView::ParameterPortsNames& filePathInputs = op->m_view.getInputPortsFilePathNames();
        for (FabricDFGView::ParameterPortsNames::const_iterator it = filePathInputs.begin(); it != filePathInputs.end(); it++)
        {
            MultiParams::addStringParameterInst(op, *it, "FilePath");
        }

        const FabricDFGView::ParameterPortsNames& vec3Inputs = op->m_view.getInputPortsVec3Names();
        for (FabricDFGView::ParameterPortsNames::const_iterator it = vec3Inputs.begin(); it != vec3Inputs.end(); it++)
        {
            MultiParams::addVec3ParameterInst(op, *it, Imath::Vec3<float>(0));
        }        
    }

    return 1;
}

template <typename OP>
int FabricDFGOP<OP>::openGraphButtonCallback(void* data, int index, float time, const PRM_Template* tplate)
{
    FabricDFGOP<OP>* op = reinterpret_cast<FabricDFGOP<OP>*>(data);
    op->m_ui.show();
    return 1;
}

template <typename OP>
PRM_Template FabricDFGOP<OP>::myTemplateList[] = {
    groupTemplate,                        jsonFilePathTemplate,               jsonDataTemplate,
    portsChangedTemplate,                 createGraphTemplate,                openGraphButtonTemplate,
    currentFrameTemplate,

    MultiParams::Float32PortsMultiTemplate, MultiParams::SInt32PortsMultiTemplate, MultiParams::StringPortsMultiTemplate,
    MultiParams::FilePathPortsMultiTemplate, MultiParams::Vec3PortsMultiTemplate,

    PRM_Template()
};

template <typename OP>
UT_String FabricDFGOP<OP>::getStringValue(const char* name, fpreal t) const
{
    UT_String result;
    this->evalString(result, name, 0, 0, t);
    return result;
}

template <typename OP>
void FabricDFGOP<OP>::setStringValue(const UT_String& value, const char* name, fpreal t)
{
    this->setString(value, CH_STRING_LITERAL, name, 0, t);
}

template <typename OP>
void FabricDFGOP<OP>::loadGraph()
{
    if (!m_graphLoaded)
    {
        UT_String jsonData = getStringValue("jsonData");
        getView().setFromJSON(jsonData.buffer());
        // @! Even if loading the graph failed, we set to true.
        // This is because currently, loading graph several time can fail  
        m_graphLoaded = true;
    }
}

template <typename OP>
void FabricDFGOP<OP>::setMultiParameterInputPorts(const fpreal t)
{
    // Set DFG inputs ports from Houdini inputs multi-parameters
    int num_param_instances = this->evalFloat("Float32Ports", 0, t);
    int instance_idx = this->getParm("Float32Ports").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        getView().setFloat32PortValue(MultiParams::getParameterInstFloatName(this, instance_idx),
                                      MultiParams::getParameterInstFloatValue(this, instance_idx, t));

        instance_idx++;
    }

    num_param_instances = this->evalFloat("SInt32Ports", 0, t);
    instance_idx = this->getParm("SInt32Ports").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        getView().setSInt32PortValue(MultiParams::getParameterInstIntName(this, instance_idx),
                                     MultiParams::getParameterInstIntValue(this, instance_idx, t));

        instance_idx++;
    }

    num_param_instances = this->evalFloat("StringPorts", 0, t);
    instance_idx = this->getParm("StringPorts").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        getView().setStringPortValue(MultiParams::getParameterInstStringName(this, instance_idx),
                                     MultiParams::getParameterInstStringValue(this, instance_idx, t));
        instance_idx++;
    }

    num_param_instances = this->evalFloat("FilePathPorts", 0, t);
    instance_idx = this->getParm("FilePathPorts").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        getView().setFilePathPortValue(MultiParams::getParameterInstStringName(this, instance_idx, "FilePath"),
                                       MultiParams::getParameterInstStringValue(this, instance_idx, t, "FilePath"));
        instance_idx++;
    }

    num_param_instances = this->evalFloat("Vec3Ports", 0, t);
    instance_idx = this->getParm("Vec3Ports").getMultiStartOffset();
    for (size_t i = 0; i < num_param_instances; ++i)
    {
        getView().setVec3PortValue(MultiParams::getParameterInstVec3Name(this, instance_idx),
                                       MultiParams::getParameterInstVec3Value(this, instance_idx, t));
        instance_idx++;
    }    
}

// Specializations for Houdini operator using Fabric

template class FabricDFGOP<SOP_Node>;
template class FabricDFGOP<OBJ_Geometry>;
// template class FabricDFGOP<ROP_Node>;
// template class FabricDFGOP<VOP_Node>;

} // End namespace OpenSpliceHoudini
