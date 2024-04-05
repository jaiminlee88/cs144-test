#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>
#include <iostream>
// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _retransmission_timeout(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window(bool send_syn) { // send all data in window
    if (!_syn_flag) {
        if (send_syn) {
            TCPSegment seg;
            seg.header().syn = true;
            send_segment(seg);
            _syn_flag = true;
        }
        return;
    }

    size_t win = _window_size > 0 ? _window_size : 1;
    size_t win_remained;
    while (true) {
        win_remained = win - (_next_seqno - _recv_ackno);
        if (win_remained == 0 || _fin_flag) {
            break;
        }
        size_t size = min(TCPConfig::MAX_PAYLOAD_SIZE, win_remained); // seg size control
        TCPSegment seg;
        string str  = _stream.read(size); // the socket write in
        seg.payload() = Buffer(std::move(str));
        if (seg.length_in_sequence_space() < win && _stream.eof()) {
            seg.header().fin = true;
            _fin_flag = true;
        }
        if (seg.length_in_sequence_space() == 0) {
            return;
        }
        send_segment(seg);
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
//! \returns `false` if the ackno appears invalid (acknowledges something the TCPSender hasn't sent yet)
bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    // DUMMY_CODE(ackno, window_size);
    size_t abs_ackno = unwrap(ackno, _isn, _recv_ackno);
    if (abs_ackno > _next_seqno) {
        // segs not sent yet
        return false;
    }

    _window_size = window_size; // notified by reveiver
    if (abs_ackno <= _recv_ackno) {
        return true; // received in advance or already received
    }
    _recv_ackno = abs_ackno;

    //pop pending segments
    while (!_segments_outstanding.empty()) {
        TCPSegment seg = _segments_outstanding.front();
        if (unwrap(seg.header().seqno, _isn, _next_seqno)
            + seg.length_in_sequence_space() <= abs_ackno) {
            _bytes_in_flight -= seg.length_in_sequence_space();
            _segments_outstanding.pop();
        } else {
            break;
        }
    }

    fill_window();
    _retransmission_timeout = _initial_retransmission_timeout;
    _consecutive_retransmission = 0;

    if (!_segments_outstanding.empty()) {
        _timer_running = true;
        _timer = 0;
    }
    return true;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
    // DUMMY_CODE(ms_since_last_tick);
    _timer += ms_since_last_tick;
    
    if (_timer >= _retransmission_timeout && !_segments_outstanding.empty()) {
        // cout << "-------" << _segments_outstanding.size() << endl;
        _segments_out.push(_segments_outstanding.front());
        _consecutive_retransmission++;
        _retransmission_timeout *= 2;
        _timer_running = true;
        _timer = 0;
    }
    if (_segments_outstanding.empty()) {
        _timer_running = false;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { 
    return _consecutive_retransmission;
}

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
}

void TCPSender::send_empty_segment(WrappingInt32 seqno) {
    TCPSegment seg;
    seg.header().seqno = seqno;
    _segments_out.push(seg);
}

void TCPSender::send_segment(TCPSegment &seg) {
    seg.header().seqno = wrap(_next_seqno, _isn);
    _next_seqno += seg.length_in_sequence_space();
    _bytes_in_flight += seg.length_in_sequence_space();
    _segments_outstanding.push(seg);
    _segments_out.push(seg);
    if (!_timer_running) {
        _timer_running = true;
        _timer = 0;
    }
}