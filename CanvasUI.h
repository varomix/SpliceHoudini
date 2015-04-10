#ifndef __UI__
#define __UI__

#define FEC_SHARED
#define FECS_SHARED

class OP_Node;

namespace OpenSpliceHoudini
{

class FabricDFGView;

class CanvasUI
{

public:
    CanvasUI(OP_Node* op, FabricDFGView& view)
        : m_op(op)
        , m_view(view)
    {
    }
    void show();

private:
    OP_Node* m_op;
    FabricDFGView& m_view;
};
}
#endif // __UI__