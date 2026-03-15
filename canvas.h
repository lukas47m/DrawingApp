#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <QBitArray>
#include "brush.h"
#include "CircularBuffer.hpp"

class DiffData{
public:
    std::vector<int> chunks_ID;
    std::vector<uchar> data;
    int chunk_w, chunk_h;
};

class Canvas : public QWidget
{
    Q_OBJECT

public:
    Canvas(size_t w,size_t h, QWidget *parent = nullptr);
    virtual ~Canvas() override;

    void undo();
    void redo();

    void draw(const QPoint& from, const QPoint& to, const Brush* brush);


    void setChunkSize(size_t w,size_t h);

    void doCheckpoint();

    void saveDiferencies();

protected:
    void paintEvent(QPaintEvent *) override;



private:
    int ch_w, ch_h;
    QImage image;

    QImage image_checkpoint;

    std::vector<QBitArray> changedChunks;

    CircularBuffer<DiffData*,50> actionBuffer;
    size_t undoIndex = 0;
    void addCheckpoint(DiffData* data);
    void applyChanges(DiffData* data);
    void activateChunks(const QPoint& from, const QPoint& to);
};

#endif // CANVAS_H
