// Copyright 2020 Petr Portnov <gh@progrm-jarvis.ru>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/keywords/file_name.hpp>
#include <boost/log/keywords/format.hpp>
#include <boost/log/keywords/rotation_size.hpp>
#include <boost/log/keywords/time_based_rotation.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <simple_server_lib.hpp>

namespace simple_server_app {
    static void setup_logging();
} // namespace simple_server_app

int main(int const arguments_count, char const* arguments[]) {
    ::simple_server_app::setup_logging();

    if (arguments_count < 2) {
        BOOST_LOG_TRIVIAL(error) << "Missing port parameter" << ::std::endl;
        return 1;
    }

    in_port_t port;
    try {
        port = ::std::stoi(arguments[1]);
    } catch (::std::invalid_argument const& e) {
        BOOST_LOG_TRIVIAL(error) << "Couldn't parse port: " << e.what() << ::std::endl;
        return 1;
    } catch (::std::out_of_range const& e) {
        BOOST_LOG_TRIVIAL(error) << "Couldn't parse port: " << e.what() << ::std::endl;
        return 1;
    }

    BOOST_LOG_TRIVIAL(info) << "Starting server at localhost:" << port << ::std::endl;
    ::simple_server_lib::Server server(::simple_server_lib::Server::Properties{
        ::simple_server_lib::ip::tcp::endpoint{::simple_server_lib::ip::tcp::v4(), port}, 2
    });
    BOOST_LOG_TRIVIAL(info) << "Starting has been successfully started" << ::std::endl;

    ::std::string input;
    while (input != "stop") ::std::getline(::std::cin, input);
    server.stop();

    return 0;
}

namespace simple_server_app {

    void setup_logging() {
        namespace logging = ::boost::log;
        namespace sources = logging::sources;
        namespace expressions = logging::expressions;
        namespace keywords = logging::keywords;
        namespace sinks = logging::sinks;

        logging::add_common_attributes();

        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);

        auto const format = expressions::stream << '['
                                                << expressions::format_date_time<::boost::posix_time::ptime>(
                                                       "TimeStamp", "%Y-%m-%d %H:%M:%S")
                                                << ']' << " (" << logging::trivial::severity
                                                << "): " << expressions::smessage;

        logging::add_console_log()->set_formatter(format);
        logging::add_file_log(keywords::file_name = "logs/file_%5N.log", keywords::rotation_size = 5ul << 20u,
                              keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0),
                              keywords::format = format);
    }
} // namespace simple_server_app
