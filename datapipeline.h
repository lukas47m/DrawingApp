#ifndef DATAPIPELINE_H
#define DATAPIPELINE_H


#include <vector>
#include <qtypes.h>

class DataPipeLine
{
public:
    virtual ~DataPipeLine() = default;

    virtual std::vector<uchar> forward(const std::vector<uchar>& data, int chunk_w, int chunk_h) = 0;
    virtual std::vector<uchar> back(const std::vector<uchar>& data, int chunk_w, int chunk_h) = 0;
};

class DataGroupechanels : public DataPipeLine
{
public:

    virtual std::vector<uchar> forward(const std::vector<uchar>& data, int chunk_w, int chunk_h) override;
    virtual std::vector<uchar> back(const std::vector<uchar>& data, int chunk_w, int chunk_h) override;
};

class PaethFilter : public DataPipeLine
{
public:

    virtual std::vector<uchar> forward(const std::vector<uchar>& data, int chunk_w, int chunk_h) override;
    virtual std::vector<uchar> back(const std::vector<uchar>& data, int chunk_w, int chunk_h) override;

private:
    uchar paethPredictor(int a, int b, int c);
};



#endif // DATAPIPELINE_H
