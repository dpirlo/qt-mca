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
#include "../ui/Validate.h"
#include "apMCAE.hpp"
//#include "apAutoCalib.hpp"
#include "apRecon.hpp"
#include "apThread.hpp"
#include "apAutoCalibThread.hpp"
#include <QThread>
#include <cstdio>
#include <QString>
#include <QPixmap>
#include <QTextStream>
#include <inc/QRoundProgressBar.h>

#define MONOHEAD 0
#define MULTIHEAD 1
#define ALLHEADS 2
#define PMT 0
#define CABEZAL 1
#define TEMPERATURE 2
#define ALMOHADA 3
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
#define Tab0 1
#define Tab1 1
#define Tab2 2
#define Tab3 3
#define Tab4 4
#define Tab5 5
#define Tab6 6
#define Tab7 7
#define Tab8 8
#define Tab9 9
#define WAIT_MICROSECONDS 1000000
#define Cab1 "/dev/UART_Cab1"
#define Cab2 "/dev/UART_Cab2"
#define Cab3 "/dev/UART_Cab3"
#define Cab4 "/dev/UART_Cab4"
#define Cab5 "/dev/UART_Cab5"
#define Cab6 "/dev/UART_Cab6"
#define Cab  "/dev/UART_Cab"
#define Coin "/dev/UART_Coin"

#define icon_notok      ":/qss_icons/rc/ic_cancel.png"
#define icon_ok         ":/qss_icons/rc/ic_check_circle.png"
#define icon_loading    ":/qss_icons/rc/cargando2.gif"
#define CANTIDAD_ELEMENTOS_PLANAR 97
#define CANTIDAD_ELEMENTOS_COINCIDENCIA 1

#define MAX_MB_CALIB 102//4



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
    void resetHitsValues(QString head);

    /* Area de prueba/testing */

private slots:
    /* Slots de sincronización para QCustomPlot */
    void addGraph(QVector<double> hits,  QCustomPlot *graph, int channels, QString graph_legend, QVector<int> param);
    void addGraph_Calib(QVector<double> hits,  QCustomPlot *graph, int channels, QString graph_legend, QVector<int> param);
    void addGraph_Tiempos(QVector<double> hits,  QCustomPlot *graph, int channels, QString graph_legend, QVector<int> param, bool local=false);
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
    void checkStatusFPGA(bool status);
    void checkStatusAdq(bool status);
    void recieveresetheads();
    void checkStatusMoveToServer(bool status);

    /* Threads */
    void writeRatesToLog(int index, int rate_low, int rate_med, int rate_high);
    void writeRatesCoinToLog(int rate_uno_tres, int rate_uno_cuatro, int rate_uno_cinco,int rate_dos_cuatro,int rate_dos_cinco,int rate_dos_seis,int rate_tres_cinco,int rate_tres_seis,int rate_cuatro_seis);
    void writeTempToLog(int index, double min, double med, double max);
    void writeOffSetToLog(int index,int *offsets);
    void recievedPicosLog(struct Pico_espectro Pico ,int index);
    void recievedPicosMCA(struct Pico_espectro Pico ,int index);
    void getLogErrorThread();
    void getCalibErrorThread();
    void getMCAErrorThread();
    void receivedElapsedTimeString(QString etime_string);
    void receivedHitsMCA(QVector<double> hits, int channels, QString pmt_head, int index, bool mode);
    void receivedHitsCalib(QVector<double> hits, int channels, QString pmt_head, int index, bool mode);
    void receivedHitsTiempos(QVector<double> hits, int channels, QString pmt_head, int index, bool mode);

    void receivedValuesMCA(long long time, int hv_pmt, int offset, int var, bool mode);
    void receivedValuesMCACalib(int umbral, int pico, int FWHM);
    void clearSpecPMTsGraphs();
    void clearSpecCalibGraphs();
    void clearSpecHeadsGraphs();
    void connectPortArpet();
    void OffButtonCalib();
    void AutocalibReady(bool state);
    void AutoAdqReady(bool state);
    void CopyAdqReady(bool state);
    void recievedSaturated(int Cabezal, double *Saturados);
    int loadCalibrationTables(QString head);
    void CargoTemaOscuro();
    void TimerUpdate();

    /* Slots de sincronización en el entorno gráfico */
    void setHeadMode(int index, string tab);
    void setAdvanceCoinMode(int index);
    void setAdquireMode(int index);
    void setTabMode(int index);
    void setTabLog(int index);
    void syncHeadComboBoxToMCA(int index);
    void syncCheckBoxHead1ToMCA(bool check);
    void syncCheckBoxHead2ToMCA(bool check);
    void syncCheckBoxHead3ToMCA(bool check);
    void syncCheckBoxHead4ToMCA(bool check);
    void syncCheckBoxHead5ToMCA(bool check);
    void syncCheckBoxHead6ToMCA(bool check);
    void syncHeadComboBoxToConfig(int index);
    void syncCheckBoxHead1ToConfig(bool check);
    void syncCheckBoxHead2ToConfig(bool check);
    void syncCheckBoxHead3ToConfig(bool check);
    void syncCheckBoxHead4ToConfig(bool check);
    void syncCheckBoxHead5ToConfig(bool check);
    void syncCheckBoxHead6ToConfig(bool check);
    //void on_comboBox_head_select_config_currentIndexChanged(const QString &arg1);
    void on_comboBox_adquire_mode_coin_currentIndexChanged(int index);

    /*Calibrar Cabezal Completo*/
    void on_pb_Calibrar_Cabezal_clicked();

    /* FPGA */
    void on_checkBox_FPGA_2_clicked(bool checked);
    void on_pushButton_FPGA_4_clicked();

    /* Buttons */
    void on_pushButton_initialize_clicked();
    void on_pushButton_configure_clicked();
    void on_pushButton_hv_set_clicked();
    void on_pushButton_hv_on_clicked();
    void on_pushButton_hv_off_clicked();
    void on_pushButton_hv_estado_clicked();
    void on_pushButton_select_pmt_clicked();
    void on_pushButton_hv_configure_clicked();
    void on_pushButton_l_5_clicked();
    void on_pushButton_l_10_clicked();
    void on_pushButton_l_50_clicked();
    void on_pushButton_p_5_clicked();
    void on_pushButton_p_10_clicked();
    void on_pushButton_p_50_clicked();
    void on_pushButton_reset_clicked();
    void on_actionPreferencias_triggered();
    void on_pushButton_send_terminal_clicked();
    void on_pushButton_flush_terminal_clicked();
    void on_pushButton_clear_terminal_clicked();
    void on_pushButton_stream_configure_mca_terminal_clicked();
    void on_pushButton_stream_configure_psoc_terminal_clicked();
    void on_pushButton_logguer_toggled(bool checked);

    /* AutoCalib */
    void on_pb_Autocalib_toggled(bool checked);
    void on_pb_Autocalib_Tiempos_toggled(bool checked);
    void on_pb_Autocalib_Tiempos_Debug_clicked();
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
    void on_checkBox_MLEM_clicked();
    void on_checkBox_Backprojection_clicked();
    void on_pushButton_6_clicked();

    /* CUIPET */
    void on_pushButton_aqd_file_open_clicked();

    /* Buttons de prueba/testing */

    void on_parser_coincidencia_clicked();

    void on_pushButton_select_pmt_2_clicked();

    void on_comboBox_head_mode_select_graph_2_currentIndexChanged(int index);

    void on_checkBox_temp_log_toggled(bool checked);

    void on_checkBox_rate_log_toggled(bool checked);

    void on_pushButton_p_51_clicked();

    void on_RATECAB1_clicked();

    void on_RATECAB2_clicked();

    void on_RATECAB3_clicked();

    void on_RATECAB4_clicked();

    void on_RATECAB5_clicked();

    void on_RATECAB6_clicked();

    void on_pushButton_adquirir_clicked();

    void on_calendarWidget_selectionChanged();

    void on_ChechBox_Pico_toggled(bool checked);

    void on_checkBox_Rate_Coin_toggled(bool checked);

    void updateCaption();

    void on_tabWidget_mca_currentChanged(int index);

    void on_pushButton_Encendido_1_toggled(bool checked);

    void on_pushButton_Encendido_2_toggled(bool checked);

    void on_pushButton_Encendido_3_toggled(bool checked);

    void on_pushButton_Encendido_4_toggled(bool checked);

    void on_pushButton_Encendido_5_toggled(bool checked);

    void on_pushButton_Encendido_6_toggled(bool checked);

    void on_pushButton_On_Off_Cabs_toggled(bool checked);

    void on_pbAdquirir_toggled(bool checked);

    void on_pushButton_FPGA_1_clicked();

    void on_pushButton_FPGA_3_clicked();

    void on_comboBox_FPGA_Cab_currentIndexChanged(int index);

    void on_cb_Path_alternativo_adq_toggled(bool checked);

    void on_comboBox_aqd_mode_currentIndexChanged(const QString &arg1);

    void on_pushButton_FPGA_2_clicked();

    void on_comboBox_FPGA_DISP_activated(int index);

    void on_pb_Autocalib_Tiempos_Reset_clicked();

private:
    QString openConfigurationFile();
    QString openLogFile();
    QString openDirectory();
    QString getLogFileName(QString main="");
    QString getHead(string tab);
    QString getMCA(string head, string function, bool multimode, int channels, string pmt="0");
    QString getMultiMCA(QString head);
    QString getHeadMCA(QString head);
    QString setHV(string head, string hv_value, string pmt);
    QString setHV(string head, string hv_value);
    QString setCalibTable(string head, string function, QVector<double> table, string msg_compare);
    QString setTime(string head, double time_value, string pmt);
    QString getPSOCAlta(QLineEdit *line_edit);
    bool fileExists(QString path);
    bool resetHeads();
    bool resetHead(QString Cabezal);
    bool resetPMTs();
    bool copyRecursively(const QString &srcFilePath,const QString &tgtFilePath);
    struct Pico_espectro Buscar_Pico(double* Canales, int num_canales);
    size_t sendString(string msg, string end);
    string getEstadoCabezal(int head);
    string getCoincidenceAdvanceModeDataStream();
    string getHVValue(QLineEdit *line_edit, int value=0);
    string readString(char delimeter='\r');
    string readBufferString(int buffer_size);
    string initHead(int head);
    string initSP3(int head);
    string getLocalDateAndTime();
    int getPMT(QLineEdit *line_edit);
    int parseConfigurationFile(bool mode, QString head="");
    int writePreferencesFile(QString pref, QString filename, bool force=false);
    int setPSOCDataStream(string head, string size_received, string function, QString psoc_value="");
    temp_code getTemperatureCode(double temperature);
    QVector<int> getCustomPlotParameters();
    QVector<double> getValuesFromFiles(QString filename, bool hv=false);
    QStringList getLogFromFiles(QString filename, QRegExp rx, string parser);
    QStringList availablePortsName();
    QList<int> getCheckedHeads();
    void writeFooterAndHeaderDebug(bool header);
    void writeLogFile(QString log_text, QString main="");
    void writeDebugToStdOutLogFile(QString main="");
    void SetQCustomPlotConfiguration(QCustomPlot *graph, int channels);
    void SetQCustomPlotSlots(string title_pmt_str="", string title_head_str="");
    void getElapsedTime();
    void getHeadStatus(int head_index);
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

    void setButtonCalibTiemposState(bool state, bool disable=false);

    void setButtonLoggerState(bool state, bool disable=false);
    void manageHeadCheckBox(string tab, bool show);
    void manageHeadComboBox(string tab);
    void setMCAEDataStream(string head, string function, string pmt, string mca_function, int bytes_mca=0, string hv_value="");
    void setMCAEDataStream(string tab, string function, string pmt, string mca_function, double time);
    void setMCAEDataStream(string tab, string calib_function, QVector<double> table);
    void setMCAEDataStream(string coin_function, string data_one, string data_two, bool time);
    void setMCAEDataStream(string head, bool coin=false);

    void setCoincidenceModeDataStream(string stream);
    void initCoincidenceMode();
    void setCoincidenceModeWindowTime(bool calib=false);
    void setCalibrationMode(QString head);
    void getARPETStatus();
    void showMCAEStreamDebugMode(string msg);
    void LoadHVPMT(int head);
    void setTimeModeCoin(int mode,QString head="");
    void setTimeModeCoin(int mode,bool cero=false,QString head="");
    void Cabezal_On_Off(int Cabezal, bool estado);
    void Cabezales_On_Off(bool estado);
    void UncheckHeads(void);
    void setQListElements();
    void drawTemperatureBoard();
    void drawAlmohada();
    void setTemperatureBoard(double temp, QLabel *label_pmt, int pmt);
    void clearTemperatureBoard();
    void connectSlots();
    void getPaths();
    void setPMTCustomPlotEnvironment(QList<QString> qlist);
    void setPMTCalibCustomPlotEnvironment(QList<int> qlist);
    void setHeadCustomPlotEnvironment();

signals:
    void sendAbortCommand(bool abort);
    void sendCalibAbortCommand(bool abort);
    void sendAbortMCAECommand(bool abort);
    void ToPushButtonAdquirir(bool toggle);
    void ToPushButtonLogger(bool toggle);

private:
    Ui::MainWindow *ui;
    SetPreferences *pref;
    Validate *validate_autocalib;
    SetPMTs *pmt_select;
    SetPMTs *pmt_select_autocalib;
    shared_ptr<MCAE> arpet;
    shared_ptr<AutoCalib> calibrador;
    shared_ptr<Reconstructor> recon_externa;
    QMutex mMutex, bMutex, Mutex_fpga,Mutex_adq,Mutex_copy;
    QThread *thread;
    Thread *worker;
    QThread *thread_fpga;
    Thread *worker_fpga;
    QThread *thread_adq;
    Thread *worker_adq;
    QThread *thread_copy;
    Thread *worker_copy;
    QThread *etime_th;
    Thread *etime_wr;
    QThread *mcae_th;
    Thread *mcae_wr;
    AutoCalibThread *calib_wr;
    QThread *calib_th;
    bool is_abort_mcae, is_abort_log, is_abort_calib;
    QString initfile, root_config_path, root_calib_path,root_log_path, preferencesdir, preferencesfile;
    QList<QComboBox*> heads_coin_table;
    QList<QLabel*> pmt_label_table;
    QList<QLabel*> head_status_table;
    QList<QLabel*> pmt_status_table;
    QList<QLabel*> hv_status_table;
    QList<QLabel*> calib_status_table;
    QList<QString> pmt_selected_list;
    QList<QString> pmt_selected_list_autocalib;
    QVector<string> logTemp_values,logRate_values;
    int adquire_mode;
    bool debug;
    bool init;
    bool log;
    bool stdout_mode;
    QVector<int> Estado_Cabezales;
    QString coefenerg, coefT,coefTInter, hvtable, coefx, coefy, coefest, logTemp, logRate,root_log;
    QVector<double> coefenerg_values, coefT_values,coefTInter_values, coefx_values, coefy_values, coefest_values;
    QVector< QVector<double> > Matrix_coefenerg_values;
    QVector< QVector<double> > hvtable_values;
    QVector< QVector<double> > Matrix_coefx_values;
    QVector< QVector<double> > Matrix_coefy_values;
    QVector< QVector<double> > Matrix_coefT_values;
    QVector< QVector<double> > Matrix_coefest_values;

    /* Programcion JTAG */
    const QString PMT_posJTAG[48]= {"7","5","3","1","89","91","93","95","15","13","11","9","81","83","85","87","23","21","19","17","73","75","77","79","31","29","27","25","65","67","69","71","39","37","35","33","57","59","61","63","47","45","43","41","49","51","53","55"} ;
    QString path_Planar_bit;
    QString path_SP3_bit;
    QString path_Coin_bit;
    QString path_adq_Calib;
    QString path_adq_Coin;
    QString model_planar;
    QString model_SP3;
    QString model_SP3_MEM;
    QString model_coin;
    QString device_planar;
    QString device_SP3;
    QString device_SP3_MEM;
    QString device_coin;
    QString name_Planar_bit;
    QString name_Coin_bit;
    QString name_SP3_bit;
    QString port_name;

    QVector< QVector<int> > qcp_pmt_parameters, qcp_head_parameters, qcp_pmt_calib_parameters;
    int  AT, LowLimit[6], Target;
    QVector<double> channels_ui;
    int pmt_ui_current, pmt_ui_previous;
    int headIndex;

    QTimer *timer = new QTimer(0);
    int  ogl_flag;
    int cant_archivos=1;
    int cant_archivos_copiados = 0;
    bool finish_adquirir=false;
    QStringList Mensaje_Grabar_FPGA(int modo);
    QList<QString> array_PMT;
    int offset_MEM  = 0;

    QMovie *movie_cargando = new QMovie(icon_loading);
    QString nombre_archivo_adq;
    QString size_archivo_adq;
    bool adq_running = false;
    bool copying= false;

    QStringList commands_calib;

    QString ruta_log_adquisicion="";

    QGraphicsScene *scene;
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
     * Obtiene el límite de ventana inferior para el cabezal seleccionado
     * @param head
     * @return
     */
    int getLowLimit(int head) const {return LowLimit[head-1];}
    /**
     * @brief getTarget
     *
     * Obtiene el canal de _target_ donde se realizó la calibración. Se utiliza en autocalibración.
     *
     */
    int getTarget() const {return Target;}
    void setCommandsAdquire();
};

#endif // MAINWINDOW_H
