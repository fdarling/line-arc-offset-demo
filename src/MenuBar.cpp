#include "MenuBar.h"
#include "Actions.h"

namespace LineArcOffsetDemo {

MenuBar::MenuBar(Actions *actions, QWidget *parent) : QMenuBar(parent)
{
    QMenu * const fileMenu = addMenu("File");
    fileMenu->addAction(actions->fileNew);
    fileMenu->addAction(actions->fileOpen);
    fileMenu->addAction(actions->fileSaveAs);
    fileMenu->addAction(actions->fileQuit);
}

} // namespace LineArcOffsetDemo
