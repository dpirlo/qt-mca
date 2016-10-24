#ifndef SETPREFERENCES_H
#define SETPREFERENCES_H

#include <QDialog>

namespace Ui {
class SetPreferences;
}

class SetPreferences : public QDialog
{
    Q_OBJECT

public:
    explicit SetPreferences(QWidget *parent = 0);
    bool GetDegugConsoleValue();
    void accept();
    void reject();
    ~SetPreferences();

private:
    Ui::SetPreferences *ui;
    bool debconsole;
};

#endif // SETPREFERENCES_H
