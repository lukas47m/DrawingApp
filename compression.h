#pragma once

#include <vector>
#include <qtypes.h>
#include <unordered_map>

template<class T>
class DataView{
public:
    template<class U>
    DataView(U* pointer,size_t size) :pointer(reinterpret_cast<T*>(pointer)), size_(size)  {}

    DataView(const DataView<std::remove_const_t<T>>& other)
        : pointer(&other[0]), size_(other.size()) {}

    class Iterator {
    public:
        T* pointer;
        Iterator(T* pointer) : pointer(pointer) {}
        T& operator*() {
            return *pointer;
        }
        const T& operator*() const {
            return *pointer;
        }
        Iterator& operator++() {
            pointer++;
            return *this;
        }
        Iterator& operator--() {
            pointer--;
            return *this;
        }
        bool operator==(const Iterator& other) const {
            return pointer == other.pointer;
        }
        bool operator!=(const Iterator& other) const {
            return pointer != other.pointer;
        }
    };
    const Iterator begin() const {
        return Iterator(pointer);
    }
    const Iterator end() const {
        return Iterator(pointer + size_);
    }

    Iterator begin() {
        return Iterator(pointer);
    }
    Iterator end() {
        return Iterator(pointer + size_);
    }

    T& operator[](size_t i) { return pointer[i]; }
    const T& operator[](size_t i) const{ return pointer[i]; }

    size_t size() const { return size_; }

private:
    T* pointer;
    size_t size_;
};


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


