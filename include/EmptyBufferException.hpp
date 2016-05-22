#ifndef EmptyBuffer_hpp
#define EmptyBuffer_hpp

#include <stdexcept>
#include <cstdio>

/**
 * Exception to indicate that a buffer is empty.
 */
class EmptyBufferException : public std::exception
{
public:
    virtual const char* what() const throw() {
        return "Buffer is empty";
    }
};

#endif /* end of include guard: EmptyBuffer_hpp */
