#ifndef SETPREFERENCES_H
#define SETPREFERENCES_H

#include <QDialog>
#include <iostream>

using namespace std;

namespace Ui {
class SetPreferences;
}

class SetPreferences : public QDialog
{
    Q_OBJECT

public:
    explicit SetPreferences(QWidget *parent = 0);    
    void accept();
    void reject();
    ~SetPreferences();

private:
    Ui::SetPreferences *ui;
    bool debconsole;

public:
    bool GetDegugConsoleValue() const { return debconsole; }

};

#endif // SETPREFERENCES_H
