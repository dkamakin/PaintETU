#ifndef MYIMAGE_H
#define MYIMAGE_H

#include <QPixmap>
#include <QMessageBox>
#include <QDateTime>
#include <stack>
#include <png.h>
#include "myinfo.h"

#define TO_KB 1024

struct Png_t {
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *pixelArray;
    png_bytep *firstImage;

    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
};

class MyImage {
public:
    explicit MyImage();
    ~MyImage();
    void readImage(const char*);
    void writeImage(const char*);
    void drawParallelLines(int, int, int, int, QColor, int);
    void drawLine(int, int, int, int, png_bytep);
    void writePixel(png_byte*, png_bytep);
    void repaintRectangle(QColor, QColor);
    void drawTriangle(int, int, int, int, QColor, QColor, int);
    void buildCollage(int, int);
    void addPiece(int, int, png_bytep*, png_bytep*, int, int);
    void deleteImage(png_bytep*, int);
    void freeMemory(Png_t*);
    void freeArray(png_bytep*, int);
    void writeInfo(MyInfo*);
    bool isEmpty();
    png_bytep* resampleImage(png_bytep*, int, int, int, int);
    QPixmap getMyPixmap();

private:
    bool isEmptyCheck;
    Png_t *image;
    QPixmap myPixmap;
    QString path;
};

#endif // MYIMAGE_H
