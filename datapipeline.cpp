#include "datapipeline.h"


std::vector<uchar> DataGroupechanels::forward(const std::vector<uchar>& data, int chunk_w, int chunk_h){
    std::vector<uchar> data_(data.size());
    size_t offset_size = data.size() >>2;

    for (int i = 0; i < data.size(); i += 4) {
        int i_ = i >>2;
        data_[i_] = data[i];
        data_[i_ + offset_size] = data[i + 1];
        data_[i_ + offset_size * 2] = data[i + 2];
        data_[i_ + offset_size * 3] = data[i + 3];
    }

    return data_;
}
std::vector<uchar> DataGroupechanels::back(const std::vector<uchar>& data, int chunk_w, int chunk_h) {
    std::vector<uchar> data_(data.size());
    size_t offset_size = data.size() >>2;

    for (int i = 0; i < data.size(); i += 4) {
        int i_ = i >>2;
        data_[i] = data[i_];
        data_[i + 1] = data[i_ + offset_size];
        data_[i + 2] = data[i_ + offset_size * 2];
        data_[i + 3] = data[i_ + offset_size * 3];
    }

    return data_;
}

std::vector<uchar> PaethFilter::forward(const std::vector<uchar>& data, int chunk_w, int chunk_h){
    std::vector<uchar> data_ = data;
    size_t chunk_size = chunk_w* chunk_h * 4;

    size_t chunks_num = data.size()/chunk_size;


    for (int i = 0; i < chunks_num; ++i) {
        for (int x = 1; x < chunk_w; ++x) {
            for (int y = 1; y < chunk_h; ++y) {
                for (int j = 0; j < 4; ++j) {
                    int index = i*chunk_size + (y * chunk_h + x) * 4 + j;
                    int index_a = i*chunk_size + (y * chunk_h + (x-1))* 4 + j;
                    int index_b = i*chunk_size + ((y-1) * chunk_h + x)* 4 + j;
                    int index_c = i*chunk_size + ((y-1) * chunk_h + (x-1))* 4 + j;

                    data_[index] -= paethPredictor(data[index_a],data[index_b],data[index_c]);

                }
            }
        }
    }

    return data_;
}

std::vector<uchar> PaethFilter::back(const std::vector<uchar>& data, int chunk_w, int chunk_h) {
    std::vector<uchar> data_ = data;
    size_t chunk_size = chunk_w* chunk_h * 4;

    size_t chunks_num = data.size()/chunk_size;


    for (int i = 0; i < chunks_num; ++i) {
        for (int x = 1; x < chunk_w; ++x) {
            for (int y = 1; y < chunk_h; ++y) {
                for (int j = 0; j < 4; ++j) {
                    int index = i*chunk_size + (y * chunk_h + x) * 4 + j;
                    int index_a = i*chunk_size + (y * chunk_h + (x-1))* 4 + j;
                    int index_b = i*chunk_size + ((y-1) * chunk_h + x)* 4 + j;
                    int index_c = i*chunk_size + ((y-1) * chunk_h + (x-1))* 4 + j;


                    data_[index] += paethPredictor(data_[index_a],data_[index_b],data_[index_c]);
                }
            }
        }
    }

    return data_;
}


uchar PaethFilter::paethPredictor(int a, int b, int c) {
    int p = a + b - c;
    int pa = std::abs(p - a);
    int pb = std::abs(p - b);
    int pc = std::abs(p - c);

    if (pa <= pb && pa <= pc)
        return a;
    else if (pb <= pc)
        return b;
    else
        return c;
}

