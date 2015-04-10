#ifndef __FabricDFGView_H__
#define __FabricDFGView_H__

#include "FabricDFGView.fwd.h"

#define FEC_SHARED
#define FECS_SHARED

#include <DFGWrapper/DFGWrapper.h>
#include <ASTWrapper/KLASTManager.h>
#include <Commands/CommandStack.h>

#include <vector>
#include <map>
#include <ImathVec.h>

class OP_Node;

namespace OpenSpliceHoudini
{

/// Implementation of the DFGWrapper::View
class FabricDFGView : public FabricServices::DFGWrapper::View
{

public:
    typedef std::vector<std::string> ParameterPortsNames;
    typedef std::vector<std::string> OutputPortsNames;

    FabricDFGView(OP_Node* op);
    ~FabricDFGView();

    // instance management
    // right now there are no locks in place,
    // assuming that the DCC will only access
    // these things from the main thread.
    unsigned int getId();
    static FabricDFGView* getFromId(unsigned int id);

    // accessors
    static FabricCore::Client* getClient();
    static FabricServices::DFGWrapper::Host* getHost();
    FabricServices::DFGWrapper::Binding* getBinding();
    static FabricServices::ASTWrapper::KLASTManager* getManager();
    static FabricServices::Commands::CommandStack* getStack();

    // persistence
    std::string getJSON();
    bool setFromJSON(const std::string& json);

    // logging.
    static void setLogFunc(void (*in_logFunc)(void*, const char*, unsigned int));
    static void logErrorFunc(void* userData, const char* message, unsigned int length);

    static void (*s_logFunc)(void*, const char*, unsigned int);
    static void (*s_logErrorFunc)(void*, const char*, unsigned int);

    // notifications
    // for now we only implement onPortInserted and onPortRemoved
    virtual void onNotification(char const* json);
    virtual void onNodeInserted(FabricServices::DFGWrapper::Node node)
    {
    }
    virtual void onNodeRemoved(FabricServices::DFGWrapper::Node node)
    {
    }
    virtual void onPinInserted(FabricServices::DFGWrapper::Pin pin)
    {
    }
    virtual void onPinRemoved(FabricServices::DFGWrapper::Pin pin)
    {
    }
    virtual void onPortInserted(FabricServices::DFGWrapper::Port port);
    virtual void onPortRemoved(FabricServices::DFGWrapper::Port port);
    virtual void onEndPointsConnected(FabricServices::DFGWrapper::Port src, FabricServices::DFGWrapper::Port dst)
    {
    }
    virtual void onEndPointsDisconnected(FabricServices::DFGWrapper::Port src, FabricServices::DFGWrapper::Port dst)
    {
    }
    virtual void onNodeMetadataChanged(FabricServices::DFGWrapper::Node node, const char* key, const char* metadata)
    {
    }
    virtual void onNodeTitleChanged(FabricServices::DFGWrapper::Node node, const char* title)
    {
    }
    virtual void onPortRenamed(FabricServices::DFGWrapper::Port port, const char* oldName);
    virtual void onPinRenamed(FabricServices::DFGWrapper::Pin pin, const char* oldName)
    {
    }
    virtual void
    onExecMetadataChanged(FabricServices::DFGWrapper::Executable exec, const char* key, const char* metadata)
    {
    }
    virtual void onExtDepAdded(const char* extension, const char* version)
    {
    }
    virtual void onNodeCacheRuleChanged(const char* path, const char* rule)
    {
    }
    virtual void onExecCacheRuleChanged(const char* path, const char* rule)
    {
    }

    void setWidget(FabricDFGWidgetPtr widget)
    {
        m_widget = widget;
    }

private:
    static void logFunc(void* userData, const char* message, unsigned int length);

    static FabricCore::Client s_client;
    static FabricServices::DFGWrapper::Host* s_host;
    FabricServices::DFGWrapper::Binding m_binding;
    static FabricServices::ASTWrapper::KLASTManager* s_manager;
    static FabricServices::Commands::CommandStack s_stack;

    unsigned int m_id;
    static unsigned int s_maxId;
    static std::map<unsigned int, FabricDFGView*> s_instances;

    // This part should be moved
public:
    void setSInt32PortValue(const char* name, int val);
    const ParameterPortsNames& getInputPortsSInt32Names() const
    {
        return m_inSInt32Names;
    }

    void setFloat32PortValue(const char* name, float val);
    const ParameterPortsNames& getInputPortsFloat32Names() const
    {
        return m_inFloat32Names;
    }

    void setStringPortValue(const char* name, const char* val);
    const ParameterPortsNames& getInputPortsStringNames() const
    {
        return m_inStringNames;
    }

    void setFilePathPortValue(const char* name, const char* val);
    const ParameterPortsNames& getInputPortsFilePathNames() const
    {
        return m_inFilePathNames;
    }

    void setVec3PortValue(const char* name, const Imath::Vec3<float>& val);
    const ParameterPortsNames& getInputPortsVec3Names() const
    {
        return m_inVec3Names;
    }

    FabricCore::RTVal getPolygonMeshRTVal(const char* name);
    const OutputPortsNames& getOutputPortsPolygonMeshNames() const
    {
        return m_outPolygonMeshNames;
    }

    FabricCore::RTVal getMat44RTVal(const char* name);
    const OutputPortsNames& getOutputPortsMat44Names() const
    {
        return m_outMat44Names;
    }

private:
    void storeParameterPortsNames();
    ParameterPortsNames m_inSInt32Names;
    ParameterPortsNames m_inFloat32Names;
    ParameterPortsNames m_inStringNames;
    ParameterPortsNames m_inFilePathNames;
    ParameterPortsNames m_inVec3Names;
    std::map<std::string, ParameterPortsNames*> m_parameterPortsMap;

    OutputPortsNames m_outPolygonMeshNames;
    OutputPortsNames m_outMat44Names;

    FabricDFGWidgetPtr m_widget;
    OP_Node* m_op;
};
} // End namespace OpenSpliceHoudini

#endif
