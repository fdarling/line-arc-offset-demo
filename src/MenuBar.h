#ifndef LINEARCOFFSETDEMO_MENUBAR_H
#define LINEARCOFFSETDEMO_MENUBAR_H

#include <QMenuBar>

namespace LineArcOffsetDemo {

class Actions;

class MenuBar : public QMenuBar
{
    Q_OBJECT
public:
    MenuBar(Actions *actions, QWidget *parent = nullptr);
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_MENUBAR_H
