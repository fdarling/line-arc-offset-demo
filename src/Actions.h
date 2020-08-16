#ifndef LINEARCOFFSETDEMO_ACTIONS_H
#define LINEARCOFFSETDEMO_ACTIONS_H

#include <QObject>
#include <QAction>

namespace LineArcOffsetDemo {

class Actions : public QObject
{
    Q_OBJECT
public:
    Actions(QObject *parent = nullptr);
public:
    QAction *fileNew;
    QAction *fileOpen;
    QAction *fileSaveAs;
    QAction *fileQuit;
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_ACTIONS_H
