#include "dataprocess.h"
#include "DataView.h"



std::vector<uchar> DataProcessXOR::process(const QImage& image,const QImage& image_checkpoint,const std::vector<int>& chunks_ID,int chunk_w, int chunk_h){
    std::vector<uchar> data_raw;


    size_t image_h = image.height();
    size_t ch_h_count = image_h / chunk_h;

    if(image_h % chunk_h != 0){
        ++ch_h_count;
    }

    data_raw.resize(chunks_ID.size() * 4 * chunk_w * chunk_h);
    DataView<int32_t> dataView(&data_raw[0], chunks_ID.size() * chunk_w * chunk_h);


    int index = 0;
    for(int chunkID : chunks_ID) {
        int j = chunkID % ch_h_count;
        int i = (chunkID - j ) / ch_h_count;

        QRect area(i * chunk_w, j * chunk_h,chunk_w,chunk_h);

        int dataIndex = index * chunk_w * chunk_h;

        for(int y = area.top(); y <= area.bottom(); ++y) {

            if(y >= image.height()){
                for(int x = 0; x < chunk_w; ++x) {
                    dataView[dataIndex++] = 0;
                }
                continue;
            }

            const uchar* line1 = image.constScanLine(y);
            const uchar* line2 = image_checkpoint.constScanLine(y);

            DataView<const int32_t> dataViewImage(line1 + (area.left() * 4), chunk_w);
            DataView<const int32_t> dataViewImageCheckpoint(line2+ (area.left() * 4), chunk_w);

            for(int x = 0; x < chunk_w; ++x) {
                if(area.left() + x >= image.width()){
                    for(int x = 0; x < chunk_w; ++x) {
                        dataView[dataIndex++] = 0;
                    }
                    continue;
                }
                dataView[dataIndex++] = dataViewImage[x] ^ dataViewImageCheckpoint[x];
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


    DataView<const int32_t> dataView(&data[0], data.size()/4);

    for (int i = 0; i < chunks_ID.size(); ++i) {
        int index = chunks_ID[i];
        int y = index % ch_h_count;
        int x = (index - y ) / ch_h_count;

        QRect area(x * chunk_w,y * chunk_h,chunk_w,chunk_h);


        int dataIndex = i * chunk_w * chunk_h;
        for(int y = area.top(); y <= area.bottom(); ++y) {

            if(y >= image.height()){
                continue;
            }


            uchar* line = image.scanLine(y);

            DataView<int32_t> dataViewImage(line + (area.left() * 4), chunk_w);

            for(int x = 0;  x< chunk_w; ++x) {
                if(area.left() + x >= image.width()){
                    continue;
                }
                dataViewImage[x] ^= dataView[dataIndex++];
            }
        }


    }
}

