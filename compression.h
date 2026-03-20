#pragma once

#include <vector>
#include <qtypes.h>
#include <unordered_map>
#include "DataView.h"
#include "datapipeline.h"
#include <QByteArray>


class Compression :  public DataPipeLine
{
public:
    Compression() = default;
    virtual ~Compression() = default;

    virtual std::vector<uchar> zip(const  std::vector<uchar>&) = 0;
    virtual std::vector<uchar> unzip(const std::vector<uchar>&) = 0;

    virtual std::vector<uchar> forward(const std::vector<uchar>& data, int chunk_w, int chunk_h) override {
        return zip(data);
    }
    virtual std::vector<uchar> back(const std::vector<uchar>& data, int chunk_w, int chunk_h) override {
        return unzip(data);
    }
};
template <class T>
class Huffman : public Compression {
public:

    virtual std::vector<uchar> zip(const  std::vector<uchar>&) override;
    virtual std::vector<uchar> unzip(const  std::vector<uchar>&) override;
private:
    struct FreqData{
        T data;
        uint32_t freq;
    };

    struct Node {
        T data;
        uint32_t freq;
        Node* left;
        Node* right;

        Node(T b, uint32_t f) : data(b), freq(f), left(nullptr), right(nullptr) {}
    };

    struct Compare {
        bool operator()(Node* a, Node* b) {
            if(a->freq == b->freq){
                return a->data > b->data;
            }
            return a->freq > b->freq;
        }
    };

    Node* buildTree(const DataView<const FreqData>& freq);

    void buildCodes(Node* node,
                    std::vector<bool>& path,
                    std::unordered_map<T, std::vector<bool>>& table);

    void compress(
        const DataView<const T>& input,
        const std::unordered_map<T, std::vector<bool>>& table,
        DataView<uchar>& out
        );

    void decompress(
        const DataView<const uint8_t>& compressed,
        Node* root,
        DataView<T>& out
        );

    void deleteNodes(Node* node);

    size_t calculateDataSize(Node* node, size_t layer = 0);
};


template <class T>
class Sequence : public Compression {
public:

    virtual std::vector<uchar> zip(const  std::vector<uchar>&) override;
    virtual std::vector<uchar> unzip(const  std::vector<uchar>&) override;

private:
    class SequenceCount{
    public:
        uchar count;
        T data;
    };

};


class Sequence2 : public Compression {
public:

    virtual std::vector<uchar> zip(const  std::vector<uchar>&) override;
    virtual std::vector<uchar> unzip(const  std::vector<uchar>&) override;

private:

};

class QTCompresion : public Compression {
public:

    virtual std::vector<uchar> zip(const  std::vector<uchar>& data) {
        QByteArray compressed = qCompress(data.data(), data.size(), 9);

        return std::vector<uchar>(
            compressed.begin(),
            compressed.end()
            );
    };
    virtual std::vector<uchar> unzip(const  std::vector<uchar>& data) {
        QByteArray uncompressed = qUncompress(data.data(), data.size());

        return std::vector<uchar>(
            uncompressed.begin(),
            uncompressed.end()
            );

    };
};

