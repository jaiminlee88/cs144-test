#include "byte_stream.hh"

#include <algorithm>
#include <iterator>
#include <stdexcept>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : 
    _error(false),  //!< Flag indicating that the stream suffered an error.
    _capacity(capacity),
    _read_bytes(0),
    _write_bytes(0),
    _input_ended(false),
    _q{}
    { 
    // DUMMY_CODE(capacity);
}

size_t ByteStream::write(const string &data) {
    // DUMMY_CODE(data);
    if (input_ended()) {
        return 0;
    }
    size_t size = min(remaining_capacity(), data.size());
    if (size == 0) {
        return 0;
    }
    for (size_t i = 0; i < size; ++i) {
        _q.push_back(data[i]);
        ++_write_bytes;
    }
    return size;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    // DUMMY_CODE(len);
    size_t size = min(buffer_size(), len);
    if (size <= 0) {
        return {};
    }
    string s;
    for (size_t i = 0; i < size; ++i) {
        s.append(1, _q[i]);
    }
    return s;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    // DUMMY_CODE(len);
    size_t size = min(buffer_size(), len);
    if (size <= 0) {
        return;
    }
    for (size_t i = 0; i < size; ++i) {
        _q.pop_front();
        ++_read_bytes;
    }
}

void ByteStream::end_input() { _input_ended = true; }

bool ByteStream::input_ended() const { return _input_ended; }

size_t ByteStream::buffer_size() const { return _q.size(); }

bool ByteStream::buffer_empty() const { 
    return _q.empty(); }

bool ByteStream::eof() const { return _input_ended && buffer_size() == 0; }

size_t ByteStream::bytes_written() const { return _write_bytes; }

size_t ByteStream::bytes_read() const { return _read_bytes; }

size_t ByteStream::remaining_capacity() const { return _capacity - _q.size(); }
