#include "myimage.h"

MyImage::MyImage() {
    isEmptyCheck = true;
    image = nullptr;
}

void MyImage::readImage(const char *imagePath) {
    if (image)
        freeMemory(image);

    FILE *fp = fopen(imagePath, "rb");

    if (!fp)
        return;

    image = new Png_t;
    image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!image->png_ptr) {
        fclose(fp);
        QMessageBox::critical(nullptr, "Couldn't read an image", "Couldn't create png_read_struct.");
        return;
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);

    if (!image->info_ptr) {
        png_destroy_read_struct(&image->png_ptr, NULL, NULL);
        fclose(fp);
        QMessageBox::critical(nullptr, "Couldn't read an image", "Couldn't create png_info_struct.");
        return;
    }

    if (setjmp(png_jmpbuf(image->png_ptr))) {
        png_destroy_read_struct(&image->png_ptr, NULL, NULL);
        fclose(fp);
        QMessageBox::critical(nullptr, "Couldn't read an image", "Error during init_io.");
        return;
    }

    png_init_io(image->png_ptr, fp);

    png_read_info(image->png_ptr, image->info_ptr);

    image->width = png_get_image_width(image->png_ptr, image->info_ptr);
    image->height = png_get_image_height(image->png_ptr, image->info_ptr);
    image->color_type = png_get_color_type(image->png_ptr, image->info_ptr);
    image->bit_depth = png_get_bit_depth(image->png_ptr, image->info_ptr);

    if (setjmp(png_jmpbuf(image->png_ptr))) {
        fclose(fp);
        QMessageBox::critical(nullptr, "Couldn't read an image", "Error during read_image.");
        return;
    }

    if (image->bit_depth == 16)
        png_set_strip_16(image->png_ptr);

    if (image->color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(image->png_ptr);

    if (image->color_type == PNG_COLOR_TYPE_GRAY && image->bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(image->png_ptr);

    if (png_get_valid(image->png_ptr, image->info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(image->png_ptr);

    if (image->color_type == PNG_COLOR_TYPE_RGB ||
            image->color_type == PNG_COLOR_TYPE_GRAY ||
            image->color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(image->png_ptr, 0xFF, PNG_FILLER_AFTER);

    if (image->color_type == PNG_COLOR_TYPE_GRAY ||
            image->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(image->png_ptr);

    image->number_of_passes = png_set_interlace_handling(image->png_ptr);
    png_read_update_info(image->png_ptr, image->info_ptr);

    image->pixelArray = new png_bytep[image->height];
    for (int y = 0; y < image->height; y++)
        image->pixelArray[y] = new png_byte[png_get_rowbytes(image->png_ptr, image->info_ptr)];

    png_read_image(image->png_ptr, image->pixelArray);
    fclose(fp);

    path = QString::fromUtf8(imagePath);
    isEmptyCheck = false;
    image->firstImage = image->pixelArray;
}

void MyImage::writeInfo(MyInfo *info) {
    if (isEmpty())
        return;
    QFileInfo fileInfo(path);
    info->ui->nameLabel->setText(fileInfo.baseName());
    info->ui->pathLabel->setText(fileInfo.filePath());
    info->ui->modifiedLabel->setText(fileInfo.lastModified().toString());
    info->ui->sizeLabel->setText(QString::number(fileInfo.size()/TO_KB) + " Kb");
    info->ui->widthLabel->setText(QString::number(image->width) + " px");
    info->ui->heightLabel->setText(QString::number(image->height) + " px");
    info->ui->colorLabel->setText(QString::number(image->color_type));
    info->ui->depthLabel->setText(QString::number(image->bit_depth));
}

void MyImage::drawTriangle(int x1, int y1, int x2, int y2, QColor color, QColor colorFill, int thick) {
    int x3 = x1-(x2-x1);

    drawParallelLines(x1, y1, x3, y2, color, thick);
    drawParallelLines(x1, y1, x2, y2, color, thick);
    drawParallelLines(x2, y2, x3, y2, color, thick);

    if (colorFill.isValid()) {
        png_byte libColorFill[] = {static_cast <unsigned char>(colorFill.red()),
                                   static_cast <unsigned char>(colorFill.green()),
                                   static_cast <unsigned char>(colorFill.blue())};
        if (x2 > x1)
            for (int i = x2; i > x3; i--)
                drawLine(x1, y1, i, y2, libColorFill);
        else
            for (int i = x2; i < x3; i++)
                drawLine(x1, y1, i, y2, libColorFill);
    }
}

png_bytep* MyImage::resampleImage(png_bytep *imageFrom, int newY, int newX, int oldY, int oldX) {
    if (newY == oldY && newX == oldX)
        return imageFrom;
    double hRatio = (double)newY/oldY;
    double wRatio = (double)newX/oldX;
    png_bytep *resizedImage = new png_bytep [newY];

    for (int y = 0; y < newY; y++) {
        resizedImage[y] = new png_byte [newX*4];
        for (int x = 0; x < newX; x++)
            for (int i = 0; i < 4; i++)
                resizedImage[y][x*4+i] = imageFrom[(int)(y/hRatio)][(int)(x/wRatio)*4+i];
    }

    return resizedImage;
}

void MyImage::addPiece(int x, int y, png_bytep* addTo, png_bytep* addFrom, int ySize, int xSize) {
    for (int pY = y, oY = 0; pY < y+ySize; pY++, oY++)
        for (int pX = x, oX = 0; pX < x+xSize; pX++, oX++)
            for (int i = 0; i < 4; i++)
                addTo[pY][pX*4+i] = addFrom[oY][oX*4+i];
}

void MyImage::buildCollage(int mSize, int nSize) {
    if (image->pixelArray != image->firstImage) {
        freeArray(image->pixelArray, image->height);
        image->pixelArray = image->firstImage;
    }
    if (mSize == 1 && nSize == 1)
        return;

    int resizedY = image->height/mSize;
    int resizedX = image->width/nSize;
    png_bytep *resized = resampleImage(image->pixelArray, resizedY, resizedX,
                                       image->height, image->width);

    png_bytep *newPixels = new png_bytep [image->height];
    for (int y = 0; y < image->height; y++)
        newPixels[y] = new png_byte [image->width*4];

    for (int m = 0; m < mSize; m++) {
        addPiece(0, resizedY*m, newPixels, resized, resizedY, resizedX);
        for (int n = 0; n < nSize; n++)
            addPiece(resizedX*n, resizedY*m, newPixels, resized, resizedY, resizedX);
    }

    freeArray(resized, resizedY);
    image->pixelArray = newPixels;
}

void MyImage::writePixel(png_bytep color, png_bytep pixel) {
    for (int i = 0; i < 3; i++)
        pixel[i] = color[i];
}

void MyImage::drawLine(int x1, int y1, int x2, int y2, png_bytep color) {
    const int deltaX = abs(x2 - x1);
    const int deltaY = abs(y2 - y1);
    const int signX = x1 < x2 ? 1 : -1;
    const int signY = y1 < y2 ? 1 : -1;

    int error = deltaX - deltaY;

    writePixel(color, &(image->pixelArray[y2][x2*4]));

    while (x1 != x2 || y1 != y2)  {
        writePixel(color, &(image->pixelArray[y1][x1*4]));
        const int error2 = error * 2;

        if (error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }

        if (error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
}

void MyImage::drawParallelLines(int x1, int y1, int x2, int y2, QColor color, int thick) {
    png_byte libColor[] = {static_cast <unsigned char>(color.red()),
                               static_cast <unsigned char>(color.green()),
                               static_cast <unsigned char>(color.blue())};
    if (abs(y2-y1) > abs(x2-x1))
        for (int i = -thick/2; i < thick/2; i++)
            drawLine(x1+i, y1, x2+i, y2, libColor);
    else
        for (int i = -thick/2; i < thick/2; i++)
            drawLine(x1, y1+i, x2, y2+i, libColor);
}

void MyImage::repaintRectangle(QColor colorFrom, QColor colorTo) {
    png_byte libColor[] = {static_cast <unsigned char>(colorFrom.red()),
                           static_cast <unsigned char>(colorFrom.green()),
                           static_cast <unsigned char>(colorFrom.blue())};
    int ans = 0;
    int m = image->width, n = image->height;
    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    std::vector<int> d(m, -1), d1(m), d2(m);
    std::stack<int> st;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            png_bytep px = &(image->pixelArray[i][j*4]);
            if (px[0] != libColor[0] && px[1] != libColor[1] && px[2] != libColor[2])
                d[j] = i;
        }
        while (!st.empty()) st.pop();
        for (int j = 0; j < m; ++j) {
            while (!st.empty() && d[st.top()] <= d[j]) st.pop();
            d1[j] = st.empty() ? -1 : st.top();
            st.push(j);
        }
        while (!st.empty()) st.pop();
        for (int j = m - 1; j >= 0; --j) {
            while (!st.empty() && d[st.top()] <= d[j]) st.pop();
            d2[j] = st.empty() ? m : st.top();
            st.push(j);
        }
        for (int j = 0; j < m; j++) {
            if (ans < (i - d[j]) * (d2[j] - d1[j] - 1)) {
                ans = (i - d[j]) * (d2[j] - d1[j] - 1);
                y1 = d[j] + 1;
                y2 = i;
                x1 = d1[j] + 1;
                x2 = d2[j];
            }
        }
    }
    libColor[0] = static_cast <unsigned char>(colorTo.red());
    libColor[1] = static_cast <unsigned char>(colorTo.green());
    libColor[2] = static_cast <unsigned char>(colorTo.blue());
    if (abs(y2 - y1) > abs(x2 - x1))
        for (int i = 0; i < y2 - y1 + 1; i++)
            drawLine(x1, y1 + i, x2, y1 + i, libColor);
    else
        for (int i = 0; i < x2 - x1; i++)
            drawLine(x1 + i, y1, x1 + i, y2, libColor);
}

QPixmap MyImage::getMyPixmap() {
    if (isEmpty())
        return myPixmap;
    QImage imageQt(image->width, image->height, QImage::Format_ARGB32);
    for (int y = 0; y < image->height; y++)
        for (int x = 0; x < image->width; x++) {
            png_bytep px = &(image->pixelArray[y][x*4]);
            imageQt.setPixel(x, y, qRgba(px[0], px[1], px[2], px[3]));
            }
    myPixmap = QPixmap::fromImage(imageQt);
    return myPixmap;
}

void MyImage::writeImage(const char *imagePath) {
    FILE *fp = fopen(imagePath, "wb");
    if (!fp)
        return;

    image->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!image->png_ptr) {
        QMessageBox::critical(nullptr, "Couldn't write an image", "Couldn't create png_write_struct.");
        return;
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);

    if (!image->info_ptr) {
        QMessageBox::critical(nullptr, "Couldn't write an image", "Couldn't create png_info_struct.");
        return;
    }

    if (setjmp(png_jmpbuf(image->png_ptr))) {
        QMessageBox::critical(nullptr, "Couldn't write an image", "Error during init_io.");
        return;
    }

    png_init_io(image->png_ptr, fp);

    if (setjmp(png_jmpbuf(image->png_ptr))) {
        QMessageBox::critical(nullptr, "Couldn't write an image", "Error during writing header.");
        return;
    }

    png_set_IHDR(image->png_ptr, image->info_ptr, image->width, image->height,
                 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(image->png_ptr, image->info_ptr);

    if (setjmp(png_jmpbuf(image->png_ptr))) {
        QMessageBox::critical(nullptr, "Couldn't write an image", "Error during writing bytes.");
        return;
    }

    png_write_image(image->png_ptr, image->pixelArray);

    if (setjmp(png_jmpbuf(image->png_ptr))) {
        QMessageBox::critical(nullptr, "Couldn't save a file", "Error during end of write.");
        return;
    }

    png_write_end(image->png_ptr, NULL);

    if (image->png_ptr && image->info_ptr)
        png_destroy_write_struct(&image->png_ptr, &image->info_ptr);

    fclose(fp);
}

bool MyImage::isEmpty() {
    return isEmptyCheck;
}

void MyImage::freeMemory(Png_t *image) {
    if (image->pixelArray != image->firstImage)
        freeArray(image->pixelArray, image->height);
    freeArray(image->firstImage, image->height);
    png_destroy_read_struct(&image->png_ptr, NULL, NULL);
    delete image;
}

void MyImage::freeArray(png_bytep *array, int height) {
    for (int y = 0; y < height; y++)
        delete [] array[y];
    delete [] array;
}

MyImage::~MyImage() {
    if (image)
        freeMemory(image);
    isEmptyCheck = true;
}
