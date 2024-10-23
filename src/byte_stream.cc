#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream(uint64_t capacity) : capacity_(capacity) {}

bool Writer::is_closed() const
{
    return writer().close_;
}

void Writer::push(string data)
{
    (void)data;
    Writer wt = writer();
    if ((!wt.is_closed()) && data.length() <= wt.available_capacity())
    {
        wt.buffer_.insert(wt.buffer_.end(), data.begin(), data.end());
        wt.pushed_bytes_ += data.length();
        return;
    }
    wt.set_error();
    return;
}

void Writer::close()
{
    writer().close_ = true;
}

uint64_t Writer::available_capacity() const
{
    return writer().capacity_ - writer().buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
    return pushed_bytes_;
}

bool Reader::is_finished() const
{
    return reader().close_ || reader().buffer_.empty();
}

uint64_t Reader::bytes_popped() const
{
    return reader().poped_bytes_;
}

string_view Reader::peek() const
{
    return {};
}

void Reader::pop(uint64_t len)
{
    (void)len;
}

uint64_t Reader::bytes_buffered() const
{
    return reader().buffer_.size();
}