#include "Validate.h"
#include "ui_Validate.h"

Validate::Validate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Validate)
{
    ui->setupUi(this);
}

Validate::~Validate()
{
    delete ui;
}

void Validate::accept()
{
    QDialog::accept();
}

void Validate::reject()
{
    QDialog::reject();
}

void Validate::PlotHits(QVector<double> hits,QVector<double> channels,double Pico_MAX,double Pico_MIN, double Dinodo_MAX, double Dinodo_MIN)
{
    channels_ui.resize(channels.length());
    channels_ui = channels;

    ui->label_pmt_roto->hide();
    ui->label_pico->show();
    ui->label_dinodo->show();
    ui->label_pico_max->show();
    ui->label_pico_min->show();
    ui->label_dinodo_max->show();
    ui->label_dinodo_min->show();
    ui->label_label_pico_max->show();
    ui->label_label_pico_min->show();
    ui->label_label_dinodo_max->show();
    ui->label_label_dinodo_min->show();

    ui->specValidate->clearGraphs();
    ui->specValidate->addGraph();
    //ui->specValidate->graph()->setName("Validación de calibración gruesa");
    ui->specValidate->graph()->setAdaptiveSampling(true);
    ui->specValidate->graph()->addData(channels_ui,hits);
    //ui->specValidate->legend->setVisible(true);
    ui->specValidate->rescaleAxes();
    ui->specValidate->replot();

    ui->label_pico_max->setText(QString::number(Pico_MAX));
    ui->label_pico_min->setText(QString::number(Pico_MIN));
    ui->label_dinodo_max->setText(QString::number(Dinodo_MAX));
    ui->label_dinodo_min->setText(QString::number(Dinodo_MIN));
}

void Validate::PlotRoto(int pmt_roto,QVector<double> hits,QVector<double> channels)
{
    channels_ui.resize(channels.length());
    channels_ui = channels;

    ui->label_pmt_roto->show();
    ui->label_pico->hide();
    ui->label_dinodo->hide();
    ui->label_pico_max->hide();
    ui->label_pico_min->hide();
    ui->label_dinodo_max->hide();
    ui->label_dinodo_min->hide();
    ui->label_label_pico_max->hide();
    ui->label_label_pico_min->hide();
    ui->label_label_dinodo_max->hide();
    ui->label_label_dinodo_min->hide();

    ui->specValidate->clearGraphs();
    ui->specValidate->addGraph();
    ui->specValidate->graph()->setName("PMT "+QString::number(pmt_roto));
    ui->specValidate->graph()->setAdaptiveSampling(true);
    ui->specValidate->graph()->addData(channels_ui,hits);
    ui->specValidate->legend->setVisible(true);
    ui->specValidate->rescaleAxes();
    ui->specValidate->replot();

    ui->label_pmt_roto->setText("PMT "+QString::number(pmt_roto)+" ROTO");

}
