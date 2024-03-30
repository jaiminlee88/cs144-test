#include "stream_reassembler.hh"
#include <vector>
#include <iostream>
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
    _eof_index(0),
    _unassemble_cnt(0),
    _eof(false),
    _m() {
    }

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // EOF到了不意味着结束，知识告诉你结束的index是多少
    // DUMMY_CODE(data, index, eof);
    if (eof) {
        _eof = true;
        _eof_index = index;
    }

    // cout << "before============" << endl;
    //     for (auto& it : _m) {
    //         cout << it.first << " : " << it.second << " [" << it.second.size() << "]"<< endl;
    //     }
    size_t end_index = index + data.size();
    if (_next_index < index + data.size()) {
        merge_data(data, index, end_index);
    }
    // cout << "after merge============[" << index << "] " << data << " [" <<data.size()<< "]"<< endl;
    //     for (auto& it : _m) {
    //         cout << it.first << " : " << it.second << " [" << it.second.size() << "]"<< endl;
    //     }
    // if (_m.empty()) {
    //     cout << "++++ _m.empty()=true "; 
    // } else { cout << "++++ _m.empty()=false "; }
    //     cout << " _next_index: " << _next_index 
    //      << " _m.begin()->first: " << _m.begin()->first 
    //      << " buf.cap: " << _output.remaining_capacity() << endl;
    while (!_m.empty() && _next_index == _m.begin()->first && _output.remaining_capacity() > 0) {
        size_t size = min(_m.begin()->second.size(), _output.remaining_capacity());
        // cout << "----" << _m.begin()->second.substr(0, size) << " " << size << endl;
        _output.write(_m.begin()->second.substr(0, size));
        _next_index += size;
        _unassemble_cnt -= size;
        if (_next_index >= (_m.begin()->first + _m.begin()->second.size())) {
            _m.erase(_m.begin());
            continue;
        }
        string tmp = std::move(_m.begin()->second);
        tmp.erase(0, size);
        size_t new_index = _m.begin()->first + size;
        _m.erase(_m.begin());
        _m[new_index] = std::move(tmp);
    }

    // cout << "after buffer============ _next_index:" << _next_index << endl;
    //     for (auto& it : _m) {
    //         cout << it.first << " : " << it.second << " [" << it.second.size() << "]"<< endl;
    //     }
    if (_eof && _unassemble_cnt == 0) {
        _output.end_input();
    }
}

void StreamReassembler::merge_data(const string &data, size_t sindex, size_t eindex) {
    if (_eof_index > 0 && sindex > _eof_index) {
        return;
    }
    if (_output.buffer_size() + _unassemble_cnt >= _capacity) {
        return;
    }
    if (data.size() <= 0 || eindex <= _next_index) {
        return;
    }
    if (_m.empty()) { // append head
        if (_next_index < sindex) {
            _unassemble_cnt += data.size();
            _m[sindex] = std::move(data);
        } else {
            _unassemble_cnt += data.size() - (_next_index - sindex);
            _m[_next_index] = data.substr(_next_index - sindex, eindex - _next_index + 1);
        }
        return;
    }

    string tmp = std::move(data);
    if (sindex < _next_index) {
        tmp.erase(tmp.begin(), tmp.begin() + (_next_index - sindex));
        sindex = _next_index;
        if (tmp.size() == 0) {
            return;
        }
    }
    
    if (eindex <= _m.begin()->first) {
        _unassemble_cnt += tmp.size();
        _m[sindex] = std::move(tmp);
        return;
    }
    auto end_iter = _m.end();
    --end_iter;
    if (sindex >= (end_iter->first + end_iter->second.size())) { // append tail
        _unassemble_cnt += tmp.size();
        _m[sindex] = std::move(tmp);
        return;
    }

    // find hold and fill it,
    vector<size_t> to_del_key;
    auto l_iter = _m.lower_bound(sindex);
    if (l_iter != _m.begin()) {
        --l_iter;
    }
    // cout << "----" << l_iter->first << " : " << l_iter->second << endl;
    size_t new_sindex = sindex;
    while (true) {
        if (l_iter == _m.end()) {
            break;
        }
        // 只要沾边，都并过来
        size_t l_start = l_iter->first;
        size_t l_end = l_iter->first + l_iter->second.size();
        if (l_start >= eindex) { // 并无可并
            break;
        }
        if (l_end <= sindex) { // 前一个位置与当前data无关
            ++l_iter;
            continue;
        }
        if (l_end <= eindex) { // tmp压住后半部分
            if (sindex > l_start) {
                // cout << "///////// " << sindex - l_start << " " << l_start << endl;
                new_sindex = l_start;
                tmp.insert(0, l_iter->second, 0, sindex - l_start); // 在A的开头插入B的前三个字符
            }
            to_del_key.push_back(l_start);
            ++l_iter;
            continue;
        }
        if (l_start <= sindex) { // 该段已经出现过，不用加
            tmp = "";
            break;
        } else {
            // cout << "!!!!!!!!!!1 " << tmp << endl;
            // cout << eindex - l_start << endl;
            // cout << l_end - eindex << endl;
            tmp.insert(tmp.size(), l_iter->second, eindex - l_start, l_end - eindex);
            // cout << "!!!!!!!tmp: " << tmp << endl;
            to_del_key.push_back(l_start);
            ++l_iter;
            continue;
        }
    }
    for (auto& it : to_del_key) {
        _unassemble_cnt -= _m[it].size();
        _m.erase(it);
    }
    if (tmp.size() > 0) {
        _unassemble_cnt += tmp.size();
        _m[new_sindex] = std::move(tmp);
    }
}
size_t StreamReassembler::unassembled_bytes() const { return _unassemble_cnt; }

bool StreamReassembler::empty() const { return _unassemble_cnt == 0; }
