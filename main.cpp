#include <QtGui>

#include "lantern.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(stylesheet);

    QApplication app(argc, argv);

    Lantern window;
    window.show();

    return app.exec();
}
