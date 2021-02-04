#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QColorDialog>
#include <QSettings>
#include "myimage.h"
#include "myhelp.h"
#include "myinfo.h"
#include "graphicsview.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionAuthor_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_as_triggered();
    void on_actionFAQ_triggered();
    void on_actionInfo_triggered();
    void on_lineButton_clicked();
    void on_triangleButton_clicked();
    void on_collageButton_clicked();
    void on_repaintButton_clicked();
    void saveChanges();
    void writeCoords();

private:
    int checkForCorrect(int);

private:
    Ui::MainWindow *ui;
    MyImage *image;
    MyInfo *info;
    QSettings *settings;
    MyHelp *help;
    int currentAction;
};

#endif // MAINWINDOW_H
