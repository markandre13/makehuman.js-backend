# makehuman.js backend

A daemon for [makehuman.js](https://github.com/markandre13/makehuman.js) providing access to [mediapipe_cpp_lib](https://github.com/markandre13/mediapipe_cpp_lib) and [Chordata Motion](https://chordata.cc).

### Build

The software is work in progress

#### macOS

* Xcode 16.2 (bazel 6.5.0 fails to build zlib with Xcode 16.3)
  [Apple Downloads](https://developer.apple.com/download/all/?q=xcode)
* brew
* brew install llvm makedepend opencv libev nettle wslay python@3.12 numpy
* bazel 6.5.0
* bun

### Technical

* mediapipe will open a window, which on macOS means that the mediapipe code
  needs to run in the main thread
* frontend and backend use websocket to communicate with the intention to
  move to webtransport later. the protocol on top of that is corba.

### Errors

#### client lost connection

2025-04-08T13:05:08Z ERROR  WsConnection::flushSendBuffer(): could not queue message. is the connection still open?

* since queue msg makes a copy, do we have to buffer all messages on our own?
  sometimes: in case the connection is not established yet.
* reason seems to be that the client lost the connection. shouldn't we detect
  this and then close the connection on the server side and reconnect on the
  client side? (once of the reasons i choose corba: network transparency)

#### client misses GIOP and start of message

* there is a 16 bit word instead indicating the size
* skipping those 16 bit's didn't work
* try to locate the issue on the sender side

debug wslay?

#### SEGV in wslay_event_send()

`
==11539==Hint: address points to the zero page.
    #0 0x000103a4b62c in wslay_event_send+0xd8 (libwslay.0.dylib:arm64+0x362c)
    #1 0x0001033a5348 in CORBA::detail::WsConnection::flushSendBuffer() connection.cc:293
`

#### SEGV

`
==19622==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000004 (pc 0x00010122762c bp 0x000170acd510 sp 0x000170acd4b0 T35)
==19622==The signal is caused by a READ memory access.
==19622==Hint: address points to the zero page.
    #0 0x00010122762c in wslay_event_send+0xd8 (libwslay.0.dylib:arm64+0x362c)
    #1 0x000100b812dc in CORBA::detail::WsConnection::flushSendBuffer() connection.cc:301
    #2 0x000100b7f858 in CORBA::detail::WsConnection::send(std::__1::unique_ptr<std::__1::vector<char, std::__1::allocator<char>>, std::__1::default_delete<std::__1::vector<char, std::__1::allocator<char>>>>&&) connection.cc:66
    
    THIS IS TO SEND THE REPLY: connection->send(move(encoder->buffer._data));
    
    #3 0x000100acbf48 in CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0::operator()() const orb.cc:410

    #4 0x000100acbaf8 in decltype(std::declval<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&>()()) std::__1::__invoke[abi:ne200100]<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&>(CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&) invoke.h:179
    #5 0x000100acbad4 in void std::__1::__invoke_void_return_wrapper<void, true>::__call[abi:ne200100]<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&>(CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&) invoke.h:251
    #6 0x000100acbab0 in void std::__1::__invoke_r[abi:ne200100]<void, CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&>(CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&) invoke.h:273
    #7 0x000100acba8c in std::__1::__function::__alloc_func<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0, std::__1::allocator<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0>, void ()>::operator()[abi:ne200100]() function.h:167
    #8 0x000100acad00 in std::__1::__function::__func<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0, std::__1::allocator<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0>, void ()>::operator()() function.h:319
    #9 0x0001007c8cbc in std::__1::__function::__value_func<void ()>::operator()[abi:ne200100]() const function.h:436
    #10 0x0001007c8844 in std::__1::function<void ()>::operator()() const function.h:995
    #11 0x0001007c868c in cppasync::detail::async_promise<void>::~async_promise() async.hh:222
    #12 0x0001007b93f4 in cppasync::detail::async_promise<void>::~async_promise() async.hh:215

    #13 0x0001009a1204 in Recorder_skel::_call(std::__1::basic_string_view<char, std::__1::char_traits<char>> const&, CORBA::GIOPDecoder&, CORBA::GIOPEncoder&) (.destroy) makehuman.cc:327
    #14 0x0001007c81b0 in std::__1::coroutine_handle<cppasync::detail::async_promise<void>>::destroy[abi:ne200100]() const coroutine_handle.h:149
    #15 0x0001007b9230 in std::__1::coroutine_handle<void> cppasync::detail::async_promise_base::final_awaitable::await_suspend<cppasync::detail::async_promise<void>>(std::__1::coroutine_handle<cppasync::detail::async_promise<void>>) async.hh:86
    #16 0x0001008ec738  (backend:arm64+0x100184738)
    #17 0x0001009a018c in Recorder_skel::_call(std::__1::basic_string_view<char, std::__1::char_traits<char>> const&, CORBA::GIOPDecoder&, CORBA::GIOPEncoder&) (.resume) makehuman.cc:327
    #18 0x000100832b40 in std::__1::coroutine_handle<void>::resume[abi:ne200100]() const coroutine_handle.h:69
  
  AT THIS POINT WE CONTINUE IN THE LIBEV THREAD
  
    #19 0x000100830270 in ThreadSync::resume() loop.hh:42
    #20 0x000100830000 in OpenCVLoop::libev_async_cb(ev_loop*, ev_async*, int) loop.cc:34
    #21 0x0001011c7340 in ev_invoke_pending+0x58 (libev.4.dylib:arm64+0x3340)
    #22 0x0001011c7758 in ev_run+0x3e8 (libev.4.dylib:arm64+0x3758)
    #23 0x0001007b5d58 in decltype(std::declval<int (*)(ev_loop*, int)>()(std::declval<ev_loop*>(), std::declval<int>())) std::__1::__invoke[abi:ne200100]<int (*)(ev_loop*, int), ev_loop*, int>(int (*&&)(ev_loop*, int), ev_loop*&&, int&&) invoke.h:179
    #24 0x0001007b5c14 in void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, int (*)(ev_loop*, int), ev_loop*, int, 2ul, 3ul>(std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, int (*)(ev_loop*, int), ev_loop*, int>&, std::__1::__tuple_indices<2ul, 3ul>) thread.h:199
    #25 0x0001007b5380 in void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, int (*)(ev_loop*, int), ev_loop*, int>>(void*) thread.h:208
    #26 0x0001039741e4 in asan_thread_start(void*)+0x48 (libclang_rt.asan_osx_dynamic.dylib:arm64+0x501e4)
    #27 0x0001932d9c08 in _pthread_start+0x84 (libsystem_pthread.dylib:arm64+0x6c08)
    #28 0x0001932d4b7c in thread_start+0x4 (libsystem_pthread.dylib:arm64+0x1b7c)

==19622==Register values:
 x[0] = 0x00000001464d80d0   x[1] = 0x0000000000000000   x[2] = 0x00000001039d4180   x[3] = 0x0000000000000024  
 x[4] = 0x0000000063000000   x[5] = 0x0000000000000000   x[6] = 0x0000000170a4c000   x[7] = 0x0000000000000001  
 x[8] = 0x0000000000000000   x[9] = 0x00000001464e5c40  x[10] = 0x00000001464e1338  x[11] = 0x000000015b468000  
x[12] = 0x00000000000047f0  x[13] = 0x0000000000000000  x[14] = 0x00007fffffffffff  x[15] = 0x000010700001ffff  
x[16] = 0x0000000000000049  x[17] = 0x00000001931757a0  x[18] = 0x0000000000000000  x[19] = 0x0000621000282900  
x[20] = 0x00006210002829a0  x[21] = 0x0000000000000001  x[22] = 0x00000001011cbcff  x[23] = 0x0000000000000000  
x[24] = 0x0000000000000000  x[25] = 0x0000000000000001  x[26] = 0x0000000000000080  x[27] = 0x0000000000000001  
x[28] = 0x0000000000000001     fp = 0x0000000170acd510     lr = 0x0000000101227610     sp = 0x0000000170acd4b0  
AddressSanitizer can not provide additional info.
SUMMARY: AddressSanitizer: SEGV (libwslay.0.dylib:arm64+0x362c) in wslay_event_send+0xd8
Thread T35 created by T0 here:
    #0 0x00010396ef54 in pthread_create+0x58 (libclang_rt.asan_osx_dynamic.dylib:arm64+0x4af54)
    #1 0x0001007b5220 in std::__1::__libcpp_thread_create[abi:ne200100](_opaque_pthread_t**, void* (*)(void*), void*) pthread.h:182
    #2 0x0001007b4f40 in std::__1::thread::thread<int (&)(ev_loop*, int), ev_loop*&, int, 0>(int (&)(ev_loop*, int), ev_loop*&, int&&) thread.h:218
    #3 0x00010076dbe8 in std::__1::thread::thread<int (&)(ev_loop*, int), ev_loop*&, int, 0>(int (&)(ev_loop*, int), ev_loop*&, int&&) thread.h:213
    #4 0x00010076d084 in main main.cc:101
    #5 0x000192f3ab48  (<unknown module>)
`

#### SEGV

`
=================================================================
==22257==ERROR: AddressSanitizer: heap-use-after-free on address 0x60300028d5f0 at pc 0x000107f38dc8 bp 0x00016c3739e0 sp 0x00016c373190
READ of size 24 at 0x60300028d5f0 thread T35
    #0 0x000107f38dc4 in sendto+0x278 (libclang_rt.asan_osx_dynamic.dylib:arm64+0x40dc4)
    #1 0x000107f388e4 in send+0x40 (libclang_rt.asan_osx_dynamic.dylib:arm64+0x408e4)
    #2 0x0001052dc4bc in CORBA::detail::wslay_send_callback(wslay_event_context*, unsigned char const*, unsigned long, int, void*) connection.cc:557
    #3 0x00010597e400 in wslay_frame_send+0x2d0 (libwslay.0.dylib:arm64+0x2400)
    #4 0x00010597f798 in wslay_event_send+0x244 (libwslay.0.dylib:arm64+0x3798)
    #5 0x0001052d929c in CORBA::detail::WsConnection::flushSendBuffer() connection.cc:303
    #6 0x0001052d7818 in CORBA::detail::WsConnection::send(std::__1::unique_ptr<std::__1::vector<char, std::__1::allocator<char>>, std::__1::default_delete<std::__1::vector<char, std::__1::allocator<char>>>>&&) connection.cc:68
    #7 0x000105223ee0 in CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0::operator()() const orb.cc:410
    #8 0x000105223a90 in decltype(std::declval<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&>()()) std::__1::__invoke[abi:ne200100]<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&>(CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&) invoke.h:179
    #9 0x000105223a6c in void std::__1::__invoke_void_return_wrapper<void, true>::__call[abi:ne200100]<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&>(CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&) invoke.h:251
    #10 0x000105223a48 in void std::__1::__invoke_r[abi:ne200100]<void, CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&>(CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&) invoke.h:273
    #11 0x000105223a24 in std::__1::__function::__alloc_func<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0, std::__1::allocator<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0>, void ()>::operator()[abi:ne200100]() function.h:167
    #12 0x000105222c98 in std::__1::__function::__func<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0, std::__1::allocator<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0>, void ()>::operator()() function.h:319
    #13 0x000104f20c54 in std::__1::__function::__value_func<void ()>::operator()[abi:ne200100]() const function.h:436
    #14 0x000104f207dc in std::__1::function<void ()>::operator()() const function.h:995
    #15 0x000104f20624 in cppasync::detail::async_promise<void>::~async_promise() async.hh:222
    #16 0x000104f1138c in cppasync::detail::async_promise<void>::~async_promise() async.hh:215
    #17 0x0001050f919c in Recorder_skel::_call(std::__1::basic_string_view<char, std::__1::char_traits<char>> const&, CORBA::GIOPDecoder&, CORBA::GIOPEncoder&) (.destroy) makehuman.cc:327
    #18 0x000104f20148 in std::__1::coroutine_handle<cppasync::detail::async_promise<void>>::destroy[abi:ne200100]() const coroutine_handle.h:149
    #19 0x000104f111c8 in std::__1::coroutine_handle<void> cppasync::detail::async_promise_base::final_awaitable::await_suspend<cppasync::detail::async_promise<void>>(std::__1::coroutine_handle<cppasync::detail::async_promise<void>>) async.hh:86
    #20 0x0001050446d0  (backend:arm64+0x1001846d0)
    #21 0x0001050f8124 in Recorder_skel::_call(std::__1::basic_string_view<char, std::__1::char_traits<char>> const&, CORBA::GIOPDecoder&, CORBA::GIOPEncoder&) (.resume) makehuman.cc:327
    #22 0x000104f8aad8 in std::__1::coroutine_handle<void>::resume[abi:ne200100]() const coroutine_handle.h:69
    #23 0x000104f88208 in ThreadSync::resume() loop.hh:42
    #24 0x000104f87f98 in OpenCVLoop::libev_async_cb(ev_loop*, ev_async*, int) loop.cc:34
    #25 0x00010591f340 in ev_invoke_pending+0x58 (libev.4.dylib:arm64+0x3340)
    #26 0x00010591f758 in ev_run+0x3e8 (libev.4.dylib:arm64+0x3758)
    #27 0x000104f0dcf0 in decltype(std::declval<int (*)(ev_loop*, int)>()(std::declval<ev_loop*>(), std::declval<int>())) std::__1::__invoke[abi:ne200100]<int (*)(ev_loop*, int), ev_loop*, int>(int (*&&)(ev_loop*, int), ev_loop*&&, int&&) invoke.h:179
    #28 0x000104f0dbac in void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, int (*)(ev_loop*, int), ev_loop*, int, 2ul, 3ul>(std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, int (*)(ev_loop*, int), ev_loop*, int>&, std::__1::__tuple_indices<2ul, 3ul>) thread.h:199
    #29 0x000104f0d318 in void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, int (*)(ev_loop*, int), ev_loop*, int>>(void*) thread.h:208
    #30 0x000107f481e4 in asan_thread_start(void*)+0x48 (libclang_rt.asan_osx_dynamic.dylib:arm64+0x501e4)
    #31 0x0001932d9c08 in _pthread_start+0x84 (libsystem_pthread.dylib:arm64+0x6c08)
    #32 0x0001932d4b7c in thread_start+0x4 (libsystem_pthread.dylib:arm64+0x1b7c)

0x60300028d5f0 is located 0 bytes inside of 24-byte region [0x60300028d5f0,0x60300028d608)
freed by thread T0 here:
    #0 0x000107f4b5dc in free+0x74 (libclang_rt.asan_osx_dynamic.dylib:arm64+0x535dc)
    #1 0x00010597ee28 in wslay_event_omsg_free+0x18 (libwslay.0.dylib:arm64+0x2e28)
    #2 0x00010597f830 in wslay_event_send+0x2dc (libwslay.0.dylib:arm64+0x3830)
    #3 0x0001052d929c in CORBA::detail::WsConnection::flushSendBuffer() connection.cc:303
    #4 0x0001052d7818 in CORBA::detail::WsConnection::send(std::__1::unique_ptr<std::__1::vector<char, std::__1::allocator<char>>, std::__1::default_delete<std::__1::vector<char, std::__1::allocator<char>>>>&&) connection.cc:68
    #5 0x0001051e299c in CORBA::ORB::onewayCall(CORBA::Stub*, char const*, std::__1::function<void (CORBA::GIOPEncoder&)>) orb.cc:318
    #6 0x0001050291cc in Frontend_stub::frame(unsigned int) makehuman.cc:59
    #7 0x000104f27e08 in Backend_impl::Backend_impl(std::__1::shared_ptr<CORBA::ORB>, ev_loop*, OpenCVLoop*)::$_0::operator()(cv::Mat const&, int) const backend_impl.cc:30
    #8 0x000104f27d20 in decltype(std::declval<Backend_impl::Backend_impl(std::__1::shared_ptr<CORBA::ORB>, ev_loop*, OpenCVLoop*)::$_0&>()(std::declval<cv::Mat const&>(), std::declval<int>())) std::__1::__invoke[abi:ne200100]<Backend_impl::Backend_impl(std::__1::shared_ptr<CORBA::ORB>, ev_loop*, OpenCVLoop*)::$_0&, cv::Mat const&, int>(Backend_impl::Backend_impl(std::__1::shared_ptr<CORBA::ORB>, ev_loop*, OpenCVLoop*)::$_0&, cv::Mat const&, int&&) invoke.h:179
    #9 0x000104f27c80 in void std::__1::__invoke_void_return_wrapper<void, true>::__call[abi:ne200100]<Backend_impl::Backend_impl(std::__1::shared_ptr<CORBA::ORB>, ev_loop*, OpenCVLoop*)::$_0&, cv::Mat const&, int>(Backend_impl::Backend_impl(std::__1::shared_ptr<CORBA::ORB>, ev_loop*, OpenCVLoop*)::$_0&, cv::Mat const&, int&&) invoke.h:251
    #10 0x000104f27c4c in void std::__1::__invoke_r[abi:ne200100]<void, Backend_impl::Backend_impl(std::__1::shared_ptr<CORBA::ORB>, ev_loop*, OpenCVLoop*)::$_0&, cv::Mat const&, int>(Backend_impl::Backend_impl(std::__1::shared_ptr<CORBA::ORB>, ev_loop*, OpenCVLoop*)::$_0&, cv::Mat const&, int&&) invoke.h:273
    #11 0x000104f27c18 in std::__1::__function::__alloc_func<Backend_impl::Backend_impl(std::__1::shared_ptr<CORBA::ORB>, ev_loop*, OpenCVLoop*)::$_0, std::__1::allocator<Backend_impl::Backend_impl(std::__1::shared_ptr<CORBA::ORB>, ev_loop*, OpenCVLoop*)::$_0>, void (cv::Mat const&, int)>::operator()[abi:ne200100](cv::Mat const&, int&&) function.h:167
    #12 0x000104f27134 in std::__1::__function::__func<Backend_impl::Backend_impl(std::__1::shared_ptr<CORBA::ORB>, ev_loop*, OpenCVLoop*)::$_0, std::__1::allocator<Backend_impl::Backend_impl(std::__1::shared_ptr<CORBA::ORB>, ev_loop*, OpenCVLoop*)::$_0>, void (cv::Mat const&, int)>::operator()(cv::Mat const&, int&&) function.h:319
    #13 0x000104f8e82c in std::__1::__function::__value_func<void (cv::Mat const&, int)>::operator()[abi:ne200100](cv::Mat const&, int&&) const function.h:436
    #14 0x000104f89bfc in std::__1::function<void (cv::Mat const&, int)>::operator()(cv::Mat const&, int) const function.h:995
    #15 0x000104f88d7c in OpenCVLoop::run() loop.cc:81
    #16 0x000104ec5038 in main main.cc:102
    #17 0x000192f3ab48  (<unknown module>)

previously allocated by thread T35 here:
    #0 0x000107f4b4f0 in malloc+0x70 (libclang_rt.asan_osx_dynamic.dylib:arm64+0x534f0)
    #1 0x00010597e9d8 in wslay_event_queue_msg_ex+0xb4 (libwslay.0.dylib:arm64+0x29d8)
    #2 0x0001052d90d4 in CORBA::detail::WsConnection::flushSendBuffer() connection.cc:290
    #3 0x0001052d7818 in CORBA::detail::WsConnection::send(std::__1::unique_ptr<std::__1::vector<char, std::__1::allocator<char>>, std::__1::default_delete<std::__1::vector<char, std::__1::allocator<char>>>>&&) connection.cc:68
    #4 0x000105223ee0 in CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0::operator()() const orb.cc:410
    #5 0x000105223a90 in decltype(std::declval<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&>()()) std::__1::__invoke[abi:ne200100]<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&>(CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&) invoke.h:179
    #6 0x000105223a6c in void std::__1::__invoke_void_return_wrapper<void, true>::__call[abi:ne200100]<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&>(CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&) invoke.h:251
    #7 0x000105223a48 in void std::__1::__invoke_r[abi:ne200100]<void, CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&>(CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0&) invoke.h:273
    #8 0x000105223a24 in std::__1::__function::__alloc_func<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0, std::__1::allocator<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0>, void ()>::operator()[abi:ne200100]() function.h:167
    #9 0x000105222c98 in std::__1::__function::__func<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0, std::__1::allocator<CORBA::ORB::socketRcvd(CORBA::detail::Connection*, void const*, unsigned long)::$_0>, void ()>::operator()() function.h:319
    #10 0x000104f20c54 in std::__1::__function::__value_func<void ()>::operator()[abi:ne200100]() const function.h:436
    #11 0x000104f207dc in std::__1::function<void ()>::operator()() const function.h:995
    #12 0x000104f20624 in cppasync::detail::async_promise<void>::~async_promise() async.hh:222
    #13 0x000104f1138c in cppasync::detail::async_promise<void>::~async_promise() async.hh:215
    #14 0x0001050f919c in Recorder_skel::_call(std::__1::basic_string_view<char, std::__1::char_traits<char>> const&, CORBA::GIOPDecoder&, CORBA::GIOPEncoder&) (.destroy) makehuman.cc:327
    #15 0x000104f20148 in std::__1::coroutine_handle<cppasync::detail::async_promise<void>>::destroy[abi:ne200100]() const coroutine_handle.h:149
    #16 0x000104f111c8 in std::__1::coroutine_handle<void> cppasync::detail::async_promise_base::final_awaitable::await_suspend<cppasync::detail::async_promise<void>>(std::__1::coroutine_handle<cppasync::detail::async_promise<void>>) async.hh:86
    #17 0x0001050446d0  (backend:arm64+0x1001846d0)
    #18 0x0001050f8124 in Recorder_skel::_call(std::__1::basic_string_view<char, std::__1::char_traits<char>> const&, CORBA::GIOPDecoder&, CORBA::GIOPEncoder&) (.resume) makehuman.cc:327
    #19 0x000104f8aad8 in std::__1::coroutine_handle<void>::resume[abi:ne200100]() const coroutine_handle.h:69
    #20 0x000104f88208 in ThreadSync::resume() loop.hh:42
    #21 0x000104f87f98 in OpenCVLoop::libev_async_cb(ev_loop*, ev_async*, int) loop.cc:34
    #22 0x00010591f340 in ev_invoke_pending+0x58 (libev.4.dylib:arm64+0x3340)
    #23 0x00010591f758 in ev_run+0x3e8 (libev.4.dylib:arm64+0x3758)
    #24 0x000104f0dcf0 in decltype(std::declval<int (*)(ev_loop*, int)>()(std::declval<ev_loop*>(), std::declval<int>())) std::__1::__invoke[abi:ne200100]<int (*)(ev_loop*, int), ev_loop*, int>(int (*&&)(ev_loop*, int), ev_loop*&&, int&&) invoke.h:179
    #25 0x000104f0dbac in void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, int (*)(ev_loop*, int), ev_loop*, int, 2ul, 3ul>(std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, int (*)(ev_loop*, int), ev_loop*, int>&, std::__1::__tuple_indices<2ul, 3ul>) thread.h:199
    #26 0x000104f0d318 in void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, int (*)(ev_loop*, int), ev_loop*, int>>(void*) thread.h:208
    #27 0x000107f481e4 in asan_thread_start(void*)+0x48 (libclang_rt.asan_osx_dynamic.dylib:arm64+0x501e4)
    #28 0x0001932d9c08 in _pthread_start+0x84 (libsystem_pthread.dylib:arm64+0x6c08)
    #29 0x0001932d4b7c in thread_start+0x4 (libsystem_pthread.dylib:arm64+0x1b7c)

Thread T35 created by T0 here:
    #0 0x000107f42f54 in pthread_create+0x58 (libclang_rt.asan_osx_dynamic.dylib:arm64+0x4af54)
    #1 0x000104f0d1b8 in std::__1::__libcpp_thread_create[abi:ne200100](_opaque_pthread_t**, void* (*)(void*), void*) pthread.h:182
    #2 0x000104f0ced8 in std::__1::thread::thread<int (&)(ev_loop*, int), ev_loop*&, int, 0>(int (&)(ev_loop*, int), ev_loop*&, int&&) thread.h:218
    #3 0x000104ec5b80 in std::__1::thread::thread<int (&)(ev_loop*, int), ev_loop*&, int, 0>(int (&)(ev_loop*, int), ev_loop*&, int&&) thread.h:213
    #4 0x000104ec501c in main main.cc:101
    #5 0x000192f3ab48  (<unknown module>)

SUMMARY: AddressSanitizer: heap-use-after-free connection.cc:557 in CORBA::detail::wslay_send_callback(wslay_event_context*, unsigned char const*, unsigned long, int, void*)
Shadow bytes around the buggy address:
  0x60300028d300: 00 00 fa fa fd fd fd fa fa fa fd fd fd fa fa fa
  0x60300028d380: fd fd fd fa fa fa fd fd fd fa fa fa fd fd fd fa
  0x60300028d400: fa fa fd fd fd fa fa fa fd fd fd fa fa fa 00 00
  0x60300028d480: 00 00 fa fa fd fd fd fa fa fa fd fd fd fd fa fa
  0x60300028d500: fd fd fd fd fa fa fd fd fd fa fa fa fd fd fd fa
=>0x60300028d580: fa fa fd fd fd fd fa fa fd fd fd fa fa fa[fd]fd
  0x60300028d600: fd fa fa fa fd fd fd fa fa fa fa fa fa fa fa fa
  0x60300028d680: fa fa fa fa fa fa fd fd fd fd fa fa fd fd fd fa
  0x60300028d700: fa fa fd fd fd fa fa fa fd fd fd fd fa fa fd fd
  0x60300028d780: fd fa fa fa fd fd fd fd fa fa fd fd fd fa fa fa
  0x60300028d800: fd fd fd fd fa fa fd fd fd fa fa fa fd fd fd fd
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==22257==ABORTING
`

debug wslay?