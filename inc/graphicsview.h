#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QPoint>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QGraphicsItemGroup>
#include <QMessageBox>

#define THICK 1

#define NO_ACTION 0
#define LINE 1
#define TRIANGLE 2

class graphicsView : public QGraphicsView {
    Q_OBJECT

signals:
    void drawDone();
    void mouseMoved();

public:
    explicit graphicsView(QWidget *parent = nullptr);
    ~graphicsView();
    void setLine(QColor, QColor, int, int);
    void drawLine(int, int, int, int, QPen);
    void setImage(QPixmap);
    void updateGroup();
    void deleteGroup(QGraphicsItemGroup*);
    QPoint getLastCoords();
    QPoint getFirstCoords();
    QColor getColor();
    QColor getColorFill();

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPoint firstClickCoords;
    QPoint lastClickCoords;
    QPen myPen;
    QPen myFillPen;
    QColor color;
    QColor colorFill;
    QPixmap pixmap;
    QGraphicsItem *qPixmap;
    QGraphicsScene *scene;
    QGraphicsItemGroup *group;
    bool leftMouseButtonPressed;
    int thick;
    int action;
    int mSize;
    int nSize;
    int x1;
    int y1;
};

#endif // GRAPHICSVIEW_H
