#include "CanvasUI.h"
#include "FabricDFGView.h"
#include "FabricDFGWidget.h"

#include <QtGui/QApplication>
#include <QtGui/QWidget>
#include <RE/RE_QtWindow.h>

#include <OP/OP_Node.h>

namespace OpenSpliceHoudini
{

void CanvasUI::show()
{
    // // qApp->aboutQt();
    // QWidget* parent = RE_QtWindow::mainQtWindow(); // NOT WORKING
    QWidget* parent = 0; // WORKING, the new widget will be a window as specified in Qt Doc
    FabricDFGWidgetPtr dfgw(new FabricDFGWidget(parent, &m_view));
    dfgw->setOp(m_op);
    m_view.setWidget(dfgw);
    UT_WorkBuffer opFullName;
    m_op->getFullPath(opFullName);
    dfgw->setWindowTitle(opFullName.buffer());
    dfgw->show();
    dfgw->activateWindow();
}
}
