#include "brush.h"

#include <QPainter>

Brush::Brush(size_t w, size_t h) : texture(w,h, QImage::Format_RGBA8888) {
    texture.fill(QColor(255, 255, 255, 255));
}

void Brush::setSize(size_t w, size_t h){
    texture = QImage(w,h, QImage::Format_RGBA8888);
}

bool Brush::loadTexture(const std::string& path){
    return texture.load(QString::fromStdString(path));
}