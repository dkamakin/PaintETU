#include "graphicsview.h"

graphicsView::graphicsView(QWidget *parent) :
    QGraphicsView(parent) {
    action = NO_ACTION;
    leftMouseButtonPressed = false;
    group = nullptr;
    scene = nullptr;
    qPixmap = nullptr;
}

void graphicsView::setLine(QColor colorRecieved, QColor colorFillRecived, int thickRecieved, int actionRecieved) {
    if (colorFillRecived.isValid())
        myFillPen = QPen(colorFillRecived, THICK, Qt::SolidLine, Qt::RoundCap);
    myPen = QPen(colorRecieved, THICK, Qt::SolidLine, Qt::RoundCap);
    colorFill = colorFillRecived;
    color = colorRecieved;
    action = actionRecieved;
    thick = thickRecieved;
}

void graphicsView::setImage(QPixmap pixmapRecieved) {
    if (scene) {
        scene->removeItem(qPixmap);
        delete qPixmap;
        delete scene;
    }

    scene = new QGraphicsScene();
    pixmap = pixmapRecieved;
    qPixmap = scene->addPixmap(pixmap);
    setScene(scene);
    setMouseTracking(true);
}

void graphicsView::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton || !action)
        return;
    firstClickCoords = mapToScene(event->pos()).toPoint();
    x1 = firstClickCoords.x(), y1 = firstClickCoords.y();
    leftMouseButtonPressed = true;
    group = nullptr;
}

void graphicsView::mouseMoveEvent(QMouseEvent *event) {
    lastClickCoords = mapToScene(event->pos()).toPoint();
    emit mouseMoved();

    if (!action || !leftMouseButtonPressed)
        return;

    int x2 = lastClickCoords.x(), y2 = lastClickCoords.y();
    int x3 = x1-(x2-x1);
    updateGroup();

    switch (action) {
    case LINE:
        drawLine(x1, y1, x2, y2, myPen);
        break;
    case TRIANGLE:
        drawLine(x1, y1, x2, y2, myPen);
        drawLine(x2, y2, x3, y2, myPen);
        drawLine(x1, y1, x3, y2, myPen);
        if (colorFill.isValid()) {
            if (x2 > x1)
                for (int i = x2; i > x3; i--)
                    group->addToGroup(scene->addLine(x1, y1, i, y2, myFillPen));
            else
                for (int i = x2; i < x3; i++)
                    group->addToGroup(scene->addLine(x1, y1, i, y2, myFillPen));
        }
        break;
    default:
        break;
    }

    scene->addItem(group);
}

void graphicsView::mouseReleaseEvent(QMouseEvent *event) {
    if (!action || !leftMouseButtonPressed)
        return;
    lastClickCoords = mapToScene(event->pos()).toPoint();
    int x2 = lastClickCoords.x(), y2 = lastClickCoords.y();

    try {
        if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 ||
                x1-thick/2 < 0 || x2-thick/2 < 0 ||
                y1-thick/2 < 0 || y2-thick/2 < 0)
            throw std::exception();
        if (x1 > pixmap.width() || y1 > pixmap.height() ||
                x2 > pixmap.width() || y2 > pixmap.height() ||
                x1+thick/2 > pixmap.width() || x2+thick/2 > pixmap.width() ||
                y1+thick/2 > pixmap.height() || y2+thick/2 > pixmap.height())
            throw std::exception();
        if (action == TRIANGLE && (x1-(x2-x1) < 0 || x1+(x1-x2) > pixmap.width()))
            throw std::exception();
        emit drawDone();
    } catch(std::exception&) {
        QMessageBox::warning(this, "Wrong coordinates", "Sorry, your shape has wrong coordinates");
        deleteGroup(group);
    }

    leftMouseButtonPressed = false;
}

void graphicsView::drawLine(int x1, int y1, int x2, int y2, QPen pen) {
    if (abs(y2-y1) > abs(x2-x1))
        for (int i = -thick/2; i < thick/2; i++)
            group->addToGroup(scene->addLine(x1+i, y1, x2+i, y2, pen));
    else
        for (int i = -thick/2; i < thick/2; i++)
            group->addToGroup(scene->addLine(x1, y1+i, x2, y2+i, pen));
}

void graphicsView::deleteGroup(QGraphicsItemGroup *group) {
    if (!group)
        return;
    foreach (QGraphicsItem *item, scene->items())
        if (item->group() == group)
            delete item;
    delete group;
}

void graphicsView::updateGroup() {
    if (group)
        scene->removeItem(group);
    group = new QGraphicsItemGroup();
}


QPoint graphicsView::getLastCoords() {
    return lastClickCoords;
}

QPoint graphicsView::getFirstCoords() {
    return firstClickCoords;
}

QColor graphicsView::getColor() {
    return color;
}

QColor graphicsView::getColorFill() {
    return colorFill;
}

graphicsView::~graphicsView() {
    if (scene) {
        scene->removeItem(qPixmap);
        delete qPixmap;
        delete scene;
    }
}
