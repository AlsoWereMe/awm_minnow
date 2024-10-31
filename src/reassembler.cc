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

void Reassembler::insert( uint64_t first_index, std::string data, bool is_last_substring )
{
  Writer& writer = output_.writer();

  // Just return when writer is closed.
  if ( writer.is_closed() ) {
    return;
  }

  // Close the writer when the last substring is empty.
  if ( data.empty() && is_last_substring ) {
    writer.close();
    return;
  } else if ( data.empty() ) {
    return;
  }

  uint64_t data_end_index = first_index + data.size();
  uint64_t available_capacity = writer.available_capacity();

  if ( available_capacity == 0 ) {
    return;
  }

  // Drop the whole substring when it had written already.
  if ( data_end_index <= next_byte_index_ ) {
    return;
  }

  // Keep the non overlapping part of substring if there is overlap.
  if ( first_index < next_byte_index_ ) {
    // In this situation, first index will be replaced with the number of pushed bytes.
    // And data will be replaced with the non overlapping substring.
    data = data.substr( next_byte_index_ - first_index );
    first_index = next_byte_index_;
  }

  // The size from the first index to the max capacity is the really free capacity the current substring can use.
  uint64_t free_capacity = available_capacity - ( first_index - next_byte_index_ );
  // In same, the really written length "pushed_length" is the minimun value between the length of data and free
  // capacity.
  uint64_t pushed_length = std::min( data.size(), free_capacity );
  data = data.substr( 0, pushed_length );

  // Now there are two situation, equal or greater.
  if ( first_index == next_byte_index_ ) {
    writer.push( data );
    next_byte_index_ = writer.bytes_pushed();
    push_assembled_str( writer );
  } else {
    auto it = unassembled_substrings_.find( first_index );
    if ( it == unassembled_substrings_.end() || it->second.size() < data.size() ) {
      // Remerber use move so that can transmitt the value but not the pointer.
      unassembled_substrings_[first_index] = std::move( data );
    }
  }

  // Save the EOF index for close writer.
  if ( is_last_substring ) {
    eof_index_ = data_end_index;
    get_eof_ = true;
  }

  if ( get_eof_ && writer.bytes_pushed() == eof_index_ ) {
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
