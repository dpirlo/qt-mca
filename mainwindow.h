#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "./qcustomplot.h"
#include "apComunicacionMCA.hpp"

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
    int on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_triple_ventana_clicked();
    void on_pushButton_hv_clicked();
    void on_pushButton_energia_clicked();
    void on_pushButton_posicion_X_clicked();
    void on_pushButton_posicion_Y_clicked();
    void on_pushButton_salir_graficos_clicked();
    void on_pushButton_salir_clicked();
    void SetLabelState(bool state, QLabel *label);
    QString OpenConfigurationFile();


    /* EJEMPLOS, luego eliminarlos */
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();

    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void recibirdatosSerie();

private:
    Ui::MainWindow *ui;
    ComunicacionMCA mca;
};

#endif // MAINWINDOW_H
