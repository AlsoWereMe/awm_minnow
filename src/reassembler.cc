#include "reassembler.hh"

using namespace std;

void Reassembler::push_assembled_str( Writer& writer )
{
  auto it = unassembled_substrings_.begin();
  while ( it != unassembled_substrings_.end() ) {
    uint64_t index = it->first;
    std::string& data = it->second;
    // Map defaultly sort itself by key so just break the loop when the begin index is greater than next_byte_index.
    if ( index > next_byte_index_ ) {
      break;
    }
    uint64_t overlap = next_byte_index_ - index;
    if ( overlap < data.size() ) {
      writer.push( data.substr( overlap ) );
    }
    next_byte_index_ = writer.bytes_pushed();
    it = unassembled_substrings_.erase( it );
  }
  if ( writer.bytes_pushed() == eof_index_ ) {
    writer.close();
  }
}

void Reassembler::insert(uint64_t first_index, std::string data, bool is_last_substring) {
    Writer &writer = output_.writer();

    if (writer.is_closed()) {
        return;
    }

    uint64_t data_end_index = first_index + data.size();

    // **记录 EOF 信息**
    if (is_last_substring) {
        eof_index_ = data_end_index;
        get_eof_ = true;
    }

    // **处理空数据**
    if (data.empty()) {
        // 检查是否需要关闭输出流
        if (get_eof_ && writer.bytes_pushed() == eof_index_) {
            writer.close();
        }
        return;
    }

    // **处理已经写入的数据**
    if (data_end_index <= next_byte_index_) {
        // 检查是否需要关闭输出流
        if (get_eof_ && writer.bytes_pushed() == eof_index_) {
            writer.close();
        }
        return;
    }

    // **修剪已经写入的部分**
    if (first_index < next_byte_index_) {
        data = data.substr(next_byte_index_ - first_index);
        first_index = next_byte_index_;
    }

    // **计算可用容量**
    uint64_t available_capacity = writer.available_capacity();

    if (available_capacity == 0) {
        // 检查是否需要关闭输出流
        if (get_eof_ && writer.bytes_pushed() == eof_index_) {
            writer.close();
        }
        return;
    }

    // **计算可以插入的数据长度**
    uint64_t max_insert_size = available_capacity - (first_index - next_byte_index_);

    if (max_insert_size == 0) {
        // 检查是否需要关闭输出流
        if (get_eof_ && writer.bytes_pushed() == eof_index_) {
            writer.close();
        }
        return;
    }

    if (data.size() > max_insert_size) {
        data = data.substr(0, max_insert_size);
        data_end_index = first_index + data.size();
    }

    // **插入数据**
    if (first_index == next_byte_index_) {
        writer.push(data);
        next_byte_index_ = writer.bytes_pushed();
        push_assembled_str(writer);
    } else {
        // 保存未能立即写入的数据
        auto it = unassembled_substrings_.find(first_index);
        if (it == unassembled_substrings_.end() || it->second.size() < data.size()) {
            unassembled_substrings_[first_index] = std::move(data);
        }
    }

    // **检查是否需要关闭输出流**
    if (get_eof_ && writer.bytes_pushed() == eof_index_) {
        writer.close();
    }
}


uint64_t Reassembler::bytes_pending() const
{
  uint64_t cnt = 0;
  uint64_t next_expected = next_byte_index_;

  for ( const auto& [index, data] : unassembled_substrings_ ) {
    if ( index >= next_expected ) {
      // For the non overlapping string, add its total length.
      cnt += data.length();
    } else if ( index + data.length() > next_expected ) {
      // For the overlapping string, add its non overlapping length.
      cnt += data.length() - ( next_expected - index );
    }
    // Maintaining expected index.
    next_expected = std::max( next_expected, index + data.length() );
  }
  return cnt;
}
