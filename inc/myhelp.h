#ifndef MYHELP_H
#define MYHELP_H

#include <QDialog>

namespace Ui {
class myhelp;
}

class MyHelp : public QDialog {
    Q_OBJECT

public:
    explicit MyHelp(QWidget *parent = nullptr);
    ~MyHelp();

private:
    Ui::myhelp *ui;
};

#endif // MYHELP_H
