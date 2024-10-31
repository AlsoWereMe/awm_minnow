#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  return close_;
}

void Writer::push( string data )
{
  (void)data;
  // Get the size we will write.
  uint64_t size = available_capacity() >= data.length() ? data.length() : available_capacity();
  // Get the real string we insert to.
  std::string str = data.substr( 0, size );

  // Insert str to the buffer.
  buffer_.insert( buffer_.end(), str.begin(), str.end() );
  // Record the bytes already pushed.
  pushed_bytes_ += size;
}

void Writer::close()
{
  close_ = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
  return pushed_bytes_;
}

bool Reader::is_finished() const
{
  return close_ && buffer_.empty();
}

uint64_t Reader::bytes_popped() const
{
  return poped_bytes_;
}

string_view Reader::peek() const
{
  return string_view( &buffer_[0], 1 );
}

void Reader::pop( uint64_t len )
{
  (void)len;
  // Get the size we will pop.
  uint64_t size = buffer_.size() >= len ? len : buffer_.size();
  // Pop elements.
  // Most of time spend on "read",
  // so there should use a structure which can give us O(1) time complexity to pop elements.
  for ( uint64_t i = 0; i < size; i++ ) {
    buffer_.pop_front();
    poped_bytes_ += 1;
  }
}

uint64_t Reader::bytes_buffered() const
{
  return buffer_.size();
}