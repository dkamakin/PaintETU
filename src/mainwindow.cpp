#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    info = new MyInfo();
    image = new MyImage();
    help = new MyHelp();
    settings = new QSettings();
    currentAction = NO_ACTION;

    connect(ui->view, SIGNAL(drawDone()), this, SLOT(saveChanges()));
    connect(ui->view, SIGNAL(mouseMoved()), this, SLOT(writeCoords()));
}

void MainWindow::on_actionOpen_triggered() {
    QString fileName = QFileDialog::getOpenFileName(this, "Choose the image to read",
                                                    settings->value("path", "./").toString(),
                                                    "Image (*.png)");
    if (fileName.isNull())
        return;
    image->readImage(fileName.toUtf8().constData());

    if (image->isEmpty()) {
        QMessageBox::warning(this, "Something went wrong", "Sorry, couldn't read that image");
        return;
    }

    ui->view->setImage(image->getMyPixmap());
    setWindowTitle(QFileInfo(fileName).baseName() + QString(" - ETUPaint"));
    settings->setValue("path", fileName);
}

void MainWindow::on_actionSave_as_triggered() {
    if (checkForCorrect(currentAction))
        return;
    QString fileName = QFileDialog::getSaveFileName(this, "Choose the image or make a new one",
                                                    settings->value("path", "./").toString(),
                                                    "Image (*.png)");
    if (fileName.isNull())
        return;

    image->writeImage(fileName.toUtf8().constData());
}

void MainWindow::saveChanges() {
    int x1 = ui->view->getFirstCoords().x(), y1 = ui->view->getFirstCoords().y();
    int x2 = ui->view->getLastCoords().x(), y2 = ui->view->getLastCoords().y();
    QColor color = ui->view->getColor();
    QColor colorFill = ui->view->getColorFill();

    switch (currentAction) {
    case LINE:
        image->drawParallelLines(x1, y1, x2, y2, color, ui->lineThick->value());
        break;
    case TRIANGLE:
        image->drawTriangle(x1, y1, x2, y2, color, colorFill, ui->triangleThick->value());
        break;
    default:
        break;
    }

    ui->view->setImage(image->getMyPixmap());
}

void MainWindow::on_repaintButton_clicked() {
    if (checkForCorrect(currentAction))
        return;
    QColor color = QColorDialog::getColor();
    QColor colorFill = QColorDialog::getColor();

    if (!color.isValid() || !colorFill.isValid())
        return;

    image->repaintRectangle(color, colorFill);
    ui->view->setImage(image->getMyPixmap());
}

void MainWindow::on_collageButton_clicked() {
    if (checkForCorrect(currentAction))
        return;
    image->buildCollage(ui->mSizeCollage->value(), ui->nSizeCollage->value());
    ui->view->setImage(image->getMyPixmap());
}

void MainWindow::on_triangleButton_clicked() {
    if (checkForCorrect(TRIANGLE))
        return;
    QColor colorFill = nullptr;

    QColor color = QColorDialog::getColor();
    if (!color.isValid())
        return;

    QMessageBox::StandardButton reply =
            QMessageBox::question(this, "Triangle", "Fill the triangle?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        colorFill = QColorDialog::getColor();
        if (!colorFill.isValid())
            return;
    }

    ui->view->setLine(color, colorFill, ui->triangleThick->value(), TRIANGLE);
}

void MainWindow::on_lineButton_clicked() {
    if (checkForCorrect(LINE))
        return;
    QColor color = QColorDialog::getColor();

    if (!color.isValid())
        return;

    ui->view->setLine(color, nullptr, ui->lineThick->value(), LINE);
}

void MainWindow::on_actionFAQ_triggered() {
    help->exec();
}

void MainWindow::on_actionInfo_triggered() {
    if (checkForCorrect(currentAction))
        return;
    image->writeInfo(info);
    info->exec();
}

void MainWindow::on_actionAuthor_triggered() {
   QMessageBox::information(this, "Author", "Daniil Kamakin, 9381");
}

int MainWindow::checkForCorrect(int action) {
    if (image->isEmpty()) {
        QMessageBox::information(this, "No image", "Please, read an image first");
        return 1;
    }
    currentAction = action;
    return 0;
}

void MainWindow::writeCoords() {
    QPoint coords = ui->view->getLastCoords();
    ui->labelCoords->setText(QString::number(coords.x()) + ' ' + QString::number(coords.y()));
}

MainWindow::~MainWindow() {
    delete ui;
}
