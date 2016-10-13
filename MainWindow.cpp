#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    TimeOut(1000),
    bytes_int(CHANNELS*6+16),
    channels_ui(CHANNELS),
    hits_ui(CHANNELS),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->maximumSize());
    arpet=shared_ptr<MCAE>(new MCAE(TimeOut));
    ui->comboBox_port->addItems(availablePortsName());
    manageHeadCheckBox("config",false);
    manageHeadCheckBox("mca",false);
    getPMTLabelNames();    
    setAdquireMode(ui->comboBox_adquire_mode->currentIndex());
    ui->lineEdit_pmt->setValidator( new QIntValidator(1, PMTs, this) );
    ui->lineEdit_hv_value->setValidator( new QIntValidator(0, MAX_HV_VALUE, this) );
    ui->lineEdit_alta->setValidator( new QIntValidator(1, MAX_HIGH_HV_VOLTAGE, this) );
    ui->tabWidget_mca->setCurrentWidget(ui->tab_esp_2);
    ui->tabWidget_general->setCurrentWidget(ui->config);
    resetHitsValues();    
}

MainWindow::~MainWindow()
{
    arpet->portDisconnect();
    delete ui;
}

void MainWindow::checkCombosStatus()
{
     QObject::connect(ui->comboBox_head_mode_select_graph ,SIGNAL(currentIndexChanged (int)),this,SLOT(setHeadModeGraph(int)));
     QObject::connect(ui->comboBox_head_mode_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(setHeadModeConfig(int)));
     QObject::connect(ui->comboBox_adquire_mode ,SIGNAL(currentIndexChanged (int)),this,SLOT(setAdquireMode(int)));     
     QObject::connect(ui->comboBox_head_mode_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(syncHeadModeComboBoxToMCA(int)));
     QObject::connect(ui->comboBox_head_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(syncHeadComboBoxToMCA(int)));
     QObject::connect(ui->comboBox_head_mode_select_graph ,SIGNAL(currentIndexChanged (int)),this,SLOT(syncHeadModeComboBoxToConfig(int)));
     QObject::connect(ui->comboBox_head_select_graph ,SIGNAL(currentIndexChanged (int)),this,SLOT(syncHeadComboBoxToConfig(int)));
     QObject::connect(ui->checkBox_mca_1 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead1ToConfig(bool)));
     QObject::connect(ui->checkBox_mca_2 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead2ToConfig(bool)));
     QObject::connect(ui->checkBox_mca_3 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead3ToConfig(bool)));
     QObject::connect(ui->checkBox_mca_4 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead4ToConfig(bool)));
     QObject::connect(ui->checkBox_mca_5 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead5ToConfig(bool)));
     QObject::connect(ui->checkBox_mca_6 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead6ToConfig(bool)));
     QObject::connect(ui->checkBox_c_1 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead1ToMCA(bool)));
     QObject::connect(ui->checkBox_c_2 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead2ToMCA(bool)));
     QObject::connect(ui->checkBox_c_3 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead3ToMCA(bool)));
     QObject::connect(ui->checkBox_c_4 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead4ToMCA(bool)));
     QObject::connect(ui->checkBox_c_5 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead5ToMCA(bool)));
     QObject::connect(ui->checkBox_c_6 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead6ToMCA(bool)));
}

void MainWindow::getPMTLabelNames()
{
    pmt_label_table.push_back(ui->label_pmt_01);
    pmt_label_table.push_back(ui->label_pmt_02);
    pmt_label_table.push_back(ui->label_pmt_03);
    pmt_label_table.push_back(ui->label_pmt_04);
    pmt_label_table.push_back(ui->label_pmt_05);
    pmt_label_table.push_back(ui->label_pmt_06);
    pmt_label_table.push_back(ui->label_pmt_07);
    pmt_label_table.push_back(ui->label_pmt_08);
    pmt_label_table.push_back(ui->label_pmt_09);
    pmt_label_table.push_back(ui->label_pmt_10);
    pmt_label_table.push_back(ui->label_pmt_11);
    pmt_label_table.push_back(ui->label_pmt_12);
    pmt_label_table.push_back(ui->label_pmt_13);
    pmt_label_table.push_back(ui->label_pmt_14);
    pmt_label_table.push_back(ui->label_pmt_15);
    pmt_label_table.push_back(ui->label_pmt_16);
    pmt_label_table.push_back(ui->label_pmt_17);
    pmt_label_table.push_back(ui->label_pmt_18);
    pmt_label_table.push_back(ui->label_pmt_19);
    pmt_label_table.push_back(ui->label_pmt_20);
    pmt_label_table.push_back(ui->label_pmt_21);
    pmt_label_table.push_back(ui->label_pmt_22);
    pmt_label_table.push_back(ui->label_pmt_23);
    pmt_label_table.push_back(ui->label_pmt_24);
    pmt_label_table.push_back(ui->label_pmt_25);
    pmt_label_table.push_back(ui->label_pmt_26);
    pmt_label_table.push_back(ui->label_pmt_27);
    pmt_label_table.push_back(ui->label_pmt_28);
    pmt_label_table.push_back(ui->label_pmt_29);
    pmt_label_table.push_back(ui->label_pmt_30);
    pmt_label_table.push_back(ui->label_pmt_31);
    pmt_label_table.push_back(ui->label_pmt_32);
    pmt_label_table.push_back(ui->label_pmt_33);
    pmt_label_table.push_back(ui->label_pmt_34);
    pmt_label_table.push_back(ui->label_pmt_35);
    pmt_label_table.push_back(ui->label_pmt_36);
    pmt_label_table.push_back(ui->label_pmt_37);
    pmt_label_table.push_back(ui->label_pmt_38);
    pmt_label_table.push_back(ui->label_pmt_39);
    pmt_label_table.push_back(ui->label_pmt_40);
    pmt_label_table.push_back(ui->label_pmt_41);
    pmt_label_table.push_back(ui->label_pmt_42);
    pmt_label_table.push_back(ui->label_pmt_43);
    pmt_label_table.push_back(ui->label_pmt_44);
    pmt_label_table.push_back(ui->label_pmt_45);
    pmt_label_table.push_back(ui->label_pmt_46);
    pmt_label_table.push_back(ui->label_pmt_47);
    pmt_label_table.push_back(ui->label_pmt_48);

    pmt_status_table.push_back(ui->label_pmt_estado_1);
    pmt_status_table.push_back(ui->label_pmt_estado_2);
    pmt_status_table.push_back(ui->label_pmt_estado_3);
    pmt_status_table.push_back(ui->label_pmt_estado_4);
    pmt_status_table.push_back(ui->label_pmt_estado_5);
    pmt_status_table.push_back(ui->label_pmt_estado_6);

    hv_status_table.push_back(ui->label_hv_estado_1);
    hv_status_table.push_back(ui->label_hv_estado_2);
    hv_status_table.push_back(ui->label_hv_estado_3);
    hv_status_table.push_back(ui->label_hv_estado_4);
    hv_status_table.push_back(ui->label_hv_estado_5);
    hv_status_table.push_back(ui->label_hv_estado_6);

    head_status_table.push_back(ui->label_cabezal_estado_1);
    head_status_table.push_back(ui->label_cabezal_estado_2);
    head_status_table.push_back(ui->label_cabezal_estado_3);
    head_status_table.push_back(ui->label_cabezal_estado_4);
    head_status_table.push_back(ui->label_cabezal_estado_5);
    head_status_table.push_back(ui->label_cabezal_estado_6);
}

/* Pestaña: "Configuración" */

void MainWindow::on_pushButton_arpet_on_clicked()
{
    string msg;
    try
    {
        SendString(arpet->getAP_ON(),arpet->getEnd_MCA());
        sleep(1);
        SendString(arpet->getAP_ON(),arpet->getEnd_MCA());
        msg = ReadString();
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr((string("Hubo un inconveniente al intentar encender el equipo. Revise la conexión. Error: ")+string(ex.excdesc)).c_str()));
    }
    setLabelState(!arpet->verifyMCAEStream(msg,arpet->getAnsAP_ON()),ui->label_arpet_power_supply);
}

void MainWindow::on_pushButton_arpet_off_clicked()
{
    string msg;
    try
    {
        SendString(arpet->getAP_OFF(),arpet->getEnd_MCA());
        msg = ReadString();
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr((string("Hubo un inconveniente al intentar apagar el equipo. Revise la conexión. Error: ")+string(ex.excdesc)).c_str()));
    }
    setLabelState(arpet->verifyMCAEStream(msg,arpet->getAnsAP_OFF()),ui->label_arpet_power_supply,true);
}

void MainWindow::on_pushButton_triple_ventana_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_triple_ventana->setText(fileName);
}

void MainWindow::on_pushButton_hv_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_hv->setText(fileName);
}

void MainWindow::on_pushButton_energia_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_energia->setText(fileName);
}


void MainWindow::on_pushButton_posicion_X_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_posicion_X->setText(fileName);
}

void MainWindow::on_pushButton_posicion_Y_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_posicion_Y->setText(fileName);
}

void MainWindow::on_pushButton_tiempos_cabezal_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_tiempos_cabezal->setText(fileName);
}

void MainWindow::on_pushButton_obtener_rutas_clicked()
{
    string initfile=".//config_cabs.ini";
    QString filename=QString::fromStdString(initfile);
    parseConfigurationFile(filename);

    ui->textBrowser_triple_ventana->setText(coefest);
    ui->textBrowser_hv->setText(hvtable);
    ui->textBrowser_energia->setText(coefenerg);
    ui->textBrowser_posicion_X->setText(coefx);
    ui->textBrowser_posicion_Y->setText(coefy);
    ui->textBrowser_tiempos_cabezal->setText(coefT);

    ui->lineEdit_alta->setText(QString::number(AT));
    ui->lineEdit_limiteinferior->setText(QString::number(LowLimit));
}

int MainWindow::on_pushButton_conectar_clicked()
{

    if(arpet->isPortOpen())
    {
        ui->pushButton_conectar->setText("Conectar");
        arpet->portDisconnect();
    }
    else
    {
        ui->pushButton_conectar->setText("Desconectar");
        try{
            QString port_name=ui->comboBox_port->currentText();
            arpet->portConnect(port_name.toStdString().c_str());
            QMessageBox::information(this,tr("Información"),tr("Conectado al puerto: ") + port_name);
        }
        catch(boost::system::system_error e)
            {
            QMessageBox::critical(this,tr("Error"),tr("No se puede acceder al puerto serie. Revise la conexión USB. Error: ")+tr(e.what()));;
            return MCAE::FAILED;
        }
    }

    return MCAE::OK;
}

void MainWindow::on_pushButton_head_init_clicked()
{

   int head_index=getHead("config").toInt();   
   /* Incialización del cabezal */
   setMCAEDataStream("config", arpet->getFunCHead(), arpet->getBrCst(), arpet->getInit_MCA());   
   string msg_head;
   try
   {
       SendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
       msg_head = ReadString();
   }
   catch(Exceptions & ex)
   {
       QMessageBox::critical(this,tr("Atención"),tr((string("No se puede/n inicializar el/los cabezal/es seleccionado/s. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
   }
   setLabelState(!arpet->verifyMCAEStream(msg_head,arpet->getAnsHeadInit()),head_status_table[head_index-1]);

   /* Inicialización de las Spartans 3*/
   setMCAEDataStream("config", arpet->getFunCSP3(), arpet->getBrCst(), arpet->getInit_MCA());  
   string msg_pmts;
   try
   {
       SendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
       msg_pmts = ReadString();
   }
   catch(Exceptions & ex)
   {
       QMessageBox::critical(this,tr("Atención"),tr((string("No se pueden inicializar los PMT en el/los cabezal/es seleccionado/s. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
   }
   setLabelState(!arpet->verifyMCAEStream(msg_pmts,arpet->getAnsMultiInit()),pmt_status_table[head_index-1]);

   ui->label_config_init->setText("Recepción del Cabezal: "+QString::fromStdString(msg_head)+"\nRecepción de los PMTs: "+QString::fromStdString(msg_pmts));
}

void MainWindow::on_pushButton_configurar_clicked()
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead("config").toStdString() + arpet->getFunCHead());

    /** TODO:
     * Implementar una función que realice el seteo de los HV a los PMTs en función del cabezal seleccionado
     * Ver de obtener todos los path, de los seis cabezales, solo de un .ini
     */
    cout<<arpet->getHeader_MCAE()<<endl;
}

void MainWindow::on_pushButton_hv_set_clicked()
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead("config").toStdString() + arpet->getFunCHV());

    /** TODO:
     * Seteo de HV a la tensión indicada.
     * El seteo se debe realizar de manera escalonada
     */
    cout<<arpet->getHeader_MCAE()<<endl;

}

void MainWindow::on_pushButton_hv_on_clicked()
{
    /** TODO:
     * Ver el tamaño de la trama
     */

    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead("config").toStdString() + arpet->getFunCHV());
    arpet->setTrama_MCAE(arpet->getHeader_MCAE()+arpet->getHV_ON()+arpet->getEnd_HV());
    cout<<arpet->getTrama_MCAE()<<endl;
}

void MainWindow::on_pushButton_hv_off_clicked()
{
    /** TODO:
     * Ver el tamaño de la trama
     */

    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead("config").toStdString() + arpet->getFunCHV());
    arpet->setTrama_MCAE(arpet->getHeader_MCAE()+arpet->getHV_OFF()+arpet->getEnd_HV());
    cout<<arpet->getTrama_MCAE()<<endl;
}

void MainWindow::on_pushButton_hv_estado_clicked()
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead("config").toStdString()+arpet->getFunCHV());

    /** TODO:
     * Obtener estado de la fuente HV y mostrarlo en el label
     */
    cout<<arpet->getHeader_MCAE()<<endl;
}

void MainWindow::setHeadModeConfig(int index)
{
    setHeadMode(index,"config");
}


/* Pestaña: "MCA" */

MainWindow::temp_code MainWindow::getTemperatureCode(double temperature)
{
    if (temperature<20) return ERROR;
    if (temperature>=20 && temperature<49) return NORMAL;
    if (temperature>=49 && temperature<56) return WARM;
    if (temperature>=56 && temperature<60) return HOT;
    if (temperature>=60) return TOO_HOT;
    else return NO_VALUE;
}

void MainWindow::setTemperatureBoard(double temp, QLabel *label_pmt, int pmt)
{
    QPalette palette_temperature;
    palette_temperature.setColor(QPalette::Background,Qt::black);

    switch (getTemperatureCode(temp)) {
    case ERROR:
        palette_temperature.setColor(QPalette::Background,Qt::lightGray);
        break;
    case NORMAL:
        palette_temperature.setColor(QPalette::Background,Qt::green);
        break;
    case WARM:
        palette_temperature.setColor(QPalette::Background,Qt::yellow);
        break;
    case HOT:
        palette_temperature.setColor(QPalette::Background,QColor::fromRgb(255,140,0)); // Naranja en RGB = 255,140,0
        break;
    case TOO_HOT:
        palette_temperature.setColor(QPalette::Background,Qt::red);
        break;
    default:
        break;
    }
    QString label_text="<span style='font-weight:600;'>"+QString::number(pmt)+"<br></span><span style='font-size:18pt; font-weight:600;'>"+QString::number(round(temp))+"</span>";
    label_pmt->setPalette(palette_temperature);
    label_pmt->setText(label_text);
}

void MainWindow::drawTemperatureBoard()
{
    double temp;

    try
    {
        for(int pmt = 0; pmt < PMTs; pmt++)
        {
            setMCAEDataStream("mca", arpet->getFunCSP3(), QString::number(pmt+1).toStdString(), arpet->getTemp_MCA());
            SendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
            string msg = ReadString();
            temp=arpet->getPMTTemperature(msg);

            cout<<"================================"<<endl;
            cout<<"Enviado: "<<arpet->getTrama_MCAE()<<endl;
            cout<<"Recibido: "<<msg<<endl;
            cout<<"Temperatura: "<<temp<<endl;
            cout<<"================================"<<endl;

            setTemperatureBoard(temp,pmt_label_table[pmt],pmt+1);
        }
    }
    catch( Exceptions & ex )
    {
         QMessageBox::critical(this,tr("Atención"),tr((string("Imposible obtener los valores de temperatura. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }
}


void MainWindow::clearTemperatureBoard()
{
    QPalette palette_temperature;
    palette_temperature.setColor(QPalette::Background,Qt::transparent);
    for(int pmt = 0; pmt < PMTs; pmt++)
       {
          pmt_label_table[pmt]->setPalette(palette_temperature);
          pmt_label_table[pmt]->setText(QString::number(pmt+1));
       }
}

void MainWindow::on_pushButton_adquirir_clicked()
{
    QString q_msg;
    bool accum=true;
    if (!(ui->checkBox_accum->isChecked())) accum=false;

    switch (adquire_mode) {
    case MONOMODE:
        q_msg = getMCA("mca",arpet->getFunCSP3());
        getPlot(accum, ui->specPMTs);        
        break;
    case MULTIMODE:
        q_msg = getMCA("mca",arpet->getFunCHead());
        getPlot(accum, ui->specHead);        
        break;
    case TEMPERATURE:
        drawTemperatureBoard();
        break;
    default:
        break;
    }

    ui->label_received->setText(q_msg);
    cout << arpet->getTrama_MCAE() << endl;
}

void MainWindow::on_pushButton_reset_clicked()
{
    switch (adquire_mode) {
    case MONOMODE:
        resetHitsValues();
        break;
    case MULTIMODE:
        resetHitsValues();
        break;
    case TEMPERATURE:
        clearTemperatureBoard();
        break;
    default:
        break;
    }
}


void MainWindow::on_pushButton_hv_configure_clicked()
{
    QString q_msg = setHV("mca",getHVValue());
    ui->label_received->setText(q_msg);
    cout << arpet->getTrama_MCAE() << endl;
}

void MainWindow::on_pushButton_l_5_clicked()
{
    QString q_msg = setHV("mca",getHVValue(-5));
    ui->label_received->setText(q_msg);
    cout << arpet->getTrama_MCAE() << endl;
}

void MainWindow::on_pushButton_l_10_clicked()
{
    QString q_msg = setHV("mca",getHVValue(-10));
    ui->label_received->setText(q_msg);
    cout << arpet->getTrama_MCAE() << endl;
}

void MainWindow::on_pushButton_l_50_clicked()
{
    QString q_msg = setHV("mca",getHVValue(-50));
    ui->label_received->setText(q_msg);
    cout << arpet->getTrama_MCAE() << endl;
}

void MainWindow::on_pushButton_p_5_clicked()
{
    QString q_msg = setHV("mca",getHVValue(5));
    ui->label_received->setText(q_msg);
    cout << arpet->getTrama_MCAE() << endl;
}

void MainWindow::on_pushButton_p_10_clicked()
{
    QString q_msg = setHV("mca",getHVValue(10));
    ui->label_received->setText(q_msg);
    cout << arpet->getTrama_MCAE() << endl;
}

void MainWindow::on_pushButton_p_50_clicked()
{
    QString q_msg = setHV("mca",getHVValue(50));
    ui->label_received->setText(q_msg);
    cout << arpet->getTrama_MCAE() << endl;
}


void MainWindow::on_pushButton_decrease_clicked()
{
    QString pmt=ui->lineEdit_pmt->text();
    if (pmt.toInt()>1)
    {
        int pmt_decrease=pmt.toInt()-1;
        ui->lineEdit_pmt->setText(QString::number(pmt_decrease));
    }
}

void MainWindow::on_pushButton_increase_clicked()
{
    QString pmt=ui->lineEdit_pmt->text();
    if (pmt.toInt()<PMTs)
    {
        int pmt_increase=pmt.toInt()+1;
        ui->lineEdit_pmt->setText(QString::number(pmt_increase));
    }
}

void MainWindow::setHeadModeGraph(int index)
{
    setHeadMode(index,"mca");
}

void MainWindow::setAdquireMode(int index)
{
    adquire_mode=index;
    switch (adquire_mode) {
    case MONOMODE:
        ui->frame_PMT->show();
        ui->frame_HV->show();
        ui->frame_MCA->show();
        ui->frame_temp->hide();
        ui->tabWidget_mca->setCurrentWidget(ui->tab_esp_2);
        break;
    case MULTIMODE:
        ui->frame_PMT->hide();
        ui->frame_HV->hide();
        ui->frame_temp->hide();
        ui->frame_MCA->show();
        ui->tabWidget_mca->setCurrentWidget(ui->tab_esp_1);
        break;
    case TEMPERATURE:
        ui->frame_PMT->hide();
        ui->frame_HV->hide();
        ui->tabWidget_mca->setCurrentWidget(ui->tab_esp_3);
        ui->frame_MCA->hide();
        ui->frame_temp->show();
    default:
        break;
    }
}

QString MainWindow::getMCA(string tab, string function)
{
    pmt_ui_current=getPMT();
    if (pmt_ui_current!=pmt_ui_previous) resetHitsValues();
    pmt_ui_previous=pmt_ui_current;
    setMCAEDataStream(tab, function, QString::number(pmt_ui_current).toStdString(), arpet->getData_MCA(),bytes_int);    
    string msg, msg_data;
    try
    {
        SendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg = ReadString();
        msg_data = ReadBufferString(bytes_int);
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr((string("No se pueden obtener los valores de MCA. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }
    arpet->getMCASplitData(msg_data, CHANNELS);    

    return QString::fromStdString(msg);
}

QString MainWindow::setHV(string tab, string hv_value)
{
    setMCAEDataStream(tab, arpet->getFunCSP3(), QString::number(getPMT()).toStdString(), arpet->getSetHV_MCA(),0, hv_value);
    string msg;
    try
    {
        SendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg = ReadString();
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }
    resetHitsValues();

    return QString::fromStdString(msg);
}

int MainWindow::getPMT()
{
    QString pmt=ui->lineEdit_pmt->text();
    switch (adquire_mode) {
    case MONOMODE:
        if(pmt.isEmpty() || pmt.toInt()==0)
        {
            pmt=QString::number(1);
            ui->lineEdit_pmt->setText(pmt);
        }
        break;
    case MULTIMODE:
        ui->lineEdit_pmt->setText(0);
        break;
    default:
        break;
    }

    return ui->lineEdit_pmt->text().toInt();
}

void MainWindow::setPMT(int value)
{
     ui->lineEdit_pmt->setText(QString::number(value));
}

string MainWindow::getHVValue(int value)
{
    int hv_value_int;
    if (ui->lineEdit_hv_value->text().isEmpty()) hv_value_int=0;
    if (value==0) hv_value_int=ui->lineEdit_hv_value->text().toInt();
    else  hv_value_int=ui->lineEdit_hv_value->text().toInt() + value;
    if (hv_value_int<0) hv_value_int=0;
    ui->lineEdit_hv_value->setText(QString::number(hv_value_int));

    return QString::number(hv_value_int).toStdString();
}

void MainWindow::setMCAEDataStream(string tab, string function, string pmt, string mca_function, int bytes_mca, string hv_value)
{
  arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead(tab).toStdString() + function);
  arpet->setMCAEStream(pmt, bytes_mca, mca_function, hv_value);
}

void MainWindow::resetHitsValues()
{
    hits_ui.fill(0);
}

void MainWindow::getPlot(bool accum, QCustomPlot *graph)
{
    /* Datos del gráfico */    
    if (!accum){
        resetHitsValues();
    }
    channels_ui = arpet->getChannels();
    transform(hits_ui.begin(), hits_ui.end(), arpet->getHitsMCA().begin(), hits_ui.begin(), plus<double>());

    /* Se genera los ejes */
    double c_max = *max_element(hits_ui.begin(),hits_ui.end());
    double c_min =0;
    if (c_max==0) {c_max=1; c_min=-1;}

    cout<<"El máximo elemento de Hits: "<<c_max<<endl;

    graph->addGraph();
    graph->graph(0)->setData(channels_ui, hits_ui);
    graph->xAxis2->setVisible(true);
    graph->xAxis2->setTickLabels(true);
    graph->yAxis2->setVisible(true);
    graph->yAxis2->setTickLabels(true);
    graph->xAxis->setLabel("Canales");
    graph->yAxis->setLabel("Cuentas");

    /* Rangos y grafico */
    graph->xAxis->setRange(0, CHANNELS);
    graph->yAxis->setRange(c_min, c_max*1.25);
    graph->replot();
}

/* Métodos generales del entorno gráfico */

void MainWindow::getARPETStatus()
{
    setMCAEDataStream("config", arpet->getFunCHead(), arpet->getBrCst(), arpet->getInit_MCA());    
    string msg_head;
    try
    {
        SendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg_head = ReadString();
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede obtener el estado del equipo. Revise la conexión. Error: ")+string(ex.excdesc)).c_str()));
    }
    setLabelState(!arpet->verifyMCAEStream(msg_head,arpet->getAnsHeadInit()),ui->label_arpet_power_supply);
}

/**
 * @brief MainWindow::parseConfigurationFile
 * @param filename
 * @return
 */
int MainWindow::parseConfigurationFile(QString filename)
{
    QFile configfile(filename);
    if (!configfile.open(QIODevice::ReadOnly)) {
        filename=openConfigurationFile();
        configfile.setFileName(filename);
        if (!configfile.open(QIODevice::ReadOnly)){
            qDebug() << "No se puede abrir el archivo de configuración. Error: " << configfile.errorString();
            QMessageBox::critical(this,tr("Atención"),tr("No se puede abrir el archivo de configuración."));
            return MCAE::FILE_NOT_FOUND;
        }
    }

    QSettings settings(filename, QSettings::IniFormat);

    /* Parameters */
    AT = settings.value("SetUp/AT", "US").toInt();
    LowLimit = settings.value("SetUp/LowLimit", "US").toInt();

    /* Paths to the configuration files */

    QString head= getHead("config");
    QString root=settings.value("Paths/root", "US").toString();

    coefT = root+settings.value("Cabezal"+head+"/coefT", "US").toString();
    coefenerg = root+settings.value("Cabezal"+head+"/coefenerg", "US").toString();
    hvtable = root+settings.value("Cabezal"+head+"/hvtable", "US").toString();
    coefx = root+settings.value("Cabezal"+head+"/coefx", "US").toString();
    coefy = root+settings.value("Cabezal"+head+"/coefy", "US").toString();
    coefest = root+settings.value("Cabezal"+head+"/coefest", "US").toString();
    coefT = root+settings.value("Cabezal"+head+"/coefT", "US").toString();

    configfile.close();

    return MCAE::OK;
}

/**
 * @brief MainWindow::openConfigurationFile
 * @return
 */
QString MainWindow::openConfigurationFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de configuración"),
                                                    QDir::homePath(),
                                                    tr("Texto (*.ini)"));
    return filename;
}

/**
 * @brief MainWindow::SetLabelState
 * @param state
 * @param label
 */
void MainWindow::setLabelState(bool state, QLabel *label, bool power_off)
{
    QPalette palette;

    if (!state && !power_off)
    {
        palette.setColor(QPalette::Background,Qt::green);
        label->setPalette(palette);
    }
    else if(power_off)
    {
        palette.setColor(QPalette::Background,Qt::gray);
        label->setPalette(palette);
    }
    else
    {
        palette.setColor(QPalette::Background,Qt::red);
        label->setPalette(palette);
    }

    return;
}


/**
 * @brief MainWindow::availablePortsName
 * @return
 */
QStringList MainWindow::availablePortsName()
{

    QStringList portsName;

    QDir dir("/dev/");
    QStringList filters;

    filters << "ttyUSB*";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::System);
    QFileInfoList list = dir.entryInfoList();

    for (int i=0; i< list.size(); i++)
    {
        portsName.append(list.at(i).canonicalFilePath ());
    }

    return portsName;
}

QString MainWindow::getHead(string tab)
{
    QString head;
    if (tab=="mca")
    {
        if (ui->comboBox_head_mode_select_graph->currentIndex()==MONOHEAD)
        {
           head=ui->comboBox_head_select_graph->currentText();
        }
    }
    else if (tab=="config")
    {
        if (ui->comboBox_head_mode_select_config->currentIndex()==MONOHEAD)
        {
           head=ui->comboBox_head_select_config->currentText();
        }
    }
    else head="";

    return head;
}

string MainWindow::ReadString(char delimeter)
{
    string msg;
    try{
         arpet->portReadString(&msg,delimeter);
    }
    catch( Exceptions & ex ){
         Exceptions exception_stop(ex.excdesc);
         throw exception_stop;
    }
    return msg;
}

string MainWindow::ReadBufferString(int buffer_size)
{
    string msg;
    try{
         arpet->portReadBufferString(&msg,buffer_size);
    }
    catch( Exceptions & ex ){
         Exceptions exception_stop(ex.excdesc);
         throw exception_stop;
    }
    return msg;
}

size_t MainWindow::SendString(string msg, string end)
{
    arpet->portFlush();
    size_t bytes_transfered = 0;

    try{
        string sended=msg + end;
        bytes_transfered = arpet->portWrite(&sended);
    }
    catch(boost::system::system_error e){
        Exceptions exception_serial_port((string("No se puede acceder al puerto serie. Error: ")+string(e.what())).c_str());
        throw exception_serial_port;
    }
//string("No se puede acceder al puerto serie. Error: ",
    return bytes_transfered;
}

void MainWindow::manageHeadCheckBox(string tab, bool show)
{
    if (tab=="config")
    {
        if (show) ui->frame_multihead_config->show();
        else ui->frame_multihead_config->hide();
    }
    else if(tab=="mca")
    {
        if (show) ui->frame_multihead_graph->show();
        else ui->frame_multihead_graph->hide();
    }
    else return;
}

void MainWindow::manageHeadComboBox(string tab, bool show)
{
    if (tab=="config"){
        if (show) ui->comboBox_head_select_config->show();
        else ui->comboBox_head_select_config->hide();
    }
    else if(tab=="mca")
    {
        if (show) ui->comboBox_head_select_graph->show();
        else ui->comboBox_head_select_graph->hide();
    }
    else return;
}


void MainWindow::setHeadMode(int index, string tab)
{
    switch (index) {
    case MONOHEAD:
        manageHeadComboBox(tab, true);
        manageHeadCheckBox(tab, false);
        break;
    case MULTIHEAD:
        manageHeadComboBox(tab, false);
        manageHeadCheckBox(tab, true);
        break;
    default:
        break;
    }
}

void MainWindow::syncHeadModeComboBoxToConfig(int index)
{
    ui->comboBox_head_mode_select_config->setCurrentIndex(index);
}

void MainWindow::syncHeadComboBoxToConfig(int index)
{
    ui->comboBox_head_select_config->setCurrentIndex(index);
}

void MainWindow::syncHeadModeComboBoxToMCA(int index)
{
    ui->comboBox_head_mode_select_graph->setCurrentIndex(index);
}

void MainWindow::syncHeadComboBoxToMCA(int index)
{
    ui->comboBox_head_select_graph->setCurrentIndex(index);
}

void MainWindow::syncCheckBoxHead1ToConfig(bool check)
{
    ui->checkBox_c_1->setChecked(check);
}

void MainWindow::syncCheckBoxHead2ToConfig(bool check)
{
    ui->checkBox_c_2->setChecked(check);
}

void MainWindow::syncCheckBoxHead3ToConfig(bool check)
{
    ui->checkBox_c_3->setChecked(check);
}

void MainWindow::syncCheckBoxHead4ToConfig(bool check)
{
    ui->checkBox_c_4->setChecked(check);
}

void MainWindow::syncCheckBoxHead5ToConfig(bool check)
{
    ui->checkBox_c_5->setChecked(check);
}

void MainWindow::syncCheckBoxHead6ToConfig(bool check)
{
    ui->checkBox_c_6->setChecked(check);
}

void MainWindow::syncCheckBoxHead1ToMCA(bool check)
{
    ui->checkBox_mca_1->setChecked(check);
}

void MainWindow::syncCheckBoxHead2ToMCA(bool check)
{
    ui->checkBox_mca_2->setChecked(check);
}

void MainWindow::syncCheckBoxHead3ToMCA(bool check)
{
    ui->checkBox_mca_3->setChecked(check);
}

void MainWindow::syncCheckBoxHead4ToMCA(bool check)
{
    ui->checkBox_mca_4->setChecked(check);
}

void MainWindow::syncCheckBoxHead5ToMCA(bool check)
{
    ui->checkBox_mca_5->setChecked(check);
}

void MainWindow::syncCheckBoxHead6ToMCA(bool check)
{
    ui->checkBox_mca_6->setChecked(check);
}

/* TEST BUTTONS. TODO: Delete all of them after debug */

void MainWindow::on_pushButton_clicked()
{
   QString sended = ui->plainTextEdit->toPlainText();

   size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
   string msg;
   try
   {
       msg = ReadString();
   }
   catch(Exceptions & ex)
   {
       QMessageBox::critical(this,tr("Atención"),tr(ex.excdesc));
   }
   QString q_msg=QString::fromStdString(msg);

   ui->label_12->setText(q_msg);

}

void MainWindow::on_pushButton_2_clicked()
{
   arpet->portFlush();
}

void MainWindow::on_pushButton_3_clicked()
{
    QString sended="#C401090009@0064010;";
    size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
    string msg;
    try
    {
        msg = ReadString();
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr(ex.excdesc));
    }
    QString q_msg=QString::fromStdString(msg);
    QString q_bytes=QString::number(bytes);
    ui->label_19->setText(q_bytes);
    ui->label_12->setText(q_msg);
    ui->label_20->setText(sended);
}

void MainWindow::on_pushButton_4_clicked()
{
    QString sended="#C402090009@0064010;";
    size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
    string msg;
    try
    {
        msg = ReadString();
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr(ex.excdesc));
    }
    QString q_msg=QString::fromStdString(msg);
    QString q_bytes=QString::number(bytes);
    ui->label_19->setText(q_bytes);
    ui->label_12->setText(q_msg);
    ui->label_20->setText(sended);
}

void MainWindow::on_pushButton_5_clicked()
{
    QString sended="#C402071552@196515";
    size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
    string msg;
    try
    {
        msg = ReadString();
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr(ex.excdesc));
    }
    QString q_msg=QString::fromStdString(msg);
    QString q_bytes=QString::number(bytes);
    string msg_data=ReadBufferString(6160);

    QByteArray q_msg_data(msg_data.c_str(), msg_data.length());

    QByteArray y = q_msg_data.left(4);

    cout<<y.toStdString()<<endl;

//    sleep(1);
//    arpet->portFlush();
    ui->label_19->setText(q_bytes);
    ui->label_12->setText(q_msg);
    ui->label_20->setText(sended);
}

void MainWindow::on_pushButton_9_clicked()
{
    QString sended="#C401071552@01650<";
    size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
    string msg;
    try
    {
        msg = ReadString();
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr(ex.excdesc));
    }
    QString q_msg=QString::fromStdString(msg);
    QString q_bytes=QString::number(bytes);
    string msg_data=ReadBufferString(6160);

    arpet->getMCASplitData(msg_data,CHANNELS);

    int frame=arpet->getFrameMCA();
    long time_mca=arpet->getTimeMCA();
    cout<<"Frame: "<< frame <<endl;
    cout<<"Adquisition time: "<< time_mca <<endl;
    cout<<"Temperature: "<<arpet->getTempValueMCA()<<endl;
    QVector<double> canales=arpet->getChannels();
    QVector<double> hits=arpet->getHitsMCA();

    ui->label_19->setText(q_bytes);
    ui->label_12->setText(q_msg);
    ui->label_20->setText(sended);
}

void MainWindow::on_pushButton_6_clicked()
{
    QString sended="#C402071552@02650=";
    size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
    string msg;
    try
    {
        msg = ReadString();
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr(ex.excdesc));
    }
    QString q_msg=QString::fromStdString(msg);
    QString q_bytes=QString::number(bytes);
    ui->label_19->setText(q_bytes);
    ui->label_12->setText(q_msg);
    ui->label_20->setText(sended);
}

void MainWindow::on_pushButton_7_clicked()
{
    QString sended="#C401071552@01650<";
    size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
    string msg;
    try
    {
        msg = ReadString();
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr(ex.excdesc));
    }
    QString q_msg=QString::fromStdString(msg);
    QString q_bytes=QString::number(bytes);
    ui->label_19->setText(q_bytes);
    ui->label_12->setText(q_msg);
    ui->label_20->setText(sended);
}

void MainWindow::getPreferences()
{

}

/**********************************************************/

