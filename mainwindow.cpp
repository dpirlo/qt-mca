#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include <QString>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_3_clicked()
{
    ui->textEdit->clear();    
}

void MainWindow::on_pushButton_triple_ventana_clicked()
{
    QString fileName = OpenConfigurationFile();
    ui->textBrowser_triple_ventana->setText(fileName);
}

void MainWindow::on_pushButton_hv_clicked()
{
    QString fileName = OpenConfigurationFile();
    ui->textBrowser_hv->setText(fileName);
}

void MainWindow::on_pushButton_energia_clicked()
{
    QString fileName = OpenConfigurationFile();
    ui->textBrowser_energia->setText(fileName);
}


void MainWindow::on_pushButton_posicion_X_clicked()
{
    QString fileName = OpenConfigurationFile();
    ui->textBrowser_posicion_X->setText(fileName);
}

void MainWindow::on_pushButton_posicion_Y_clicked()
{
    QString fileName = OpenConfigurationFile();
    ui->textBrowser_posicion_Y->setText(fileName);
}

void MainWindow::on_pushButton_salir_clicked()
{
    QApplication::quit();
}

void MainWindow::on_pushButton_salir_graficos_clicked()
{
     QApplication::quit();
}

QString MainWindow::OpenConfigurationFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de configuración"),
                                                    QDir::homePath(),
                                                    tr("Texto (*.txt)"));
    return filename;
}

void MainWindow::SetLabelState(bool state, QLabel *label)
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


/* TOMARLOS COMO EJEMPLO PARA LOS ESTADOS EN EL CABEZAL */

void MainWindow::on_pushButton_4_clicked()
{
      SetLabelState(true,ui->label_cabezal_estado);

}

void MainWindow::on_pushButton_5_clicked()
{
    SetLabelState(false,ui->label_cabezal_estado);
}

/*********************************************************/

/* TOMARLOS COMO EJEMPLO PARA ENVIO Y RECEPCIÓN DEL SERIE */

int MainWindow::on_pushButton_clicked()
{

    QMessageBox *mbox = new QMessageBox(this);

    try{
        mca.portConnect("/dev/ttyUSB1",115200);
    }
    catch(boost::system::system_error e)
        {
        mbox->setText(e.what());
        mbox->exec();
        return -1;
        }

    delete mbox;
    return 0;
}

void MainWindow::on_pushButton_2_clicked()
{
    unsigned char w = {2};
    unsigned char c;

    mca.portWrite(w,1);

    mca.portRead(&c); //TODO: Ojo que bloquea, implemetar timeout
    std::cout<<c<<std::endl;
}

/*********************************************************/
