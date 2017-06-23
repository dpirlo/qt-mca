/**
 * @class MainWindow
 *
 * @brief Clase de entorno gráfico
 *
 * Esta clase corresponde al entorno gráfico de la aplicación *qt-mca*
 *
 * Tiene como objetivo realizar el control y mantenimiento del tomógrafo por
 * emisión de positrones AR-PET.
 * Realiza el matenimiento y configuración de varios parámetros del equipo, tales
 * como calibración de los fotomultiplicadores y cabezales, control de temperatura,
 * control de alta tensión y cantidad de cuentas adquiridas en el equipo.
 *
 * @note Utiliza la clase MCAE para la comunicación con el ARPET.
 *
 * @note Clase heredada de QMainWindow
 *
 * @author Ariel Hernández
 *
 * @version $Version
 *
 * Contacto: ahernandez@cae.cnea.gov.ar
 *           ariel.h.estevenz@ieee.org
 *
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QMessageBox"
#include "qcustomplot.h"
#include "SetPreferences.h"
#include "SetPMTs.h"
#include "apMCAE.hpp"
#include "apAutoCalib.hpp"
#include "apRecon.hpp"
#include "apThread.hpp"
#include "apAutoCalibThread.hpp"
#include <QThread>
#include <cstdio>
#include <QString>

#define MONOHEAD 0
#define MULTIHEAD 1
#define ALLHEADS 2
#define PMT 0
#define CABEZAL 1
#define TEMPERATURE 2
#define HEAD 0
#define HEADS 6
#define COIN_NORMAL 0
#define COIN_AUTOCOINCIDENCE 1
#define COIN_AVANCED 2
#define COIN_CALIB 3
#define COIN_VERIF 4
#define COIN_INTER_CABEZAL 5
#define TIEMPOS_NULOS_PMTS 0
#define CHAR_LF 0x0A

#define WAIT_MICROSECONDS 1000000

using namespace ap;

/**
    El Namespace Ui contiene los métodos y propiedades del entorno gráfico de la aplicación qt-mca.
*/
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    /**
     * @brief The temp_code enum
     *
     * Definición de enum utilizado para las escalas de temperatura.
     *
     */
    enum temp_code {TOO_HOT,HOT,WARM,NORMAL,ERROR,NO_VALUE};

public:
    explicit MainWindow(QWidget *parent = 0);
    void checkCombosStatus();
    ~MainWindow();

    /* Area de prueba/testing */

private slots:
    /* Slots de sincronización para QCustomPlot */
    void addGraph(QVector<double> hits,  QCustomPlot *graph, int channels, QString graph_legend, QVector<int> param);
    void addGraph_Calib(QVector<double> hits,  QCustomPlot *graph, int channels, QString graph_legend, QVector<int> param);
    void titleDoubleClickPMT(QMouseEvent* event);
    void titleDoubleClickHead(QMouseEvent* event);
    void axisLabelDoubleClickPMT(QCPAxis *axis, QCPAxis::SelectablePart part);
    void axisLabelDoubleClickHead(QCPAxis *axis, QCPAxis::SelectablePart part);
    void legendDoubleClickPMT(QCPLegend *legend, QCPAbstractLegendItem *item);
    void legendDoubleClickHead(QCPLegend *legend, QCPAbstractLegendItem *item);
    void removeSelectedGraphPMT();
    void removeSelectedGraphHead();
    void removeAllGraphsPMT();
    void removeAllGraphsHead();
    void contextMenuRequestPMT(QPoint pos);
    void contextMenuRequestHead(QPoint pos);
    void moveLegendPMT();
    void moveLegendHead();
    void mousePressPMT();
    void mousePressHead();
    void mouseWheelPMT();
    void mouseWheelHead();
    void selectionChangedPMT();
    void selectionChangedHead();
    void resetGraphZoomPMT();
    void resetGraphZoomHead();
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);

    /* Threads */
    void writeRatesToLog(int index, int rate_low, int rate_med, int rate_high);
    void writeTempToLog(int index, double min, double med, double max);
    void getLogErrorThread();
    void getCalibErrorThread();
    void getMCAErrorThread();
    void receivedElapsedTimeString(QString etime_string);
    void receivedHitsMCA(QVector<double> hits, int channels, QString pmt_head, int index, bool mode);
    void receivedHitsCalib(QVector<double> hits, int channels, QString pmt_head, int index, bool mode);
    void receivedValuesMCA(long long time, int hv_pmt, int offset, int var, bool mode);
    void clearSpecPMTsGraphs();
    void clearSpecCalibGraphs();
    void clearSpecHeadsGraphs();
    void connectPortArpet();


    /* Slots de sincronización en el entorno gráfico */
    void setHeadMode(int index, string tab);
    void setAdvanceCoinMode(int index);
    void setHeadModeConfig(int index);
    void setHeadModeGraph(int index);
    void setAdquireMode(int index);
    void setTabMode(int index);
    void setTabHead(int index);
    void setTabLog(int index);
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
    void on_comboBox_head_select_config_currentIndexChanged(const QString &arg1);
    void on_comboBox_adquire_mode_coin_currentIndexChanged(int index);

    /* Buttons */
    void on_pushButton_init_configure_clicked();
    void on_pushButton_triple_ventana_clicked();
    void on_pushButton_hv_clicked();
    void on_pushButton_energia_clicked();
    void on_pushButton_posicion_X_clicked();
    void on_pushButton_posicion_Y_clicked();
    void on_pushButton_obtener_ini_clicked();
    void on_pushButton_tiempos_cabezal_clicked();
    void on_pushButton_initialize_clicked();
    void on_pushButton_configure_clicked();
    void on_pushButton_hv_set_clicked();
    void on_pushButton_hv_on_clicked();
    void on_pushButton_hv_off_clicked();
    void on_pushButton_hv_estado_clicked();
    void on_pushButton_adquirir_toggled(bool checked);
    void on_pushButton_select_pmt_clicked();
    void on_pushButton_hv_configure_clicked();
    void on_pushButton_l_5_clicked();
    void on_pushButton_l_10_clicked();
    void on_pushButton_l_50_clicked();
    void on_pushButton_p_5_clicked();
    void on_pushButton_p_10_clicked();
    void on_pushButton_p_50_clicked();
    void on_pushButton_reset_clicked();
    void on_pushButton_arpet_on_clicked();
    void on_pushButton_arpet_off_clicked();
    void on_actionPreferencias_triggered();
    void on_pushButton_send_terminal_clicked();
    void on_pushButton_flush_terminal_clicked();
    void on_pushButton_clear_terminal_clicked();
    void on_pushButton_stream_configure_mca_terminal_clicked();
    void on_pushButton_stream_configure_psoc_terminal_clicked();
    void on_pushButton_logguer_toggled(bool checked);

    /* AutoCalib */
//    void on_pushButton_clicked();
    void on_pushButton_toggled(bool checked);
    void on_pushButton_triple_ventana_2_clicked();
    void on_pushButton_triple_ventana_3_clicked();
    void on_pushButton_triple_ventana_4_clicked();
    void on_pushButton_triple_ventana_6_clicked();
    void on_pushButton_triple_ventana_7_clicked();
    void on_pushButton_triple_ventana_5_clicked();
    void on_pushButton_triple_ventana_8_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_triple_ventana_9_clicked();
    void on_pushButton_triple_ventana_13_clicked();
    void on_pushButton_triple_ventana_10_clicked();
    void on_pushButton_triple_ventana_11_clicked();
    void on_pushButton_triple_ventana_16_clicked();
    void on_pushButton_triple_ventana_15_clicked();
    void on_pushButton_triple_ventana_12_clicked();
    void on_pushButton_triple_ventana_14_clicked();
    void on_pushButton_3_clicked();

    /* Reconstrucción */
    void on_pushButton_5_clicked();
    void on_pushButton_APIRL_PATH_clicked();
    void on_pushButton_INTERFILES_clicked();
    void on_pushButton_arch_recon_clicked();
    void on_pushButton_Est_ini_clicked();
    void on_pushButton_Arch_sens_clicked();
    void on_pushButton_Arch_count_skimming_clicked();
    void on_pushButton_INTERFILES_2_clicked();
    void on_pushButton_INTERFILES_3_clicked();
    void on_checkBox_MLEM_clicked(bool checked);
    void on_checkBox_Backprojection_clicked(bool checked);
    void on_pushButton_6_clicked();
    void on_comboBox_head_mode_select_config_currentIndexChanged(int index);

    /* CUIPET */
    void on_pushButton_cuipet_aqd_file_open_clicked();

    /* Buttons de prueba/testing */

    void on_parser_coincidencia_clicked();

    void on_pushButton_select_pmt_2_clicked();

    void on_pushButton_tiempos_cabezal_2_clicked();

    void on_pushButton_log_filename_clicked();

    void on_comboBox_head_mode_select_graph_2_currentIndexChanged(int index);

    void on_pushButton_Log_clicked();

    void on_checkBox_temp_log_toggled(bool checked);

    void on_checkBox_rate_log_toggled(bool checked);


private:
    void connectSlots();
    QString openConfigurationFile();
    QString openLogFile();
    QString openDirectory();
    bool copyRecursively(const QString &srcFilePath,const QString &tgtFilePath);
    void getPaths();
    int parseConfigurationFile(bool mode, QString head="");
    QStringList availablePortsName();
    QList<int> getCheckedHeads();
    QString port_name;
    string getLocalDateAndTime();
    void writeFooterAndHeaderDebug(bool header);
    QString getLogFileName(QString main="");
    void writeLogFile(QString log_text, QString main="");
    void writeDebugToStdOutLogFile(QString main="");
    int writePreferencesFile(QString pref, QString filename, bool force=false);
    void getElapsedTime();
    void getHeadStatus(int head_index);
    QString getHead(string tab);
    string initHead(int head);
    string initSP3(int head);
    void setCalibrationTables(int head);
    void setInitialConfigurations();
    void setPreferencesConfiguration();
    void getPreferencesSettingsFile();
    void setPreferencesSettingsFile(QString folder, QString variable, QString value);
    void setLabelState(bool state, QLabel *label, bool error=false);
    void setTextBrowserState(bool state, QTextBrowser *tbro);
    void setButtonState(bool state, QPushButton * button, bool disable=false);
    void setButtonAdquireState(bool state, bool disable=false);
    void setButtonCalibState(bool state, bool disable=false);
    void setButtonConnectState(bool state, bool disable=false);
    void setButtonLoggerState(bool state, bool disable=false);
    string readString(char delimeter='\r');
    string readBufferString(int buffer_size);
    size_t sendString(string msg, string end);
    void manageHeadCheckBox(string tab, bool show);
    void manageHeadComboBox(string tab, bool show);
    QString getMCA(string head, string function, bool multimode, int channels, string pmt="0");
    QString getMultiMCA(QString head);
    QString getHeadMCA(QString head);
    void setMCAEDataStream(string head, string function, string pmt, string mca_function, int bytes_mca=0, string hv_value="");
    void setMCAEDataStream(string tab, string function, string pmt, string mca_function, double time);
    void setMCAEDataStream(string tab, string calib_function, QVector<double> table);
    void setMCAEDataStream(string coin_function, string data_one, string data_two, bool time);
    void setMCAEDataStream(string head, bool coin=false);
    int setPSOCDataStream(string head, string size_received, string function, QString psoc_value="");
    void setPMTCustomPlotEnvironment(QList<QString> qlist);
    void setPMTCalibCustomPlotEnvironment(QList<int> qlist);
    void setHeadCustomPlotEnvironment();
    QVector<int> getCustomPlotParameters();
    void SetQCustomPlotConfiguration(QCustomPlot *graph, int channels);
    void SetQCustomPlotSlots(string title_pmt_str="", string title_head_str="");
    QString setHV(string head, string hv_value, string pmt);
    QString setHV(string head, string hv_value);
    QString setCalibTable(string head, string function, QVector<double> table, string msg_compare);
    QString setTime(string head, double time_value, string pmt);
    int getPMT(QLineEdit *line_edit);
    QString getPSOCAlta(QLineEdit *line_edit);
    string getHVValue(QLineEdit *line_edit, int value=0);
    void resetHitsValues();
    bool resetHeads();
    bool resetPMTs(bool centroide=false);
    void setQListElements();
    void drawTemperatureBoard();
    void setTemperatureBoard(double temp, QLabel *label_pmt, int pmt);
    void clearTemperatureBoard();
    temp_code getTemperatureCode(double temperature);
    QVector<double> getValuesFromFiles(QString filename, bool hv=false);
    QStringList getLogFromFiles(QString filename, QRegExp rx, string parser);
    void setCoincidenceModeDataStream(string stream);
    string getCoincidenceAdvanceModeDataStream();
    void initCoincidenceMode();
    void setCoincidenceModeWindowTime();
    void setCalibrationMode(QString head);
    void getARPETStatus();
    void showMCAEStreamDebugMode(string msg);

    /* Area de prueba/testing */

signals:
    void sendAbortCommand(bool abort);
    void sendCalibAbortCommand(bool abort);
    void sendAbortMCAECommand(bool abort);
    void ToPushButtonAdquirir(bool toggle);
    void ToPushButtonLogger(bool toggle);
    void ToPushButtonCalib(bool toggle);

private:
    Ui::MainWindow *ui;
    SetPreferences *pref;
    SetPMTs *pmt_select;
    SetPMTs *pmt_select_autocalib;
    shared_ptr<MCAE> arpet;
    shared_ptr<AutoCalib> calibrador;
    shared_ptr<Reconstructor> recon_externa;
    QMutex mMutex;
    QThread *thread;
    Thread *worker;
    QThread *etime_th;
    Thread *etime_wr;
    QThread *mcae_th;
    Thread *mcae_wr;
    AutoCalibThread *calib_wr;
    QThread *calib_th;
    bool is_abort_mcae, is_abort_log, is_abort_calib;
    QString initfile, root_config_path, root_calib_path, preferencesdir, preferencesfile;
    QList<QComboBox*> heads_coin_table;
    QList<QLabel*> pmt_label_table;
    QList<QLabel*> head_status_table;
    QList<QLabel*> pmt_status_table;
    QList<QLabel*> hv_status_table;
    QList<QLabel*> calib_status_table;
    QList<QString> pmt_selected_list;
    QList<QString> pmt_selected_list_autocalib;
    QVector<string> logTemp_values,logRate_values;
  //  QList<QPushButton*> pmt_button_table;
    int adquire_mode;
    bool debug, init, log, stdout_mode;
    QString coefenerg, coefT,coefTInter, hvtable, coefx, coefy, coefest, logTemp, logRate,root_log;
    QVector<double> hvtable_values, coefenerg_values, coefT_values,coefTInter_values, coefx_values, coefy_values, coefest_values;
    QVector< QVector<int> > qcp_pmt_parameters, qcp_head_parameters, qcp_pmt_calib_parameters;
    int  AT, LowLimit, Target;
    QVector<double> channels_ui;
    int pmt_ui_current, pmt_ui_previous;
    int headIndex;

    /* Area de prueba/testing */


public:
    /**
     * @brief getPreferencesDir
     */
    QString getPreferencesDir() const { return preferencesdir; }
    /**
     * @brief setInitFileConfigPath
     * @param file
     */
    void setInitFileConfigPath(QString file) { initfile = file; }
    /**
     * @brief setCalibDirectoryPath
     * @param path
     */
    void setCalibDirectoryPath(QString path) { root_calib_path = path; }
    /**
     * @brief setDebugMode
     *
     * Configura el valor de _debug_ a partir del menú preferencias.
     *
     * @param mode
     */
    void setDebugMode(bool mode) { debug = mode; }
    /**
     * @brief setLogMode
     *
     * Configura el valor de _log_ a partir del menú preferencias.
     *
     * @param mode
     */
    void setLogMode(bool mode) { log = mode; }
    /**
     * @brief setStdOutDebugMode
     *
     * Configura el valor de _stdout\_mode_ a partir del menú preferencias.
     *
     * @param mode
     */
    void setStdOutDebugMode(bool mode) { stdout_mode = mode; }
    /**
     * @brief setPMTSelectedList
     *
     * Configura la lista de PMTs a partir de la selección realizada en el _tab_ "mca".
     *
     * @param list
     */
    void setPMTSelectedList(QList<QString> list) { pmt_selected_list = list; }
    void setPMTSelectedListAutocalib(QList<QString> list) { pmt_selected_list = list; }
    /**
     * @brief setIsAbortMCAEFlag
     *
     * Configura la bandera de aborto que habilita a enviar las señales para abortar
     * el proceso de adquisición MCAE
     *
     * @param flag
     */
    void setIsAbortMCAEFlag(bool flag) { is_abort_mcae = flag; }
    void setIsAbortCalibFlag(bool flag) { is_abort_calib = flag; }
    /**
     * @brief setIsAbortLogFlag
     *
     * Configura la bandera de aborto que habilita a enviar las señales para abortar
     * el proceso log
     *
     * @param flag
     */
    void setIsAbortLogFlag(bool flag) { is_abort_log = flag; }
    /**
     * @brief getPMTSelectedList
     *
     * Devuelve la lista de PMTs seleccionados configurada en el menú de selección de PMTs.
     *
     * @return pmt_selected_list
     */
    QList<QString> getPMTSelectedList() { return pmt_selected_list; }
    /**
     * @brief setHitsInit
     *
     * Configura el estado del vector de cuentas.
     * @deprecated: Verificar este método.
     *
     * @param status
     */
    void setHitsInit(bool status) { init = status;}
    /**
     * @brief getAT
     *
     * Obtiene el valor de alta tensión del cabezal seleccionado
     *
     */
    int getAT() const {return AT;}
    /**
     * @brief getLowLimit
     *
     * Obtiene el límite de ventana inferior para el cabezal seleccionado
     *
     */
    int getLowLimit() const {return LowLimit;}
    /**
     * @brief getTarget
     *
     * Obtiene el canal de _target_ donde se realizó la calibración. Se utiliza en autocalibración.
     *
     */
    int getTarget() const {return Target;}
};

#endif // MAINWINDOW_H
