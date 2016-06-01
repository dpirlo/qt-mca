#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"
#include "apMCAE.hpp"
#include <QString>

#define MULTIHEAD 1
#define MONOHEAD 0
#define MULTIMODE 1
#define MONOMODE 0

using namespace ap;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);    
    void checkCombosStatus();
    ~MainWindow();
    /* Pruebas */
    QByteArray getPart(const QByteArray& message, int part, bool toEnd);

private slots:
    void setHeadMode(int index, string tab);
    void setHeadModeConfig(int index);
    void setHeadModeGraph(int index);
    void setAdquireMode(int index);
    int on_pushButton_conectar_clicked();    
    void on_pushButton_triple_ventana_clicked();
    void on_pushButton_hv_clicked();
    void on_pushButton_energia_clicked();
    void on_pushButton_posicion_X_clicked();
    void on_pushButton_posicion_Y_clicked();
    void on_pushButton_salir_graficos_clicked();
    void on_pushButton_salir_clicked();
    void on_pushButton_obtener_rutas_clicked();
    void on_pushButton_tiempos_cabezal_clicked();
    void on_pushButton_configurar_clicked();
    void on_pushButton_hv_set_clicked();
    void on_pushButton_hv_on_clicked();
    void on_pushButton_hv_off_clicked();
    void on_pushButton_hv_estado_clicked();
    void on_pushButton_adquirir_clicked();
    void on_pushButton_clicked();
    void on_pushButton_decrease_clicked();
    void on_pushButton_increase_clicked();

    /*Buttons de prueba*/
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();    
    void on_pushButton_9_clicked();

private:
    QString openConfigurationFile();
    int parseConfigurationFile(QString filename);
    QStringList availablePortsName();
    QString getHead(string tab);
    void setLabelState(bool state, QLabel *label);
    string ReadString();
    string ReadBufferString(int buffer_size);
    size_t SendString(string msg, string end);
    void manageHeadCheckBox(string tab, bool show);
    void manageHeadComboBox(string tab, bool show);


private:
    Ui::MainWindow *ui;
    shared_ptr<MCAE> arpet;
    QString coefenerg, coefT, hvtable, coefx, coefy, coefest;
    int  AT, LowLimit;
    int TimeOut;
};

#endif // MAINWINDOW_H
