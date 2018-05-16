#ifndef VALIDATE_H
#define VALIDATE_H

#include <QDialog>

namespace Ui {
class Validate;
}

class Validate : public QDialog
{
    Q_OBJECT

public:
    QVector<double> channels_ui;
    explicit Validate(QWidget *parent = 0);
    void accept();
    void reject();
    ~Validate();
    void PlotHits(QVector<double> hits, QVector<double> channels, double Pico_MAX, double Pico_MIN, double Dinodo_MAX, double Dinodo_MIN);
    void PlotRoto(int pmt_roto,QVector<double> hits,QVector<double> channels);

private:
    Ui::Validate *ui;
};

#endif // VALIDATE_H
