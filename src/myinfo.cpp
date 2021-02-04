#include "myinfo.h"
#include "ui_myinfo.h"

MyInfo::MyInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MyInfo) {
    ui->setupUi(this);
}

MyInfo::~MyInfo() {
    delete ui;
}
