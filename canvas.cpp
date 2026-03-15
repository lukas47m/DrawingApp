#include "canvas.h"
#include <QDebug>

Canvas::Canvas(size_t width, size_t height,QWidget *parent) : QWidget(parent),image(width, height, QImage::Format_RGBA8888) {

    image.fill(QColor(255, 255, 255, 255));
    setFixedSize(width, height);
}

void Canvas::undo(){
    qDebug() << "Undo triggered";
}
void Canvas::redo(){
    qDebug() << "Redo triggered";
}

void Canvas::paintEvent(QPaintEvent * p)
{
    QPainter painter(this);
    painter.drawImage(0, 0, image);
}

void Canvas::draw(const QPoint& from, const QPoint& to, const Brush* brush){
    QPainter painter(&image);

    QPoint p = {brush->texture.width()/2, brush->texture.height()/2};

    QPoint from_ = from - p;
    QPoint to_ = to - p;
    QPoint dir = to_ - from_;

    double distance = QLineF(from_, to_).length();
    double minDistance = 5;
    size_t count = (distance/minDistance);


    painter.drawImage(from_,brush->texture);

    for (int var = 1; var < count; ++var) {
        double t = (double)var/count;

        QPoint point = from_ + t * dir;

        painter.drawImage(point,brush->texture);

    }



    update();
}