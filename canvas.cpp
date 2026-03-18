#include "canvas.h"
#include <QDebug>

Canvas::Canvas(size_t width, size_t height,QWidget *parent) : QWidget(parent),image(width, height, QImage::Format_RGBA8888)
{

    image.fill(QColor(255, 255, 255, 255));

    doCheckpoint();

    setFixedSize(width, height);

    setChunkSize(16,16);

}
Canvas::~Canvas(){
    for(auto& action : actionBuffer) {
        delete action;
    }
}

void Canvas::setChunkSize(size_t w,size_t h){
    size_t image_w = image.width();
    size_t image_h = image.height();

    ch_w = w;
    ch_h = h;

    size_t ch_w_count = image_w / w;
    size_t ch_h_count = image_h / h;

    if(image_w % w != 0){
        ++ch_w_count;
    }
    if(image_h % h != 0){
        ++ch_h_count;
    }

    changedChunks.resize(ch_w_count);

    for (int var = 0; var < changedChunks.size(); ++var) {
        changedChunks[var].resize(ch_h_count);
    }
}

void Canvas::doCheckpoint(){
    image_checkpoint = image;
    for (int var = 0; var < changedChunks.size(); ++var) {
        changedChunks[var].fill(false);
    }
}

void Canvas::saveDiferencies(){
    DiffData* data = new DiffData();

    data->strategy = {DiffData::Strategy({
        new PaethFilter(),
        new DataGroupechanels(),
        //new Sequence<uint32_t>(),
        new Sequence<uchar>(),
        new Huffman<uchar>(),
        //new QTCompresion()
    }),DiffData::Strategy({
        new Huffman<uchar>(),
    }),DiffData::Strategy({
        new DataGroupechanels(),
        new Sequence<uchar>(),
    }),DiffData::Strategy({
        new QTCompresion()
    })};

    data->chunk_w = ch_w;
    data->chunk_h = ch_h;

    size_t count = 0;
    for(int i = 0; i < changedChunks.size(); ++i) {
        count += changedChunks[i].count(true);
    }
    data->chunks_ID.resize(count);
    int index = 0;
    for(int i = 0; i < changedChunks.size(); ++i) {
        for(int j = 0; j < changedChunks[0].size(); ++j) {
            if(changedChunks[i][j]) {
                int chunkID = i * changedChunks[0].size() + j;
                data->chunks_ID[index] = chunkID;

                ++index;
            }
        }
    }

    DataProcessXOR processData;

    std::vector<uchar> data_raw = processData.process(image, image_checkpoint, data->chunks_ID, data->chunk_w, data->chunk_h);

    zip(data, data_raw);

    addCheckpoint(data);
}

void Canvas::undo(){
    if (undoIndex > 0) {
        undoIndex--;
        DiffData* data = actionBuffer.get(undoIndex);
        applyChanges(data);
    }
}
void Canvas::redo(){
    if (undoIndex < actionBuffer.size()) {
        DiffData* data = actionBuffer.get(undoIndex);
        applyChanges(data);
        undoIndex++;
    }
}


void Canvas::addCheckpoint(DiffData* data) {
    size_t size = actionBuffer.size() - undoIndex;

    for(size_t i = 0; i < size; i++) {
        delete actionBuffer.pop_back();
    }
    bool isFull = actionBuffer.isFull();
    if(isFull) {
        delete actionBuffer.pop_front();
    }
    actionBuffer.add(data);
    undoIndex = actionBuffer.size();
}
void Canvas::applyChanges(DiffData* data) {


    std::vector<uchar> data_raw = unZip(data);

    DataProcessXOR processData;

    processData.apply(image, data->chunks_ID, data->chunk_w, data->chunk_h, data_raw);

    doCheckpoint();

    update();
}

void Canvas::paintEvent(QPaintEvent * p)
{
    QPainter painter(this);
    painter.drawImage(0, 0, image);
}

void Canvas::draw(const QPoint& from, const QPoint& to, const Brush* brush){
    QPainter painter(&image);

    QPoint p = {brush->texture.width()/2, brush->texture.height()/2};

    QPoint from_ = from - p;
    QPoint to_ = to - p;
    QPoint dir = to_ - from_;

    double distance = QLineF(from_, to_).length();
    double minDistance = 5;
    size_t count = (distance/minDistance);


    painter.drawImage(from_,brush->texture);

    activateChunks(from_, from_ + QPoint{brush->texture.width(), brush->texture.height()});

    for (int var = 1; var < count; ++var) {
        double t = (double)var/count;

        QPoint point = from_ + t * dir;

        painter.drawImage(point,brush->texture);

        activateChunks(point, point + QPoint{brush->texture.width(), brush->texture.height()});

    }



    update();
}

QPoint clampPoint(QPoint p, QPoint min, QPoint max)
{
    return QPoint(
        std::clamp(p.x(), min.x(), max.x()),
        std::clamp(p.y(), min.y(), max.y())
        );
}

void Canvas::activateChunks(const QPoint& from, const QPoint& to){
    QPoint max = QPoint(changedChunks.size() - 1,changedChunks[0].size()-1);
    QPoint from_ = clampPoint({from.x()/ch_w, from.y()/ch_h},{0,0}, max);
    QPoint to_ = clampPoint({to.x()/ch_w, to.y()/ch_h},{0,0}, max);

    for (int i = from_.x(); i <= to_.x(); ++i) {
        for (int j = from_.y(); j <= to_.y(); ++j) {
            changedChunks[i][j] = true;
        }
    }
}

void Canvas::zip(DiffData* diffData, const std::vector<uchar>& data){

    diffData->zip(data);
    diffData->originalSize = data.size();

    diffData->printStat();
}
std::vector<uchar> Canvas::unZip(DiffData* diffData){
    std::vector<uchar> data;

    data = diffData->unzip();

    diffData->printStat();

    return data;
}

void Canvas::saveImage(const QString& path){
    image.save(path);
}
