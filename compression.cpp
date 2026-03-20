#include "compression.h"
#include <queue>
#include <QDebug>


template <class T>
std::vector<uchar> Huffman<T>::zip(const std::vector<uchar>& data){

    if(data.size()%sizeof(T)!=0){
        throw "bad data";
    }

    std::unordered_map<T, uint32_t> freq;

    DataView<const T> data_view(&data[0], data.size()/sizeof(T));

    for (auto& record : data_view) {
        freq[record]++;
    }


    std::vector<uchar> data_(8 + freq.size() * sizeof(FreqData));

    DataView<uint32_t> freqTableSize(&data_[0],1);
    freqTableSize[0] = freq.size();

    DataView<FreqData> freqTable(&data_[8],freq.size());

    int index = 0;
    for (auto &r : freq) {
        freqTable[index].data = r.first;
        freqTable[index].freq = r.second;
        ++index;

    }

    if(index == 1) {
        return data_;
    }

    Node* root = buildTree(freqTable);

    size_t size_raw = calculateDataSize(root);

    size_raw += 7; // 111
    size_raw >>= 3;

    DataView<uint32_t> dataSize(&data_[4],1);
    dataSize[0] = size_raw;

    if(size_raw + freqTable.size() * sizeof(FreqData) >= data.size()){
        freqTableSize[0] = 0;

        data_.resize(8 + data.size());
        std::copy(data.begin(), data.end(), data_.begin() + 8);

        deleteNodes(root);
        return data_;
    }

    std::unordered_map<T, std::vector<bool>> table;
    std::vector<bool> path;

    buildCodes(root, path, table);

    deleteNodes(root);

    data_.resize(data_.size() + size_raw);

    DataView<uchar> dataView(&data_[8 + freq.size() * sizeof(FreqData)],size_raw);

    compress(data_view, table, dataView);

    return data_;
}

template <class T>
std::vector<uchar> Huffman<T>::unzip(const std::vector<uchar>& data_){
    std::vector<uchar> data;

    DataView<const uint32_t> freqTableSize(&data_[0],1);

    if(freqTableSize[0] == 0) {
        data.resize(data_.size() - 8);
        std::copy(data_.begin() + 8, data_.end(), data.begin());
        return data;
    }

    DataView<const FreqData> freqTable(&data_[8],freqTableSize[0]);

    if(freqTableSize[0] == 1) {

        data.reserve(freqTable[0].freq * sizeof(T));

        DataView<T> out_view(&data[0],freqTable[0].freq);

        for (auto &record : out_view) {
            record = freqTable[0].data;
        }

        return data;
    }

    Node* root = buildTree(freqTable);

    DataView<const uint32_t> dataSize(&data_[4],1);

    data.resize(root->freq * sizeof(T));

    size_t data_index = 8+ freqTableSize[0] * sizeof(FreqData);

    DataView<const uchar> data_raw(&data_[data_index],data_.size()-data_index);

    DataView<T> data_view(&data[0], root->freq);

    decompress(data_raw, root, data_view);


    deleteNodes(root);


    return data;
}

template <class T>
typename Huffman<T>::Node* Huffman<T>::buildTree(const DataView<const FreqData>& freq) {
    std::priority_queue<Node*, std::vector<Node*>, Compare> pq;


    for (auto &r : freq) {
        pq.push(new Node(r.data, r.freq));
    }

    while (pq.size() > 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();

        Node* parent = new Node(0, left->freq + right->freq);
        parent->left = left;
        parent->right = right;

        pq.push(parent);
    }

    return pq.top();
}

template <class T>
void Huffman<T>::buildCodes(Node* node,
                std::vector<bool>& path,
                std::unordered_map<T, std::vector<bool>>& table) {

    if (!node->left && !node->right) {
        table[node->data] = path;
        return;
    }

    path.push_back(false);
    buildCodes(node->left, path, table);
    path.pop_back();

    path.push_back(true);
    buildCodes(node->right, path, table);
    path.pop_back();
}

template <class T>
void Huffman<T>::compress(
    const  DataView<const T>& input,
    const std::unordered_map<T, std::vector<bool>>& table,
    DataView<uchar>& output
    ) {

    uchar current = 0;
    int bitCount = 0;

    int index = 0;

    for (int i = 0; i < input.size(); ++i) {

        const auto& code = table.at(input[i]);

        for (bool bit : code) {
            current <<= 1;
            if (bit) current |= 1;

            bitCount++;

            if (bitCount == 8) {
                output[index++] = current;
                current = 0;
                bitCount = 0;
            }
        }
    }

    if (bitCount > 0) {
        current <<= (8 - bitCount);
        output[index++] = current;
    }
}

template <class T>
void Huffman<T>::decompress(
    const DataView<const uint8_t>& compressed,
    Node* root,
    DataView<T>& output
    ) {
    Node* current = root;

    int index = 0;
    for (uint8_t byte : compressed) {
        for (int i = 7; i >= 0; i--) {
            bool bit = (byte >> i) & 1;

            if (bit)
                current = current->right;
            else
                current = current->left;


            if (!current->left && !current->right) {

                output[index++] = current->data;

                if(index == root->freq){
                    return;
                }

                current = root;
            }
        }
    }
}

template <class T>
void Huffman<T>::deleteNodes(Node* node){
    if(node->left){
        deleteNodes(node->left);
    }
    if(node->right){
        deleteNodes(node->right);
    }

    delete node;
}

template <class T>
size_t Huffman<T>::calculateDataSize(Node* node,size_t layer){

    if(!node->left){
        return node->freq * layer;
    }


    return calculateDataSize(node->left,layer+1) + calculateDataSize(node->right,layer+1);
}

template <class T>
std::vector<uchar> Sequence<T>::zip(const  std::vector<uchar>& data){
    if(data.size()%sizeof(T)!=0){
        throw "bad data";
    }

    std::vector<uchar> data_;

    DataView<const T> data_view(&data[0], data.size()/sizeof(T));


    T lastRecord = data_view[0];
    uchar count = 1;

    size_t count_all = 0;
    for (int i = 1; i < data_view.size(); ++i) {
        if(lastRecord == data_view[i] && count != 255){
            ++count;
        }else{
            lastRecord = data_view[i];
            count = 1;
            ++count_all;
        }
    }

    if(count){
        ++count_all;
    }

    data_.resize(count_all * sizeof(SequenceCount));
    DataView<SequenceCount> dataView(&data_[0], count_all);

    lastRecord = data_view[0];
    count = 1;
    int index = 0;
    for (int i = 1; i < data_view.size(); ++i) {
        if(lastRecord == data_view[i] && count != 255){
            ++count;
        }else{
            dataView[index].data = lastRecord;
            dataView[index].count = count;
            ++index;

            lastRecord = data_view[i];
            count = 1;
        }
    }

    if(count){
        dataView[index].data = lastRecord;
        dataView[index].count = count;
    }

    return data_;

}

template <class T>
std::vector<uchar> Sequence<T>::unzip(const  std::vector<uchar>& data_){
    std::vector<uchar> data;

    DataView<const SequenceCount> data_view(&data_[0], data_.size()/sizeof(SequenceCount));

    size_t count = 0;
    for (auto& record : data_view) {
        count += record.count;
    }
    data.resize(count * sizeof(T));
    DataView<T> dataView(&data[0], count);
    int index = 0;
    for (auto& record : data_view) {
        for (int i = 0; i < record.count; ++i) {
            dataView[index++] = record.data;
        }
    }

    return data;
}


std::vector<uchar> Sequence2::zip(const  std::vector<uchar>& data){
    std::vector<uchar> data_;
    std::vector<bool> pom;


    data_.push_back(0);
    data_.push_back(0);
    data_.push_back(0);
    data_.push_back(0);

    pom.push_back(false);
    data_.push_back(data[0]);
    for (int i = 1; i < data.size(); ++i) {

        uchar best_result = 0;
        uchar best_offset = 0;


        for (int j = 0; j < 256; ++j) {
            if(i-j==0)break;
            for (int c = 0; c < 256; ++c) {
                if(i+c==data.size())break;
                if(data[i - j - 1 + c] != data[i + c]){
                    break;
                }

                if(c > best_result){
                    best_result = c;
                    best_offset = j;
                }
            }
        }

        if(best_result > 1){
            pom.push_back(true);
            data_.push_back(best_offset);
            data_.push_back(best_result);

            i += best_result;
        } else{
            pom.push_back(false);
            data_.push_back(data[i]);
        }
    }

    DataView<uint32_t> pomSizeV(&data_[0],1);
    uint32_t pomSize = (pom.size() + 7) >>3;
    pomSizeV[0] = pomSize;

    data_.reserve(data_.size() + pomSize);

    for (int i = 0; i < pomSize; ++i) {
        uchar record = 0;
        for (int j = 0; j < 8; ++j) {
            if ((i<<3)+j == pom.size()){
                record <<= (8-j);
                break;
            }

            record <<= 1;

            if (pom[(i<<3)+j]){
                record |= 1;
            }
        }
        data_.push_back(record);
    }

    return data_;

}

std::vector<uchar> Sequence2::unzip(const  std::vector<uchar>& data_){
    std::vector<uchar> data;


    DataView<const uint32_t> pomSizeV(&data_[0],1);
    uint32_t pomSize = pomSizeV[0];

    uint32_t dataSize = data_.size()-pomSize - 4;

    DataView<const uchar> pomView(&data_[data_.size()-pomSize],pomSize);

    DataView<const uchar> dataView(&data_[4],dataSize);

    int pomIndex = 0;

    for (int i = 0; i < dataSize; ++i) {

        bool tag =pomView[pomIndex>>3] & (128>>(pomIndex&7));

        if (tag){
            uchar offset = dataView[i];
            uchar size = dataView[i +1];
            int start = data.size() - 1 - offset;

            for (int j = 0; j <= size; ++j) {
                data.push_back(data[start + j]);
            }
            ++i;
        }else{
            data.push_back(dataView[i]);
        }
        ++pomIndex;
    }


    return data;
}

template class Huffman<uchar>;
template class Huffman<uint32_t>;


template class Sequence<uchar>;
template class Sequence<uint32_t>;


