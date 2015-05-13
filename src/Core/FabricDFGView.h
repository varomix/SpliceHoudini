// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

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

class OP_Network;

namespace OpenSpliceHoudini
{

typedef void (*CopyAttributesFunc)(OP_Network& node, FabricServices::DFGWrapper::Binding& binding);

/// A Fabric View with access to an Houdini node
class FabricDFGView : public FabricServices::DFGWrapper::View
{

public:
    FabricDFGView(OP_Network* op);
    ~FabricDFGView();

    unsigned int getId();
    static FabricDFGView* getFromId(unsigned int id);

    static FabricCore::Client* getClient();
    static FabricServices::DFGWrapper::Host* getHost();
    FabricServices::DFGWrapper::Binding* getBinding();
    static FabricServices::ASTWrapper::KLASTManager* getManager();
    static FabricServices::Commands::CommandStack* getStack();

    /// persistence
    std::string getJSON();
    void createBindingFromJSON(const std::string& json);

    /// logging.
    static void setLogFunc(void (*in_logFunc)(void*, const char*, unsigned int));
    static void logErrorFunc(void* userData, const char* message, unsigned int length);

    static void (*s_logFunc)(void*, const char*, unsigned int);
    static void (*s_logErrorFunc)(void*, const char*, unsigned int);

    void setMyGraph()
    {
        setGraph(FabricServices::DFGWrapper::GraphExecutablePtr::StaticCast(m_binding.getExecutable()));
    }

    void setWidget(FabricDFGWidgetPtr widget)
    {
        m_widget = widget;
    }

    bool hasOuputPort();

protected:
    /*! \name Notifications
    */
    virtual void onNotification(char const* json)
    {
    }
    virtual void onNodeInserted(FabricServices::DFGWrapper::NodePtr node)
    {
    }
    virtual void onNodeRemoved(FabricServices::DFGWrapper::NodePtr node)
    {
    }
    virtual void onPinInserted(FabricServices::DFGWrapper::PinPtr pin)
    {
    }
    virtual void onPinRemoved(FabricServices::DFGWrapper::PinPtr pin)
    {
    }
    virtual void onPortInserted(FabricServices::DFGWrapper::PortPtr port);
    virtual void onPortRemoved(FabricServices::DFGWrapper::PortPtr port);

    virtual void onEndPointsConnected(FabricServices::DFGWrapper::EndPointPtr src,
                                      FabricServices::DFGWrapper::EndPointPtr dst);
    virtual void onEndPointsDisconnected(FabricServices::DFGWrapper::EndPointPtr src,
                                         FabricServices::DFGWrapper::EndPointPtr dst);

    virtual void onNodeMetadataChanged(FabricServices::DFGWrapper::NodePtr node, const char* key, const char* metadata)
    {
    }
    virtual void onNodeTitleChanged(FabricServices::DFGWrapper::NodePtr node, const char* title)
    {
    }
    virtual void onPortRenamed(FabricServices::DFGWrapper::PortPtr port, const char* oldName);
    virtual void onPinRenamed(FabricServices::DFGWrapper::PinPtr pin, const char* oldName)
    {
    }

    virtual void
    onExecMetadataChanged(FabricServices::DFGWrapper::ExecutablePtr exec, const char* key, const char* metadata)
    {
    }

    virtual void onExtDepAdded(const char* extension, const char* version)
    {
    }

    virtual void onExtDepRemoved(const char* extension, const char* version)
    {
    }

    virtual void onNodeCacheRuleChanged(const char* path, const char* rule)
    {
    }
    virtual void onExecCacheRuleChanged(const char* path, const char* rule)
    {
    }

    virtual void onPortResolvedTypeChanged(FabricServices::DFGWrapper::PortPtr port, const char* resolvedType);

    virtual void onPortTypeSpecChanged(FabricServices::DFGWrapper::PortPtr port, const char* typeSpec)
    {
    }

    virtual void onPinResolvedTypeChanged(FabricServices::DFGWrapper::PinPtr pin, const char* resolvedType)
    {
    }

    virtual void onPortMetadataChanged(FabricServices::DFGWrapper::PortPtr port, const char* key, const char* metadata)
    {
    }

    virtual void onPinMetadataChanged(FabricServices::DFGWrapper::PinPtr pin, const char* key, const char* metadata)
    {
    }

private:
    static void logFunc(void* userData, const char* message, unsigned int length);

    static FabricCore::Client s_client;
    static FabricServices::DFGWrapper::Host* s_host;
    mutable FabricServices::DFGWrapper::Binding m_binding;
    static FabricServices::ASTWrapper::KLASTManager* s_manager;
    static FabricServices::Commands::CommandStack s_stack;
    unsigned int m_id;
    static unsigned int s_maxId;
    static std::map<unsigned int, FabricDFGView*> s_instances;

    void setSInt32PortValue(const char* name, int val);
    void setUInt32PortValue(const char* name, int val);
    void setFloat32PortValue(const char* name, float val);
    void setStringPortValue(const char* name, const char* val);
    void setFilePathPortValue(const char* name, const char* val);
    void setVec3PortValue(const char* name, const Imath::Vec3<float>& val);

public:
    /// Houdini and DFG bindings
    void addParametersFromInputPorts();
    const FabricServices::DFGWrapper::PortList getPolygonMeshOutputPorts() const;

    /// Set Canvas input ports using the Houdini node data (params and/or attributes)
    void setInputPortsFromOpNode(const float t, CopyAttributesFunc func);

private:
    void dirtyOp(bool saveGraph);
    void saveJsonData();

    FabricDFGWidgetPtr m_widget;
    OP_Network* m_op;
};
} // End namespace OpenSpliceHoudini

#endif
