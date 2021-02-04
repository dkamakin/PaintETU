#include "myhelp.h"
#include "ui_myhelp.h"

MyHelp::MyHelp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::myhelp) {
    ui->setupUi(this);
}

MyHelp::~MyHelp() {
    delete ui;
}
