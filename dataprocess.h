#ifndef DATAPROCESS_H
#define DATAPROCESS_H

#include <qtypes.h>
#include <vector>
#include <QImage>

class DataProcess
{
public:
    virtual ~DataProcess() = default;
    virtual std::vector<uchar> process(const QImage& image,const QImage& image_checkpoint,const std::vector<int>& chunks_ID,int chunk_w, int chunk_h) = 0;
    virtual void apply(QImage& image,const std::vector<int>& chunks_ID,int chunk_w, int chunk_h,const std::vector<uchar>& data) = 0;
};

class DataProcessXOR : public DataProcess
{
public:
    virtual std::vector<uchar> process(const QImage& image,const QImage& image_checkpoint,const std::vector<int>& chunks_ID,int chunk_w, int chunk_h) override;
    virtual void apply(QImage& image,const std::vector<int>& chunks_ID,int chunk_w, int chunk_h,const std::vector<uchar>& data) override;
};



#endif // DATAPROCESS_H
