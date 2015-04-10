#ifndef __MultiParams_H_
#define __MultiParams_H_

#include <PRM/PRM_Parm.h>
#include <PRM/PRM_SpareData.h>
#include <OP/OP_Operator.h>

#include <ImathVec.h>

#define DECLARE_MULTI_PARAMETER_TYPE(TOKEN)                                                                            \
    static PRM_Name TOKEN##Ports;                                                                                      \
    static PRM_Name TOKEN##Port;                                                                                       \
    static PRM_Default TOKEN##PortDefault;                                                                             \
    static PRM_Name TOKEN##PortVal;                                                                                    \
    static PRM_Name TOKEN##PortxVal;                                                                                   \
    static PRM_Name TOKEN##PortyVal;                                                                                   \
    static PRM_Name TOKEN##PortzVal;                                                                                   \
    static PRM_Default TOKEN##PortValDefault;                                                                          \
    static PRM_Template TOKEN##PortsTemplate[];                                                                        \
    static PRM_Template TOKEN##PortsMultiTemplate

namespace OpenSpliceHoudini
{

/// Fabric DFG input ports are reflected in the Houdini node UI (at least for basic types)
/// We use Houdini Multi-parameters to be able to dynamically add or remove a multi-param instance
/// when the user add or delete an input port in the DFG.
/// This class is doing the "dirty work" to add instances to multi-parameters.
class MultiParams
{

public:
    MultiParams();
    ~MultiParams();

    DECLARE_MULTI_PARAMETER_TYPE(Float32);
    DECLARE_MULTI_PARAMETER_TYPE(SInt32);
    DECLARE_MULTI_PARAMETER_TYPE(String);
    DECLARE_MULTI_PARAMETER_TYPE(FilePath);
    DECLARE_MULTI_PARAMETER_TYPE(Vec3);

    static void clear(OP_Parameters* op);

    static void addFloatParameterInst(OP_Parameters* op, const std::string& name, float val);
    static bool removeFloatParameterInst(OP_Parameters* op, const std::string& name);
    static const UT_String getParameterInstFloatName(OP_Parameters* op, int instance_idx);
    static float getParameterInstFloatValue(OP_Parameters* op, int instance_idx, fpreal t = 0);

    static void addIntParameterInst(OP_Parameters* op, const std::string& name, int val);
    static bool removeIntParameterInst(OP_Parameters* op, const std::string& name);
    static const UT_String getParameterInstIntName(OP_Parameters* op, int instance_idx);
    static int getParameterInstIntValue(OP_Parameters* op, int instance_idx, fpreal t = 0);

    static void addStringParameterInst(OP_Parameters* op,
                                       const std::string& name,
                                       const std::string& option = "string",
                                       const char* val = "");
    static bool
    removeStringParameterInst(OP_Parameters* op, const std::string& name, const std::string& option = "string");
    static const UT_String
    getParameterInstStringName(OP_Parameters* op, int instance_idx, const std::string& option = "string");
    static const UT_String getParameterInstStringValue(OP_Parameters* op,
                                                       int instance_idx,
                                                       fpreal t = 0,
                                                       const std::string& option = "string");


    static void addVec3ParameterInst(OP_Parameters* op, const std::string& name, Imath::Vec3<float> val);
    static bool removeVec3ParameterInst(OP_Parameters* op, const std::string& name);
    static const UT_String getParameterInstVec3Name(OP_Parameters* op, int instance_idx);
    static Imath::Vec3<float> getParameterInstVec3Value(OP_Parameters* op, int instance_idx, fpreal t = 0);

    static void renameInstance(OP_Parameters* op,
                               const std::string& multiParmTypeName,
                               const std::string& oldName,
                               const std::string& newName);

private:
    static int addInstance(OP_Parameters* op, const std::string& multiParmName, const std::string& name);
    static bool removeInstance(OP_Parameters* op, const std::string& multiParmTypeName, const std::string& name);
    static PRM_Default portDefaultName;
};

} // End namespace OpenSpliceHoudini
#endif // __MultiParams_H_