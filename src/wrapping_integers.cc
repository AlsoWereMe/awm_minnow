#include "wrapping_integers.hh"

using namespace std;
Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  (void)n;
  (void)zero_point;
  return Wrap32 { zero_point.raw_value_ + (uint32_t)n};
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  (void)zero_point;
  (void)checkpoint;
  uint64_t tmp_abs_seqno = 0;
  if (zero_point.raw_value_ > raw_value_)
  {
    tmp_abs_seqno = (uint64_t)(raw_value_) - (uint64_t)zero_point.raw_value_ + (1ULL << 32);
  } else {
    tmp_abs_seqno = (uint64_t)(raw_value_) - (uint64_t)zero_point.raw_value_;
  }
  
  if (tmp_abs_seqno >= checkpoint)
  {
    return tmp_abs_seqno;
  }
  
  uint64_t right = tmp_abs_seqno | (checkpoint & 0xFFFFFFFF00000000);
  while (right <= checkpoint)
  {
    right += (1ULL << 32);
  }
  uint64_t left = right - (1ULL << 32);
  return (checkpoint - left < right - checkpoint) ? left : right;
}
