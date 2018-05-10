#include "inc/MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon(":/qss_icons/rc/icono_alpha.jpeg"));
    MainWindow w;
    w.show();
    w.checkCombosStatus();
    qRegisterMetaType<QVector<double> >("QVector<double>");
    qRegisterMetaType<Pico_espectro>("Pico_espectro");

    return a.exec();
}
