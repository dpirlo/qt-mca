#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"
#include "SetPreferences.h"
#include "apMCAE.hpp"
#include <QString>

#define MULTIHEAD 1
#define MONOHEAD 0
#define MULTIMODE 1
#define TEMPERATURE 2
#define MONOMODE 0
#define CHANNELS 1024

using namespace ap;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    enum temp_code {TOO_HOT,HOT,WARM,NORMAL,ERROR,NO_VALUE};

public:
    explicit MainWindow(QWidget *parent = 0);
    void checkCombosStatus();    
    ~MainWindow();
    /* Pruebas */

private slots:
    /* Slots de sincronización en el entorno gráfico */
    void setHeadMode(int index, string tab);
    void setHeadModeConfig(int index);
    void setHeadModeGraph(int index);    
    void setAdquireMode(int index);
    void setTabMode(int index);
    void syncHeadComboBoxToMCA(int index);
    void syncHeadModeComboBoxToMCA(int index);
    void syncCheckBoxHead1ToMCA(bool check);
    void syncCheckBoxHead2ToMCA(bool check);
    void syncCheckBoxHead3ToMCA(bool check);
    void syncCheckBoxHead4ToMCA(bool check);
    void syncCheckBoxHead5ToMCA(bool check);
    void syncCheckBoxHead6ToMCA(bool check);
    void syncHeadComboBoxToConfig(int index);
    void syncHeadModeComboBoxToConfig(int index);
    void syncCheckBoxHead1ToConfig(bool check);
    void syncCheckBoxHead2ToConfig(bool check);
    void syncCheckBoxHead3ToConfig(bool check);
    void syncCheckBoxHead4ToConfig(bool check);
    void syncCheckBoxHead5ToConfig(bool check);
    void syncCheckBoxHead6ToConfig(bool check);

    /* Buttons */
    int on_pushButton_conectar_clicked();
    void on_pushButton_triple_ventana_clicked();
    void on_pushButton_hv_clicked();
    void on_pushButton_energia_clicked();
    void on_pushButton_posicion_X_clicked();
    void on_pushButton_posicion_Y_clicked();    
    void on_pushButton_obtener_rutas_clicked();
    void on_pushButton_tiempos_cabezal_clicked();
    void on_pushButton_configurar_clicked();
    void on_pushButton_hv_set_clicked();
    void on_pushButton_hv_on_clicked();
    void on_pushButton_hv_off_clicked();
    void on_pushButton_hv_estado_clicked();
    void on_pushButton_adquirir_clicked();    
    void on_pushButton_decrease_clicked();
    void on_pushButton_increase_clicked();
    void on_pushButton_hv_configure_clicked();
    void on_pushButton_l_5_clicked();
    void on_pushButton_l_10_clicked();
    void on_pushButton_l_50_clicked();
    void on_pushButton_p_5_clicked();
    void on_pushButton_p_10_clicked();
    void on_pushButton_p_50_clicked();
    void on_pushButton_reset_clicked();
    void on_pushButton_head_init_clicked();
    void on_pushButton_arpet_on_clicked();
    void on_pushButton_arpet_off_clicked();
    void on_actionPreferencias_triggered();
    void on_pushButton_send_terminal_clicked();
    void on_pushButton_flush_terminal_clicked();
    void on_pushButton_stream_configure_mca_terminal_clicked();
    void on_pushButton_stream_configure_psoc_terminal_clicked();

    /*Buttons de prueba*/

private:
    QString openConfigurationFile();
    void getPaths();
    int parseConfigurationFile(QString filename);
    QStringList availablePortsName();
    QString getHead(string tab);
    void SetInitialConfigurations();
    void setLabelState(bool state, QLabel *label, bool power_off=false);    
    string ReadString(char delimeter='\r');
    string ReadBufferString(int buffer_size);
    size_t SendString(string msg, string end);
    void manageHeadCheckBox(string tab, bool show);
    void manageHeadComboBox(string tab, bool show);
    QString getMCA(string tab, string function);
    void setMCAEDataStream(string tab, string function, string pmt, string mca_function, int bytes_mca=0, string hv_value="");
    int setPSOCDataStream(string tab, string function, QString psoc_value="");
    void getPlot(bool accum, QCustomPlot *graph);
    QString setHV(string tab, string hv_value, string pmt);
    int getPMT(QLineEdit *line_edit);
    QString getPSOCAlta(QLineEdit *line_edit);
    void setPMT(int value);
    string getHVValue(QLineEdit *line_edit, int value=0);
    void resetHitsValues();
    void getPMTLabelNames();
    void drawTemperatureBoard();
    void setTemperatureBoard(double temp, QLabel *label_pmt, int pmt);
    void clearTemperatureBoard();
    temp_code getTemperatureCode(double temperature);
    QVector<double> getValuesFromFiles(QString filename, bool hv);
    void getARPETStatus();
    void showMCAEStreamDebugMode(string msg);

private:
    Ui::MainWindow *ui;
    SetPreferences *pref;
    shared_ptr<MCAE> arpet;
    QList<QLabel*> pmt_label_table;
    QList<QLabel*> head_status_table;
    QList<QLabel*> pmt_status_table;
    QList<QLabel*> hv_status_table;
    int adquire_mode;
    int bytes_int;
    bool debug;
    QString coefenerg, coefT, hvtable, coefx, coefy, coefest;
    QVector<double> hvtable_values, coefenerg_values, coefT_values, coefx_values, coefy_values, coefest_values;
    int  AT, LowLimit;
    int TimeOut;
    QVector<double> channels_ui,hits_ui;    
    int pmt_ui_current, pmt_ui_previous;

public:
    void setDebugMode(bool mode) { debug = mode; }

};

#endif // MAINWINDOW_H
