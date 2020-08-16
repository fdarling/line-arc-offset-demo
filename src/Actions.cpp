#include "Actions.h"

namespace LineArcOffsetDemo {

Actions::Actions(QObject *parent) : QObject(parent)
{
    // file menu
    fileNew = new QAction("&New", this);
    fileOpen = new QAction("&Open", this);
    fileSaveAs = new QAction("Save &As", this);
    fileQuit = new QAction("&Quit", this);
    fileOpen->setEnabled(false);
    fileSaveAs->setEnabled(false);
}

} // namespace LineArcOffsetDemo
