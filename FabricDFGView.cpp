#include "FabricDFGView.h"
#include "MultiParams.h"

#include <OP/OP_Node.h>

using namespace FabricServices;

namespace OpenSpliceHoudini
{

FabricCore::Client FabricDFGView::s_client;
DFGWrapper::Host* FabricDFGView::s_host = NULL;
FabricServices::ASTWrapper::KLASTManager* FabricDFGView::s_manager = NULL;
FabricServices::Commands::CommandStack FabricDFGView::s_stack;
unsigned int FabricDFGView::s_maxId = 0;
void (*FabricDFGView::s_logFunc)(void*, const char*, unsigned int) = NULL;
void (*FabricDFGView::s_logErrorFunc)(void*, const char*, unsigned int) = NULL;

std::map<unsigned int, FabricDFGView*> FabricDFGView::s_instances;

FabricDFGView::FabricDFGView(OP_Node* op)
    : m_op(op)
{
    m_id = s_maxId++;

    // construct the client
    if (s_instances.size() == 0)
    {
        try
        {
            // create a client
            FabricCore::Client::CreateOptions options;
            memset(&options, 0, sizeof(options));
            options.optimizationType = FabricCore::ClientOptimizationType_Background;
            s_client = FabricCore::Client(&logFunc, NULL, &options);

            // Load basic extensions
            s_client.loadExtension("Math", "", false);
            s_client.loadExtension("Geometry", "", false);
            s_client.loadExtension("FileIO", "", false);

            // create a host for Canvas
            s_host = new DFGWrapper::Host(s_client);

            // create KL AST manager
            s_manager = new ASTWrapper::KLASTManager(&s_client);

        }
        catch (FabricCore::Exception e)
        {
            printf("Error: %s\n", e.getDesc_cstr());
        }
    }

    s_instances.insert(std::pair<unsigned int, FabricDFGView*>(m_id, this));
    
    // create an empty binding
    m_binding = s_host->createBindingToNewGraph();

    // set the graph on the view
    setGraph(m_binding.getGraph());

    m_parameterPortsMap["SInt32"] = &m_inSInt32Names;
    m_parameterPortsMap["Float32"] = &m_inFloat32Names;
    m_parameterPortsMap["String"] = &m_inStringNames;
    m_parameterPortsMap["FilePath"] = &m_inFilePathNames;
    m_parameterPortsMap["Vec3"] = &m_inVec3Names;
}

FabricDFGView::~FabricDFGView()
{
    std::map<unsigned int, FabricDFGView*>::iterator it = s_instances.find(m_id);

    m_binding = DFGWrapper::Binding();

    if (it != s_instances.end())
    {
        s_instances.erase(it);
        if (s_instances.size() == 0)
        {
            try
            {
                s_stack.clear();
                delete (s_manager);
                delete (s_host);
                s_client = FabricCore::Client();
            }
            catch (FabricCore::Exception e)
            {
                printf("Error: %s\n", e.getDesc_cstr());
            }
        }
    }
}

unsigned int FabricDFGView::getId()
{
    return m_id;
}

FabricDFGView* FabricDFGView::getFromId(unsigned int id)
{
    std::map<unsigned int, FabricDFGView*>::iterator it = s_instances.find(id);
    if (it == s_instances.end())
        return NULL;
    return it->second;
}

FabricCore::Client* FabricDFGView::getClient()
{
    return &s_client;
}

FabricServices::DFGWrapper::Host* FabricDFGView::getHost()
{
    return s_host;
}

FabricServices::DFGWrapper::Binding* FabricDFGView::getBinding()
{
    return &m_binding;
}

FabricServices::ASTWrapper::KLASTManager* FabricDFGView::getManager()
{
    return s_manager;
}

FabricServices::Commands::CommandStack* FabricDFGView::getStack()
{
    return &s_stack;
}

std::string FabricDFGView::getJSON()
{
    try
    {
        return m_binding.getGraph().exportJSON();
    }
    catch (FabricCore::Exception e)
    {
        printf("Error: %s\n", e.getDesc_cstr());
    }
    return "";
}

bool FabricDFGView::setFromJSON(const std::string& json)
{
    try
    {
        m_binding = s_host->createBindingFromJSON(json.c_str());
        setGraph(m_binding.getGraph());
        storeParameterPortsNames();
    }
    catch (FabricCore::Exception e)
    {
        printf("Error: %s\n", e.getDesc_cstr());
        return false;
    }
    return true;
}

void FabricDFGView::setLogFunc(void (*in_logFunc)(void*, const char*, unsigned int))
{
    s_logFunc = in_logFunc;
}

void FabricDFGView::onNotification(char const* json)
{
}

void FabricDFGView::onPortInserted(FabricServices::DFGWrapper::Port port)
{
    switch (port.getPortType())
    {
    case FabricCore::DFGPortType_In:
        if (port.getDataType() == "SInt32" || port.getDataType() == "Integer")
        {
            m_inSInt32Names.push_back(port.getName());
            MultiParams::addIntParameterInst(m_op, port.getName(), 1);
        }
        else if (port.getDataType() == "Float32" || port.getDataType() == "Scalar")
        {
            m_inFloat32Names.push_back(port.getName());
            MultiParams::addFloatParameterInst(m_op, port.getName(), 1.0);
        }
        else if (port.getDataType() == "String")
        {
            m_inStringNames.push_back(port.getName());
            MultiParams::addStringParameterInst(m_op, port.getName());
        }
        else if (port.getDataType() == "FilePath")
        {
            m_inFilePathNames.push_back(port.getName());
            MultiParams::addStringParameterInst(m_op, port.getName(), "FilePath");
        }
        else if (port.getDataType() == "Vec3")
        {
            m_inVec3Names.push_back(port.getName());
            MultiParams::addVec3ParameterInst(m_op, port.getName(), Imath::Vec3<float>(0));
        }
        else
        {
            std::cout <<  port.getName() << " is a Canvas only input ! (Not reflected by Houdini)" <<  std::endl;
        }

        break;

    case FabricCore::DFGPortType_IO:
        break;
    case FabricCore::DFGPortType_Out:
        if (port.getDataType() == "PolygonMesh")
            m_outPolygonMeshNames.push_back(port.getName());

        else if (port.getDataType() == "Mat44")
            m_outMat44Names.push_back(port.getName());

        break;
    }

    int val = m_op->evalInt("__portsChanged", 0, 0);
    m_op->setInt("__portsChanged", 0, 0, (val + 1) % 2);

    m_op->setString(UT_String(getJSON().c_str()), CH_STRING_LITERAL, "jsonData", 0, 0);
}

void FabricDFGView::onPortRenamed(FabricServices::DFGWrapper::Port port, const char* oldName)
{
    std::string dataType = port.getDataType();

    // Need to check if dataType is supported !!!

    // Update input port names
    ParameterPortsNames* portNames = m_parameterPortsMap[dataType];

    ParameterPortsNames::iterator it = std::find(portNames->begin(), portNames->end(), std::string(oldName));
    if (it != portNames->end())
        *it = port.getName();

    // Update multi-parameters instance name
    MultiParams::renameInstance(m_op, dataType, oldName, port.getName());

    m_op->setString(UT_String(getJSON().c_str()), CH_STRING_LITERAL, "jsonData", 0, 0);
}

void FabricDFGView::onPortRemoved(FabricServices::DFGWrapper::Port port)
{
    // Using "if not removed" is a workarround as port.getDataType()
    // does not work as expected here for some unknown reasons.
    bool removed = MultiParams::removeFloatParameterInst(m_op, port.getName());
    if (removed)
        m_inFloat32Names.erase(std::find(m_inFloat32Names.begin(), m_inFloat32Names.end(), port.getName()));

    else if (!removed)
    {
        removed = MultiParams::removeIntParameterInst(m_op, port.getName());
        if (removed)
            m_inSInt32Names.erase(std::find(m_inSInt32Names.begin(), m_inSInt32Names.end(), port.getName()));
    }
    else if (!removed)
    {
        removed = MultiParams::removeStringParameterInst(m_op, port.getName());
        if (removed)
            m_inStringNames.erase(std::find(m_inStringNames.begin(), m_inStringNames.end(), port.getName()));
    }
    else if (!removed)
    {
        removed = MultiParams::removeStringParameterInst(m_op, port.getName(), "FilePath");
        if (removed)
            m_inFilePathNames.erase(std::find(m_inFilePathNames.begin(), m_inFilePathNames.end(), port.getName()));
    }
    else if (!removed)
    {
        removed = MultiParams::removeVec3ParameterInst(m_op, port.getName());
        if (removed)
            m_inVec3Names.erase(std::find(m_inVec3Names.begin(), m_inVec3Names.end(), port.getName()));
    }

    if (removed)
        m_op->setString(UT_String(getJSON().c_str()), CH_STRING_LITERAL, "jsonData", 0, 0);
}

// To mov
void FabricDFGView::storeParameterPortsNames()
{
    std::vector<DFGWrapper::Port> ports = m_binding.getGraph().getPorts();
    for (std::vector<DFGWrapper::Port>::iterator it = ports.begin(); it != ports.end(); it++)
    {
        // printf("Port name: %s\n", it->getName().c_str());
        // printf("Port dataType: %s\n", it->getDataType().c_str());
        // printf("Port is array: %s\n", it->isArray() ? "true" : "false");

        if (it->getPortType() == FabricCore::DFGPortType_In)
        {
            if (it->getDataType() == "SInt32")
                m_inSInt32Names.push_back(it->getName());
            else if (it->getDataType() == "Float32")
                m_inFloat32Names.push_back(it->getName());
            else if (it->getDataType() == "String")
                m_inStringNames.push_back(it->getName());
            else if (it->getDataType() == "FilePath")
                m_inFilePathNames.push_back(it->getName());
            else if (it->getDataType() == "Vec3")
                m_inVec3Names.push_back(it->getName());

            else
                std::cout <<  it->getName() << " is a Canvas only input ! (Not reflected by Houdini)" <<  std::endl;
        }

        // case FabricCore::DFGPortType_IO:
        //     break;

        // case FabricCore::DFGPortType_Out:
        //     if (it->getDataType() == "PolygonMesh")
        //         m_outPolygonMeshNames.push_back(it->getName());
        //     else if (it->getDataType() == "Mat44")
        //         m_outMat44Names.push_back(it->getName());

        //     break;
        // }
    }
}

void FabricDFGView::setSInt32PortValue(const char* name, int val)
{
    DFGWrapper::Port port = m_binding.getGraph().getPort(name);
    if (port.isValid())
    {
        FabricCore::RTVal rtVal = FabricCore::RTVal::ConstructSInt32(s_client, val);
        port.setRTVal(rtVal);
    }
}

void FabricDFGView::setFloat32PortValue(const char* name, float val)
{
    DFGWrapper::Port port = m_binding.getGraph().getPort(name);
    if (port.isValid())
    {
        FabricCore::RTVal rtVal = FabricCore::RTVal::ConstructFloat32(s_client, val);
        port.setRTVal(rtVal);
    }
}

void FabricDFGView::setStringPortValue(const char* name, const char* val)
{
    DFGWrapper::Port port = m_binding.getGraph().getPort(name);
    if (port.isValid())
    {
        FabricCore::RTVal rtVal = FabricCore::RTVal::ConstructString(s_client, val);
        port.setRTVal(rtVal);
    }
}

void FabricDFGView::setFilePathPortValue(const char* name, const char* val)
{
    DFGWrapper::Port port = m_binding.getGraph().getPort(name);
    if (port.isValid())
    {
        FabricCore::RTVal rtString = FabricCore::RTVal::ConstructString(s_client, val);
        FabricCore::RTVal rtVal = FabricCore::RTVal::Create(s_client, "FilePath", 1, &rtString);
        port.setRTVal(rtVal);
    }
}

void FabricDFGView::setVec3PortValue(const char* name, const Imath::Vec3<float>& val)
{
    DFGWrapper::Port port = m_binding.getGraph().getPort(name);
    if (port.isValid())
    {
        FabricCore::RTVal rtVec[3];
        rtVec[0] = FabricCore::RTVal::ConstructFloat32(*getClient(), val.x);
        rtVec[1] = FabricCore::RTVal::ConstructFloat32(*getClient(), val.y);
        rtVec[2] = FabricCore::RTVal::ConstructFloat32(*getClient(), val.z);
        FabricCore::RTVal rtVal = FabricCore::RTVal::Construct(*getClient(), "Vec3", 3, rtVec);
        port.setRTVal(rtVal);
    }
}

FabricCore::RTVal FabricDFGView::getPolygonMeshRTVal(const char* name)
{
    FabricCore::RTVal rtVal = m_binding.getGraph().getPort(name).getRTVal();
    return rtVal;
}

FabricCore::RTVal FabricDFGView::getMat44RTVal(const char* name)
{
    FabricCore::RTVal rtVal = m_binding.getGraph().getPort(name).getRTVal();
    return rtVal;
}

void FabricDFGView::logFunc(void* userData, const char* message, unsigned int length)
{
    if (s_logFunc)
    {
        s_logFunc(userData, message, length);
    }
    else
    {
        printf("FabricDFGView: %s\n", message);
    }
}

void FabricDFGView::logErrorFunc(void* userData, const char* message, unsigned int length)
{
    if (s_logErrorFunc)
    {
        s_logErrorFunc(userData, message, length);
    }
    else
    {
        printf("FabricDFGView: error: %s\n", message);
    }
}

} // End namespace OpenSpliceHoudini
