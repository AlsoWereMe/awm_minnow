#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  (void)message;

  if (message.RST)
  {
    reassembler_.reader().set_error();
    return;
  }
  
  
  if (!message.SYN && !syn_received_) {
      return;
  }
  string data = message.payload;
  bool eof = false;

  // Firstly receive SYN
  if (message.SYN && !syn_received_) {
      syn_received_ = true;
      isn_ = message.seqno;
      // Consider this data is the whole data
      if (message.FIN) {
          fin_received_ = true;
          eof = true;
      }
      reassembler_.insert(0, data, eof);
      return;
  }

  // Receving FIN
  if (message.FIN) {
    fin_received_ = true;
    eof = true;
  }

  // convert the seqno into absolute seqno
  uint64_t abs_seqno = message.seqno.unwrap(isn_, reassembler_.ackno() + 1);
  uint64_t stream_idx = abs_seqno - 1;

  // Insert the data into stream reassembler
  reassembler_.insert(stream_idx, data, eof);
}

TCPReceiverMessage TCPReceiver::send() const
{
    uint16_t window_size = std::min(reassembler_.writer().available_capacity(), static_cast<size_t>(UINT16_MAX));
    bool RST = reassembler_.reader().has_error();

    if (!syn_received_) {
        return TCPReceiverMessage {nullopt, window_size, RST};
    }

    uint64_t ackno_num = reassembler_.ackno() + 1;
    if (reassembler_.writer().is_closed()) {
        ackno_num += 1;
    }

    Wrap32 ackno = Wrap32::wrap(ackno_num, isn_);

    return TCPReceiverMessage {ackno, window_size, RST};
}

