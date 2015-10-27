#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "./qcustomplot.h"
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
    void on_pushButton_clicked();
    void recibirdatosSerie();
    void Graficaalgo();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_offset_clicked();
    void on_pushButton_hv_clicked();
    void on_pushButton_energia_clicked();
    void on_pushButton_posicion_clicked();
    void on_pushButton_salir_graficos_clicked();
    void on_pushButton_salir_clicked();
    void SetLabelState(bool state, QLabel *label);
    QString OpenConfigurationFile();


    /* EJEMPLOS, luego eliminarlos */
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();



private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
