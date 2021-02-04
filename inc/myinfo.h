#ifndef MYINFO_H
#define MYINFO_H

#include <QFileInfo>
#include "ui_myinfo.h"

namespace Ui {
class MyInfo;
}

class MyInfo : public QDialog {
    Q_OBJECT

public:
    explicit MyInfo(QWidget *parent = nullptr);
    ~MyInfo();

public:
    Ui::MyInfo *ui;
};

#endif // MYINFO_H
