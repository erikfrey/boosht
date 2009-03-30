#ifndef __BOOSHT_CORO_STREAMBUF_HPP__
#define __BOOSHT_CORO_STREAMBUF_HPP__

#include <iostream>
#include <streambuf>

#include <boost/array.hpp>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio.hpp>

#include <boost/coroutine/future.hpp>
#include <boost/coroutine/shared_coroutine.hpp>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "moost/http/reply.hpp"
#include "moost/http/request.hpp"
#include "moost/http/request_handler_base.hpp"
#include "moost/http/request_parser.hpp"

namespace boosht {

template <typename charT, typename traits = std::char_traits<charT> >
class coro_streambuf : public std::basic_streambuf<charT, traits>
{
public:

  typedef traits traits_type;
  typedef typename traits_type::int_type int_type;
  typedef typename traits_type::pos_type pos_type;
  typedef typename traits_type::off_type off_type;

  typedef boost::coroutines::shared_coroutine<void(void)> thread_type;

private:

  /// buffer for incoming data.
  boost::array<charT, 8192> in_buffer_;

  /// buffer for outgoing data.
  boost::array<charT, 8192> out_buffer_;

  /// coroutine upon which to wait
  thread_type::self & coro_self_;

  /// result from waiting for asio callback
  boost::coroutines::future<boost::system::error_code, std::size_t> wait_result_;

  /// our asio socket
  boost::asio::ip::tcp::socket & socket_;

protected:

  // called when there are too many characters in the buffer (thus, a write needs to be performed).
  virtual int_type overflow(int_type c);

  // called when the buffer needs to be flushed.
  virtual int_type sync();

  // when there are no more characters left in the buffer (reads a character).
  virtual int_type underflow();

public:

  explicit streambuf(thread_type::self& self,
                     boost::asio::ip::tcp::socket & socket)
  : coro_self_(self),
    wait_result_(self),
    socket_(socket)
  {
    // initialize get pointer.  underflow is called upon first read.
    setg(0, 0, 0);

    // initialize the put pointer.  overflow won't get called until this buffer is filled up
    setp(out_buffer_.begin(), out_buffer_.end());
  }
};

template <typename charT, typename traits>
typename coro_streambuf<charT, traits>::int_type
coro_streambuf<charT, traits>::overflow(coro_streambuf<charT, traits>::int_type c)
{
  charT* ibegin = out_buffer_;

  charT* iend = pptr();

  boost::asio::async_write(*socket_,
    boost::asio::const_buffer(out_buffer_, pptr() - out_buffer_),
    boost::coroutines::make_callback(wait_result_));

  boost::coroutines::wait(wait_result_);

  if (wait_result_->get<0>())
    return traits_type::eof();

  // reset out buffer
  setp(out_buffer_.begin(), out_buffer_.end());

  /// return anything but eof
  return traits_type::not_eof(c);
}

// this is called to flush the buffer
template <typename charT, typename traits>
typename coro_streambuf<charT, traits>::int_type
coro_streambuf<charT, traits>::sync()
{
  return traits_type::eq_int_type(overflow(traits_type::eof()),
                                  traits_type::eof()) ? -1 : 0;
}

// fill the input buffer.  This reads from the streambuf and
// decrypts the contents by xoring it.
template <typename charT, typename traits>
typename coro_streambuf<charT, traits>::int_type
coro_streambuf<charT, traits>::underflow()
{
  socket_->async_read_some(
    boost::asio::buffer(in_buffer_, in_buffer_.size()),
    boost::coroutines::make_callback(wait_result_));

  boost::coroutines::wait(wait_result_);

  size_t len = read_result->get<1>();

  // intialize or reset the get pointer
  setg(in_buffer_, in_buffer_, in_buffer_ + len);

  // If nothing was read, then the end is here.
  if(len == 0)
    return traits_type::eof();

  // Return the first character.
  return traits_type::not_eof(in_buffer_[0]);
}

} // boosht

#endif // __BOOSHT_CORO_STREAMBUF_HPP__
