// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#ifndef __SOP_FABRICSDFGDEFORMER_H__
#define __SOP_FABRICSDFGDEFORMER_H__

#include <SOP/SOP_Node.h>
#include "core/FabricDFGOP.h"

#include <vector>
#include <map>

namespace OpenSpliceHoudini
{

class SOP_FabricDeformer : public FabricDFGOP<SOP_Node>
{
    template <typename T>
    class ManagedBuffer
    {
        typedef std::vector<T> Buffer;
        typedef boost::shared_ptr<Buffer> BufferPtr;
        typedef std::vector<BufferPtr> Buffers;

    public:
        /// Add new buffer
        void create(const size_t size)
        {
            BufferPtr buffer(new Buffer(size));
            m_buffers.push_back(buffer);
        }

        /// Get last created buffer
        T* get()
        {
            BufferPtr ptr = m_buffers.back();
            return &(*ptr)[0];
        }

    private:
        Buffers m_buffers;
    };

public:
    static OP_TemplatePair* buildTemplatePair(OP_TemplatePair* prevstuff);
    static OP_Node* myConstructor(OP_Network*, const char*, OP_Operator*);

protected:
    SOP_FabricDeformer(OP_Network* net, const char* name, OP_Operator* op);
    virtual ~SOP_FabricDeformer();
    virtual OP_ERROR cookMySop(OP_Context& context);

    static void OnUpdateGraphCopyAttributes(OP_Network& node, DFGWrapper::Binding& binding);

private:
    static FabricCore::RTVal CreatePolygonMeshRTVal(const GU_Detail& gdpRef, SOP_FabricDeformer& sopDeformerNode);
    void setPointsPositions(OP_Context& context);

    void addIntegerBuffer(const size_t size) { m_integerBuffers.create(size); }
    int* getIntegerBuffer() { return m_integerBuffers.get(); }

    void addFloatBuffer(const size_t size) { m_floatBuffers.create(size); }
    float* getFloatBuffer() { return m_floatBuffers.get(); }

    void addVec3Buffer(const size_t size) { m_vec3Buffers.create(size); }
    UT_Vector3F* getVec3Buffer() { return m_vec3Buffers.get(); }

    ManagedBuffer<int> m_integerBuffers;
    ManagedBuffer<float> m_floatBuffers;
    ManagedBuffer<UT_Vector3F> m_vec3Buffers;
};

class OP_SOP_FabricDeformer : public OP_Operator
{
public:
    OP_SOP_FabricDeformer()
        : OP_Operator("fabricDeformer",                            // Internal name
                      "Fabric Deformer",                           // UI name
                      SOP_FabricDeformer::myConstructor,        // How to build the SOP
                      SOP_FabricDeformer::buildTemplatePair(0), // My parameters
                      1,                                           // Min # of node inputs
                      4)                                           // Max # of node inputs
    {
    }
};

} // End namespace OpenSpliceHoudini
#endif // __SOP_FABRICSDFGDEFORMER_H__
