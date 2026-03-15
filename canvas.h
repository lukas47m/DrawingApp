#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QDebug>
#include "brush.h"

class Canvas : public QWidget
{
    Q_OBJECT

public:
    Canvas(size_t w,size_t h, QWidget *parent = nullptr);

    void undo();
    void redo();

    void draw(const QPoint& from, const QPoint& to, const Brush* brush);

protected:
    void paintEvent(QPaintEvent *) override;


private:
    size_t w, h;
    QImage image;
};

#endif // CANVAS_H
