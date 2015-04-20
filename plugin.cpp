#include <OP/OP_OperatorTable.h>

#include "SOP_FabricDFG.h"
#include "SOP_FabricDFGDeformer.h"
#include "OBJ_FabricDFG.h"

void newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OpenSpliceHoudini::OP_FabricDFG);
    table->addOperator(new OpenSpliceHoudini::OP_FabricDFGDeformer);
}

void newObjectOperator(OP_OperatorTable* table)
{
    table->addOperator(new OpenSpliceHoudini::OP_FabricDFG_OBJ);
}
