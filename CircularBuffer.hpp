#pragma once

#include <cstddef>

template<typename T, size_t MAX_SIZE>
class CircularBuffer {
public:
    size_t head = 0;
    size_t tail = 0;
    T buffer[MAX_SIZE]; 
    CircularBuffer() {}
    T add(const T& value) {
        T oldValue = buffer[tail];
        buffer[tail] = value;
        tail = (tail + 1) % MAX_SIZE;
        if (tail == head) {
            head = (head + 1) % MAX_SIZE;
        }
        return oldValue;
    }
    T back() {
        T value = buffer[tail - 1];
        return value;
    }
    T pop_back() {
        T value = buffer[tail - 1];
        tail = (tail - 1) % MAX_SIZE;
        return value;
    }
    T get(size_t index) const {
        return buffer[(head + index) % MAX_SIZE];
    }
    size_t size() const {
        return (tail - head + MAX_SIZE) % MAX_SIZE;
    }
    bool isEmpty() const {
        return head == tail;
    }
    bool isFull() const {
        return (tail + 1) % MAX_SIZE == head;
    }

    class Iterator {
    public:
        CircularBuffer* buffer;
        size_t index;
        Iterator(CircularBuffer* buffer, size_t index) : buffer(buffer), index(index) {}
        T& operator*() {
            return buffer->buffer[index];
        }
        Iterator& operator++() {
            index = (index + 1) % MAX_SIZE;
            return *this;
        }
        Iterator& operator--() {
            index = (index - 1 + MAX_SIZE) % MAX_SIZE;
            return *this;
        }
        bool operator==(const Iterator& other) const {
            return index == other.index;
        }
        bool operator!=(const Iterator& other) const {
            return index != other.index;
        }
    };
    Iterator begin() {
        return Iterator(this, head);
    }
    Iterator end() {
        return Iterator(this, tail);
    }
};