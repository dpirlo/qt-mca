#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->comboBox_port->addItems(availablePortsName());
}

MainWindow::~MainWindow()
{
    delete ui;   
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

void MainWindow::on_pushButton_salir_clicked()
{
    mca.portDisconnect();
    QApplication::quit();
}

void MainWindow::on_pushButton_salir_graficos_clicked()
{
    mca.portDisconnect();
    QApplication::quit();
}

void MainWindow::on_pushButton_obtener_rutas_clicked()
{
    string initfile=".//config.ini";
    QString filename=QString::fromStdString(initfile);
    parseConfigurationFile(filename);

    ui->textBrowser_triple_ventana->setText(coefest);
    ui->textBrowser_hv->setText(hvtable);
    ui->textBrowser_energia->setText(coefenerg);
    ui->textBrowser_posicion_X->setText(coefx);
    ui->textBrowser_posicion_Y->setText(coefy);
    ui->textBrowser_tiempos_cabezal->setText(coefT);

    ui->plainTextEdit_alta->document()->setPlainText(QString::number(AT));
    ui->plainTextEdit_limiteinferior->document()->setPlainText(QString::number(LowLimit));
}

int MainWindow::on_pushButton_conectar_clicked()
{

    QMessageBox *mbox = new QMessageBox(this);

    if(mca.isPortOpen())
    {
        ui->pushButton_conectar->setText("Connect");
        mca.portDisconnect();
    }
    else
    {
        ui->pushButton_conectar->setText("Disconnect");
        try{
            QString port_name=ui->comboBox_port->currentText();
            mca.portConnect(port_name.toStdString().c_str(), 115200);
            mbox->setText("Conectado al puerto: " + port_name);
            mbox->exec();
        }
        catch(boost::system::system_error e)
            {
            mbox->setText(e.what());
            mbox->exec();
            return -1;
        }
    }

    delete mbox;
    return ComunicacionMCA::OK;
}

/* Métodos generales del entorno gráfico */

int MainWindow::parseConfigurationFile(QString filename)
{
    QMessageBox *mbox = new QMessageBox(this);

    QFile configfile(filename);
    if (!configfile.open(QIODevice::ReadOnly)) {
        filename=openConfigurationFile();
        configfile.setFileName(filename);
        if (!configfile.open(QIODevice::ReadOnly)){
            qDebug() << "No se puede abrir el archivo de configuración. Error: " << configfile.errorString();
            mbox->setText("No se puede abrir el archivo de configuración");
            mbox->exec();
            return ComunicacionMCA::FILE_NOT_FOUND;
        }
    }

    QSettings settings(filename, QSettings::IniFormat);

    /* Paths to the configuration files */
    coefT = settings.value("Paths/coefT", "US").toString();
    coefenerg = settings.value("Paths/coefenerg", "US").toString();
    hvtable = settings.value("Paths/hvtable", "US").toString();
    coefx = settings.value("Paths/coefx", "US").toString();
    coefy = settings.value("Paths/coefy", "US").toString();
    coefest = settings.value("Paths/coefest", "US").toString();
    coefT = settings.value("Paths/coefT", "US").toString();

    /* Parameters */
    AT = settings.value("SetUp/AT", "US").toInt();
    LowLimit = settings.value("SetUp/LowLimit", "US").toInt();

    configfile.close();

    return ComunicacionMCA::OK;
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
void MainWindow::setLabelState(bool state, QLabel *label)
{
    QPalette palette;

    if (!state)
    {
        palette.setColor(QPalette::Background,Qt::green);
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


/* TOMARLOS COMO EJEMPLO PARA ENVIO Y RECEPCIÓN DEL SERIE */

void MainWindow::on_pushButton_2_clicked()
{
    QMessageBox *mbox = new QMessageBox(this);
    string sended = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
    string received;

    mca.portWrite(&sended);
    //mca.portRead(&received,sended.size()); //TODO: Ojo que bloquea, implemetar timeout

    cout<<received<<endl;
    mbox->setText(QString::fromStdString(received));
    mbox->exec();
}
/*********************************************************/




