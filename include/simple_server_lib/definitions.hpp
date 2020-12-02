// Copyright 2020 Petr Portnov <me@progrm-jarvis.ru>

#ifndef INCLUDE_SIMPLE_SERVER_LIB_DEFINITIONS_HPP_
#define INCLUDE_SIMPLE_SERVER_LIB_DEFINITIONS_HPP_

#include <boost/asio.hpp>
#include <chrono>

namespace simple_server_lib {

    namespace chrono = ::std::chrono;
    namespace asio = ::boost::asio;
    namespace ip = asio::ip;

    using ::boost::system::error_code;

    /**
     * @brief Delimiter used by the implemented protocol
     */
    auto constexpr DELIMITER = '\n';
} // namespace simple_server_lib

#endif // INCLUDE_SIMPLE_SERVER_LIB_DEFINITIONS_HPP_
