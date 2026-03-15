#ifndef BRUSH_H
#define BRUSH_H

#include <QImage>
#include <QWidget>


class Brush
{
public:
    Brush(size_t w = 50, size_t h = 50);

    void setSize(size_t w, size_t h);

    bool loadTexture(const std::string& path);

    QImage texture;
private:
};

#endif // BRUSH_H
