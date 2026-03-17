#ifndef DATAVIEW_H
#define DATAVIEW_H


#include <vector>

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

#endif // DATAVIEW_H
