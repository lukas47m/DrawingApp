#include "compression.h"
#include <queue>
#include <QDebug>

template <class T>
size_t Huffman<T>::size(){
    size_t size = 8; // 2*4 (data_ / freqTable).size()
    size += data_.size();
    size += freqTable.size() * sizeof(FreqData);
    return size;
}

template <class T>
void Huffman<T>::zip(const  std::vector<uchar>& data){

    if(data.size()%sizeof(T)!=0){
        throw "bad data";
    }

    std::unordered_map<T, uint32_t> freq;

    for (int i = 0; i < data.size(); i+= sizeof(T)) {
        Pom record;
        for (int j = 0; j < sizeof(T); ++j) {
            record.d[j] = data[i + j];
        }
        freq[record.t]++;
    }

    freqTable.resize(freq.size());
    int index = 0;
    for (auto &r : freq) {
        freqTable[index].data = r.first;
        freqTable[index].freq = r.second;
        ++index;

    }
    data_.clear();

    if(index == 1) {
        return;
    }


    Node* root = buildTree(freqTable);

    size_t size_raw = calculateDataSize(root);

    size_raw += 7; // 111
    size_raw >>= 3;

    if(size_raw + freqTable.size() * sizeof(FreqData) >= data.size()){
        data_ = data;
        freqTable.clear();
        return;
    }

    std::unordered_map<T, std::vector<bool>> table;
    std::vector<bool> path;

    buildCodes(root, path, table);

    deleteNodes(root);

    data_.reserve(size_raw);
    compress(data, table, data_);
}

template <class T>
std::vector<uchar> Huffman<T>::unzip(){
    std::vector<uchar> data;

    if(freqTable.size() == 0) {
        return data_;
    }

    if(freqTable.size() == 1) {
        data.reserve(freqTable[0].freq * sizeof(T));
        Pom pom;
        pom.t = freqTable[0].data;

        for (int i = 0; i < freqTable[0].freq; ++i) {
            data.insert(data.end(),pom.d,pom.d + sizeof(T));
        }
        return data;
    }

    Node* root = buildTree(freqTable);

    data.reserve(root->freq * sizeof(T));

    decompress(data_, root, data);


    deleteNodes(root);


    return data;
}

template <class T>
typename Huffman<T>::Node* Huffman<T>::buildTree(const std::vector<FreqData>& freq) {
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
    const std::vector<uchar>& input,
    const std::unordered_map<T, std::vector<bool>>& table,
    std::vector<uchar>& output
    ) {

    uchar current = 0;
    int bitCount = 0;

    for (int i = 0; i < input.size(); i+= sizeof(T)) {
        Pom record;
        for (int j = 0; j < sizeof(T); ++j) {
            record.d[j] = input[i + j];
        }
        const auto& code = table.at(record.t);

        for (bool bit : code) {
            current <<= 1;
            if (bit) current |= 1;

            bitCount++;

            if (bitCount == 8) {
                output.push_back(current);
                current = 0;
                bitCount = 0;
            }
        }
    }

    if (bitCount > 0) {
        current <<= (8 - bitCount);
        output.push_back(current);
    }
}

template <class T>
void Huffman<T>::decompress(
    const std::vector<uint8_t>& compressed,
    Node* root,
    std::vector<uint8_t>& output
    ) {
    Node* current = root;

    for (uint8_t byte : compressed) {
        for (int i = 7; i >= 0; i--) {
            bool bit = (byte >> i) & 1;

            if (bit)
                current = current->right;
            else
                current = current->left;


            if (!current->left && !current->right) {
                Pom pom;
                pom.t = current->data;

                output.insert(output.end(),pom.d,pom.d + sizeof(T));

                if(output.size() == root->freq * sizeof(T)){
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


template class Huffman<unsigned char>;
template class Huffman<unsigned int>;

