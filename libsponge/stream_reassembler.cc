#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) :
    _output(capacity),
    _capacity(capacity),
    _next_index(0),
    _eof_index(-1),
    _unassemble_cnt(0),
    _m() {
    }

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // EOF到了不意味着结束，知识告诉你结束的index是多少
    // DUMMY_CODE(data, index, eof);
    if (eof) {
        _eof_index = index;
    }

    size_t end_index = index + data.size();
    if (_next_index < index + data.size()) {
        merge_data(data, index, end_index);
    }
    
    if (!_m.empty() && _next_index == _m.begin()->first) {

    }
}

void StreamReassembler::merge_data(const string &data, size_t sindex, size_t eindex) {
    if (data.size() <= 0 || eindex <= _next_index) {
        return;
    }
    if (_m.empty() || eindex <= _m.begin()->first) { // append head
        _unassemble_cnt += data.size();
        _m[sindex] = std::move(data);
        return;
    }
    auto end_iter = _m.end();
    --end_iter;
    if (sindex >= (end_iter->first + end_iter->second.size())) { // append tail
        _unassemble_cnt += data.size();
        _m[sindex] = std::move(data);
        return;
    }

    // find hold and fill it
    size_t pstart = SIZE_MIN;
    size_t pend = 0;
    auto l_iter = _m.lower_bound({sindex, ""});
    --l_iter;
    if (l_iter < _m.begin()) {
        pstart = sindex;
        pend = _m.begin()->first;
        _unassemble_cnt += (pend - pstart);
        _m[sindex](data.begin(), pend - pstart);
    }
    
    while (true) {
        auto l_iter = _m.upper_bound({sindex, ""});
        --l_iter;
        if (l_iter < _m.begin()) {
            l_iter = _m.begin();
        }
    }
    
   
    // while (true) {
    //     size_t l_end = down_iter->first + down_iter->data.size();
    //     auto r_iter = _m.upper_bound({l_end, ""});
    // }
}
size_t StreamReassembler::unassembled_bytes() const { return {}; }

bool StreamReassembler::empty() const { return {}; }
