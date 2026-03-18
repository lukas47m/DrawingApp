#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <QBitArray>
#include <QElapsedTimer>
#include "brush.h"
#include "CircularBuffer.hpp"
#include "compression.h"
#include "dataprocess.h"


class DiffData{
public:
    size_t originalSize;
    class Strategy{
    public:
        std::vector<uchar> data;
        size_t compressionTime;
        size_t deCompressionTime = 0;
        std::vector<DataPipeLine*> pipeline;

        Strategy(const std::vector<DataPipeLine*>& pipeline) : pipeline(pipeline){}
    };

    void printStat(){
        qDebug() << "###########################";
        qDebug() << "original:" << originalSize;
        for (int i = 0; i < strategy.size(); ++i) {
            qDebug() << i<< ":--------------------------";
            qDebug() << i <<":"<<strategy[i].data.size()<<(1.0*originalSize)/strategy[i].data.size();
            qDebug() << i <<":"<<strategy[i].compressionTime<<strategy[i].deCompressionTime;
            qDebug() << i <<":"<<strategy[i].compressionTime*1.e-9<<strategy[i].deCompressionTime*1.e-9;
            qDebug() << "-----------------------------";
        }
        qDebug() << "###########################";
    }

    std::vector<int> chunks_ID;
    int chunk_w, chunk_h;

    std::vector<Strategy> strategy;

    ~DiffData(){

        for (auto&s : strategy) {

            for (auto& p : s.pipeline) {
                delete p;
            }
        }
    }

    void zip(const std::vector<uchar>& data){

        for (auto&s : strategy) {
            QElapsedTimer t;
            t.start();
            std::vector<uchar> data_ = data;
            for (auto& pipe : s.pipeline) {
                data_ = pipe->forward(data_,chunk_w, chunk_h);
            }
            s.data = data_;

            s.compressionTime = t.nsecsElapsed();
        }
    }

    std::vector<uchar> unzip(){

        for (int j = 0; j < strategy.size(); ++j) {

            QElapsedTimer t;
            t.start();

            std::vector<uchar> data_ = strategy[j].data;
            for (int i = strategy[j].pipeline.size() - 1; i >= 0; --i) {
                data_ = strategy[j].pipeline[i]->back(data_,chunk_w, chunk_h);
            }

            if (strategy[j].deCompressionTime != 0){
                return data_;
            }

            strategy[j].deCompressionTime = t.nsecsElapsed();

            if (j == strategy.size()-1 ){
                return data_;
            }
        }
        return {};
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
