#include "dataprocess.h"



std::vector<uchar> DataProcessXOR::process(const QImage& image,const QImage& image_checkpoint,const std::vector<int>& chunks_ID,int chunk_w, int chunk_h){
    std::vector<uchar> data_raw;


    size_t image_h = image.height();
    size_t ch_h_count = image_h / chunk_h;

    if(image_h % chunk_h != 0){
        ++ch_h_count;
    }

    data_raw.resize(chunks_ID.size() * 4 * chunk_w * chunk_h);

    int index = 0;
    for(int chunkID : chunks_ID) {
        int j = chunkID % ch_h_count;
        int i = (chunkID - j ) / ch_h_count;

        QRect area(i * chunk_w, j * chunk_h,chunk_w,chunk_h);

        int dataIndex = index * chunk_w * chunk_h * 4;

        for(int y = area.top(); y <= area.bottom(); ++y) {

            const uchar* line1 = image.constScanLine(y);
            const uchar* line2 = image_checkpoint.constScanLine(y);

            int start = area.left() * 4;
            int end   = area.right() * 4 + 3;

            for(int x = start; x <= end; ++x) {
                data_raw[dataIndex++] = line1[x] ^ line2[x];
            }
        }
        ++index;
    }

    return data_raw;
}

void DataProcessXOR::apply(QImage& image,const std::vector<int>& chunks_ID,int chunk_w, int chunk_h,const std::vector<uchar>& data){

    size_t image_h = image.height();
    size_t ch_h_count = image_h / chunk_h;

    if(image_h % chunk_h != 0){
        ++ch_h_count;
    }

    for (int i = 0; i < chunks_ID.size(); ++i) {
        int index = chunks_ID[i];
        int y = index % ch_h_count;
        int x = (index - y ) / ch_h_count;

        QRect area(x * chunk_w,y * chunk_h,chunk_w,chunk_h);


        int dataIndex = i * chunk_w * chunk_h * 4;
        for(int y = area.top(); y <= area.bottom(); ++y) {

            uchar* line = image.scanLine(y);

            int start = area.left() * 4;
            int end   = area.right() * 4 + 3;

            for(int x = start;  x<= end; ++x) {
                line[x] ^= data[dataIndex++];
            }
        }


    }
}
