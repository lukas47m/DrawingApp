#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <QBitArray>
#include "brush.h"
#include "CircularBuffer.hpp"
#include "compression.h"
#include "dataprocess.h"


class DiffData{
public:
    std::vector<int> chunks_ID;
    std::vector<DataPipeLine*> pipeline;
    int chunk_w, chunk_h;

    std::vector<uchar> data;

    ~DiffData(){
        for (auto& p : pipeline) {
            delete p;
        }
    }

    void zip(const std::vector<uchar>& data){
        std::vector<uchar> data_ = data;
        for (auto& pipe : pipeline) {
            data_ = pipe->forward(data_,chunk_w, chunk_h);
        }
        this->data = data_;
    }

    std::vector<uchar> unzip(){
        std::vector<uchar> data_ = data;
        for (int i = pipeline.size() - 1; i >= 0; --i) {
            data_ = pipeline[i]->back(data_,chunk_w, chunk_h);
        }
        return data_;
    }
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

    void saveImage(const QString& path);

protected:
    void paintEvent(QPaintEvent *) override;



private:
    int ch_w, ch_h;
    QImage image;

    QImage image_checkpoint;

    std::vector<QBitArray> changedChunks;

    CircularBuffer<DiffData*,256> actionBuffer;
    size_t undoIndex = 0;
    void addCheckpoint(DiffData* data);
    void applyChanges(DiffData* data);
    void activateChunks(const QPoint& from, const QPoint& to);

    void zip(DiffData* diffData, const std::vector<uchar>& data);
    std::vector<uchar> unZip(DiffData* diffData);
};

#endif // CANVAS_H
