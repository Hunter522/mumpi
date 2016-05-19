//
//  RingBuffer.hpp
//  mumpi
//
//  Created by <author> on 12/05/2016.
//
//

#ifndef RingBuffer_hpp
#define RingBuffer_hpp

#include <mutex>

/**
 * Simple unbounded circular buffer implementation. Thread-safe.
 */
template <class T>
class RingBuffer {

public:
    RingBuffer(size_t size);
    ~RingBuffer();

    // member functions
    T top();
    size_t top(T* dest, int offset, size_t len);
    size_t topRemaining(T* outbuf);
    void push(T val);
    void push(T* src, int offset, size_t len);
    bool isEmpty();
    size_t getSize() const { return _size; };
    size_t getRemaining() const;
    unsigned int getFront() const { return _front; }
    unsigned int getBack() const { return _back; }
private:
    T* _array;
    size_t _size;
    unsigned int _front;
    unsigned int _back;
    size_t _remaining;
    std::recursive_mutex _mutex;
};

///////////////////////////
// TEMPLATE IMPLEMENTATION
///////////////////////////

/**
 * @brief Default constructor
 *
 * @param size size of ring buffer
 * @tparam T Type the RingBuffer should hold
 */
template <typename T>
RingBuffer<T>::RingBuffer(size_t size)  {
    _size = size;
    _array = new T[size];
    _front = 0;
    _back = 0;
    _remaining = 0;
}

/**
 * @brief Default destructor
 */
template <typename T>
RingBuffer<T>::~RingBuffer() {
    delete[] _array;
}

/**
 * @brief Gets the next element from the front of the buffer.
 *
 * @return the next element
 */
template <typename T>
T RingBuffer<T>::top() {
    std::unique_lock<std::recursive_mutex> lock(_mutex);
    if(!this->isEmpty()) {
        const T val = _array[_front];
        _front = (_front + 1) % _size;
        _remaining--;
        return val;
    } else {
        return NULL;
    }
}

/**
 * @brief Gets the next len elements from the front of the buffer.
 *
 * @param dest destination buffer to store the elements in
 * @param offset offset in dest buffer to start storing
 * @param len number of elements to get
 * @return number of elements retrieved
 */
template <typename T>
size_t RingBuffer<T>::top(T* dest, int offset, size_t len) {
    std::unique_lock<std::recursive_mutex> lock(_mutex);
    if(!this->isEmpty()) {
        const size_t ELEMENTS_TO_GET = std::min(getRemaining(), len);
        for(unsigned i = 0; i < ELEMENTS_TO_GET; i++) {
            dest[offset+i] = _array[_front];
            _front = (_front + 1) % _size;
            _remaining--;
        }
        return ELEMENTS_TO_GET;
    } else {
        return 0;
    }
}

/**
 * @brief Gets the remaining elements from the front of the buffer
 *
 * @param outbuf destination buffer to store elemetns in
 * @return number of elements retrieved
 */
template <typename T>
size_t RingBuffer<T>::topRemaining(T* outbuf) {
    std::unique_lock<std::recursive_mutex> lock(_mutex);
    const int remainingSaved = _remaining;
    for(unsigned i = 0; i < remainingSaved; i++) {
        outbuf[i] = _array[_front];
        _front = (_front + 1) % _size;
        _remaining--;
    }
    return remainingSaved;
}

/**
 * @brief Puts a new element at the end of the buffer, possibly overwriting the front
 * element if the buffer is full.
 *
 * @param val the element to put
 */
template <typename T>
void RingBuffer<T>::push(T val) {
    std::unique_lock<std::recursive_mutex> lock(_mutex);
    _array[_back] = val;
    _back = (_back + 1) % _size;   // increase index and loop buffer if needed
    _remaining = std::min(_size, ++_remaining);
}

/**
 * @brief Bulk puts new elements at the end of the buffer, possibly overwriting the front
 * elements if the buffer is full.
 *
 * @param src src buffer to push from
 * @param offset offset in src buffer to start copying from
 * @param len number of elements to push
 */
template <typename T>
void RingBuffer<T>::push(T* src, int offset, size_t len) {
    std::unique_lock<std::recursive_mutex> lock(_mutex);
    for(unsigned i = 0; i < len; i++) {
        _array[_back] = src[offset+i];
        if(_remaining == _size) {   // just overwritten, need move front
            _front = (_front + 1) % _size;
            // printf("moving _front to %d\n", _front);
        }
        // printf("put %d at idx %d\n", _array[_back], i);
        _back = (_back + 1) % _size;
        _remaining = std::min(_size, ++_remaining);

    }

    // printf("Current state:\n");
    // printf("Front: %d Back: %d\n", _front, _back);
    // for(unsigned i = 0; i < _size; i++) {
    //     printf("buf[%d] = %d\n", i, _array[i]);
    // }
}

template <class T>
bool RingBuffer<T>::isEmpty() {
    std::unique_lock<std::recursive_mutex> lock(_mutex);
    // return _front == _back;
    return getRemaining() == 0;
}

template <class T>
size_t RingBuffer<T>::getRemaining() const {
    return _remaining;
    // if(isEmpty()) {
    //     return 0;
    // } else if(_back > _front) {
    //     return _back - _front;
    // } else {
    //     return _size - _front + _back;
    // }
}

#endif /* RingBuffer_hpp */
