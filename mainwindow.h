#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "./qcustomplot.h"
#include "apComunicacionMCA.hpp"
#include <QString>

using namespace ap;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    int on_pushButton_conectar_clicked();
    void on_pushButton_2_clicked(); // Enviar
    void on_pushButton_triple_ventana_clicked();
    void on_pushButton_hv_clicked();
    void on_pushButton_energia_clicked();
    void on_pushButton_posicion_X_clicked();
    void on_pushButton_posicion_Y_clicked();
    void on_pushButton_salir_graficos_clicked();
    void on_pushButton_salir_clicked();

    void on_pushButton_obtener_rutas_clicked();

    void on_pushButton_tiempos_cabezal_clicked();

private:
    QString openConfigurationFile();
    int parseConfigurationFile(QString filename);
    QStringList availablePortsName();
    void setLabelState(bool state, QLabel *label);

private:
    Ui::MainWindow *ui;
    ComunicacionMCA mca;
    QString coefenerg, coefT, hvtable, coefx, coefy, coefest;
    int AT, LowLimit;
};

#endif // MAINWINDOW_H
