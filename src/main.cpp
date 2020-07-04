#include <QApplication>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    // Q_INIT_RESOURCE(application);

    QApplication app(argc, argv);
    // QCoreApplication::setOrganizationName("");
    QCoreApplication::setApplicationName("LineArcOffsetDemo");
    QCoreApplication::setApplicationVersion("v0.1");

    LineArcOffsetDemo::MainWindow mainWin;
    mainWin.show();
    return app.exec();
}
