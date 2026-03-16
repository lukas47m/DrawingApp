#pragma once

#include <vector>
#include <vector>
#include <qtypes.h>
#include <unordered_map>

class Compression
{
public:
    Compression() = default;
    virtual ~Compression() = default;

    virtual size_t size() = 0;

    virtual void zip(const  std::vector<uchar>&) = 0;
    virtual std::vector<uchar> unzip() = 0;
};

template <class T>
class Huffman : public Compression {
public:

    virtual size_t size() override;

    virtual void zip(const  std::vector<uchar>&) override;
    virtual std::vector<uchar> unzip() override;
private:
    struct FreqData{
        T data;
        uint32_t freq;
    };

    union Pom{
        T t;
        uchar d[sizeof(T)];
    };


    std::vector<uchar> data_;
    std::vector<FreqData> freqTable;

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

    Node* buildTree(const std::vector<FreqData>& freq);

    void buildCodes(Node* node,
                    std::vector<bool>& path,
                    std::unordered_map<T, std::vector<bool>>& table);

    void compress(
        const std::vector<uchar>& input,
        const std::unordered_map<T, std::vector<bool>>& table,
        std::vector<uchar>& out
        );

    void decompress(
        const std::vector<uint8_t>& compressed,
        Node* root,
        std::vector<uchar>& out
        );

    void deleteNodes(Node* node);

    size_t calculateDataSize(Node* node, size_t layer = 0);
};

