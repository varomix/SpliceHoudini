#include "FabricDFGView.h"
#include "MultiParams.h"
#include "ParameterFactory.h"

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

    setMyGraph();

    ParameterFactory::RegisterTypes();
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
        return m_binding.exportJSON();
    }
    catch (FabricCore::Exception e)
    {
        FabricCore::Exception::Throw(
            (std::string("[FabricDFGView::createBindingFromJSON: ") + e.getDesc_cstr()).c_str());
    }
    return "";
}

void FabricDFGView::createBindingFromJSON(const std::string& json)
{
    try
    {
        m_binding = s_host->createBindingFromJSON(json.c_str());
    }
    catch (FabricCore::Exception e)
    {
        FabricCore::Exception::Throw(
            (std::string("[FabricDFGView::createBindingFromJSON: ") + e.getDesc_cstr()).c_str());
    }
}

void FabricDFGView::setLogFunc(void (*in_logFunc)(void*, const char*, unsigned int))
{
    s_logFunc = in_logFunc;
}

void FabricDFGView::onPortInserted(FabricServices::DFGWrapper::PortPtr port)
{
}

void FabricDFGView::onPortResolvedTypeChanged(FabricServices::DFGWrapper::PortPtr port, const char* resolvedType)
{
    std::string portResolvedType(resolvedType);
    bool outPutPortChanged = false;

    switch (port->getPortType())
    {
    case FabricCore::DFGPortType_In:
    {
        ParameterFactory::CreateParameterFunc addParam = ParameterFactory::Get(portResolvedType);
        if (addParam)
        {
            addParam(m_op, port->getName());
        }
        else
        {
            std::cout << "FabricDFGView::onPortResolvedTypeChanged: " << port->getName()
                      << " is a Canvas only input ! Type " << portResolvedType << " not reflected by Houdini"
                      << std::endl;
        }        
        break;
    }

    case FabricCore::DFGPortType_IO:
        break;
    case FabricCore::DFGPortType_Out:
        if (portResolvedType == "PolygonMesh")
        {
            m_outPolyMeshPorts.push_back(port);
            outPutPortChanged = true;
        }
        break;
    }

    if (outPutPortChanged)
    {
        cookMyOp(true);
    }
}

void FabricDFGView::onPortRenamed(FabricServices::DFGWrapper::PortPtr port, const char* oldName)
{
    std::string resolvedType(port->getResolvedType());

    // Need to check if resolvedType is supported !!!

    // Update multi-parameters instance name
    MultiParams::renameInstance(m_op, resolvedType, oldName, port->getName());
    saveJsonData();
}

void FabricDFGView::onPortRemoved(FabricServices::DFGWrapper::PortPtr port)
{
    // Using "if not removed" is a workarround as port->getDataType()
    // does not work as expected here for some unknown reasons.
    bool removed = MultiParams::removeFloatParameterInst(m_op, port->getName());

    if (!removed)
    {
        removed = MultiParams::removeIntParameterInst(m_op, port->getName());
    }
    if (!removed)
    {
        removed = MultiParams::removeIntParameterInst(m_op, port->getName(), "UInt32");
    }
    if (!removed)
    {
        removed = MultiParams::removeStringParameterInst(m_op, port->getName());
    }
    if (!removed)
    {
        removed = MultiParams::removeStringParameterInst(m_op, port->getName(), "FilePath");
    }
    if (!removed)
    {
        removed = MultiParams::removeVec3ParameterInst(m_op, port->getName());
    }

    if (removed)
        saveJsonData();
}

void FabricDFGView::onEndPointsConnected(FabricServices::DFGWrapper::EndPointPtr src,
                                         FabricServices::DFGWrapper::EndPointPtr dst)
{
    storeOutputPolymeshPorts();
    cookMyOp(true);
}

void FabricDFGView::storeOutputPolymeshPorts()
{
    std::vector<DFGWrapper::PortPtr> ports = m_binding.getExecutable()->getPorts();
    for (std::vector<DFGWrapper::PortPtr>::iterator it = ports.begin(); it != ports.end(); it++)
    {
        DFGWrapper::PortPtr port = *it;
        std::string resolvedType(port->getResolvedType());

        if (port->getPortType() == FabricCore::DFGPortType_Out && resolvedType == "PolygonMesh")
            m_outPolyMeshPorts.push_back(port);
    }
}

void FabricDFGView::cookMyOp(bool saveGraph)
{
    int val = m_op->evalInt("__portsChanged", 0, 0);
    m_op->setInt("__portsChanged", 0, 0, (val + 1) % 2);
    if (saveGraph)
        saveJsonData();
}

void FabricDFGView::saveJsonData()
{
    m_op->setString(UT_String(getJSON().c_str()), CH_STRING_LITERAL, "jsonData", 0, 0);
}

void FabricDFGView::setSInt32PortValue(const char* name, int val)
{
    DFGWrapper::PortPtr port = m_binding.getExecutable()->getPort(name);
    if (port->isValid())
    {
        FabricCore::RTVal rtVal = FabricCore::RTVal::ConstructSInt32(s_client, val);
        m_binding.setArgValue(name, rtVal);
    }
}

void FabricDFGView::setUInt32PortValue(const char* name, int val)
{
    DFGWrapper::PortPtr port = m_binding.getExecutable()->getPort(name);
    if (port->isValid())
    {
        FabricCore::RTVal rtVal = FabricCore::RTVal::ConstructUInt32(s_client, val);
        m_binding.setArgValue(name, rtVal);
    }
}

void FabricDFGView::setFloat32PortValue(const char* name, float val)
{
    DFGWrapper::PortPtr port = m_binding.getExecutable()->getPort(name);
    if (port->isValid())
    {
        FabricCore::RTVal rtVal = FabricCore::RTVal::ConstructFloat32(s_client, val);
        m_binding.setArgValue(name, rtVal);
    }
}

void FabricDFGView::setStringPortValue(const char* name, const char* val)
{
    DFGWrapper::PortPtr port = m_binding.getExecutable()->getPort(name);
    if (port->isValid())
    {
        FabricCore::RTVal rtVal = FabricCore::RTVal::ConstructString(s_client, val);
        m_binding.setArgValue(name, rtVal);
    }
}

void FabricDFGView::setFilePathPortValue(const char* name, const char* val)
{
    DFGWrapper::PortPtr port = m_binding.getExecutable()->getPort(name);
    if (port->isValid())
    {
        FabricCore::RTVal rtString = FabricCore::RTVal::ConstructString(s_client, val);
        FabricCore::RTVal rtVal = FabricCore::RTVal::Create(s_client, "FilePath", 1, &rtString);
        m_binding.setArgValue(name, rtVal);
    }
}

void FabricDFGView::setVec3PortValue(const char* name, const Imath::Vec3<float>& val)
{
    DFGWrapper::PortPtr port = m_binding.getExecutable()->getPort(name);
    if (port->isValid())
    {
        FabricCore::RTVal rtVec[3];
        rtVec[0] = FabricCore::RTVal::ConstructFloat32(*getClient(), val.x);
        rtVec[1] = FabricCore::RTVal::ConstructFloat32(*getClient(), val.y);
        rtVec[2] = FabricCore::RTVal::ConstructFloat32(*getClient(), val.z);
        FabricCore::RTVal rtVal = FabricCore::RTVal::Construct(*getClient(), "Vec3", 3, rtVec);
        m_binding.setArgValue(name, rtVal);
    }
}

FabricCore::RTVal FabricDFGView::getMat44RTVal(const char* name)
{
    FabricCore::RTVal rtVal = m_binding.getArgValue(name);
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
