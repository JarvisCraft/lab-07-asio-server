// Copyright 2020 Petr Portnov <me@progrm-jarvis.ru>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/join.hpp>
#include <simple_server_lib/client.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace simple_server_lib {

    Client::Client(asio::io_service& executor, UserManager& user_manager)
        : socket_{executor},
          user_manager_(user_manager),
          last_time_alive_{chrono::system_clock::now()},
          state_{NotAuthenticatedState{}} {}

    Client::~Client() { close(); }

    void Client::close() { socket_.close(); }

    template <typename D>
    static ::std::vector<::std::string> split(::std::string const& string, D const& delimiter) {
        ::std::vector<::std::string> result;

        ::std::string::size_type start{0}, end;
        while ((end = string.find(delimiter, start)) != ::std::string::npos) {
            result.push_back(string.substr(start, end - start));
            start = end + 1;
        }
        result.push_back(string.substr(start));

        return result;
    }

    bool Client::handle(error_code& error) {
        BOOST_LOG_TRIVIAL(info) << "<<HANDLE>>" << ::std::endl;
        ::std::string line;
        {
            asio::streambuf buffer;
            asio::read_until(socket_, buffer, DELIMITER, error);
            if (error) {
                if (error == asio::error::eof) {
                    disconnect("disconnected", error);
                    BOOST_LOG_TRIVIAL(info) << "User has disconnected" << ::std::endl;
                } else {
                    disconnect("buffer_error", error);
                    BOOST_LOG_TRIVIAL(info) << "Disconnecting user due to buffer error: " << error << ::std::endl;
                }
                return false;
            }
            ::std::istream input(&buffer);
            ::std::getline(input, line);
        }
        auto const words = split(line, ' ');

        if (words.empty()) {
            disconnect("empty_packet", error);
            BOOST_LOG_TRIVIAL(info) << "Disconnecting user due to empty payload" << ::std::endl;
            return false;
        }

        for (auto const& word : words) { BOOST_LOG_TRIVIAL(info) << "\tword<" << word << ">" << ::std::endl; }
        if (::std::holds_alternative<NotAuthenticatedState>(state_)) {
            // User *must* authenticate

            if (words[0] == "login") {
                BOOST_LOG_TRIVIAL(info) << "Logging user" << ::std::endl;
                if (words.size() > 1) {
                    ::std::string username = words[1];
                    {
                        auto const size = words.size();
                        for (::std::size_t i = 2; i < size; ++i) username += " " + words[i];
                    }
                    if (user_manager_.authenticate(username)) {
                        state_ = AuthenticatedState{username,
                                                    [this](UserManager::usernames_t const& usernames) {
                                                        error_code error;
                                                        send("client_list_changed", error);
                                                        if (error) BOOST_LOG_TRIVIAL(error)
                                                                       << "Error on attempt to update usernames: "
                                                                       << error << ::std::endl;
                                                    }};
                        user_manager_.add_update_listener(username, ::std::get<AuthenticatedState>(state_).listener_);

                        send("login ok", error);
                        BOOST_LOG_TRIVIAL(info) << "Authenticated user by name \"" << username << "\"" << ::std::endl;
                        return true;
                    }

                    disconnect("login already", error);
                    BOOST_LOG_TRIVIAL(info) << "Disconnecting user due to username \"" << username
                                            << "\" already being logged in by this username" << ::std::endl;
                    return false;
                }

                disconnect("login missing_name", error);
                BOOST_LOG_TRIVIAL(info) << "Disconnecting user due to missing username" << ::std::endl;
                return false;
            }

            disconnect("not_authenticated", error);
            BOOST_LOG_TRIVIAL(info) << "Disconnecting user due to him not being authenticated" << ::std::endl;
            return false;
        } else if (::std::holds_alternative<AuthenticatedState>(state_)) {
            // Protocol after authentication

            if (words[0] == "ping") {
                ping(error);
                BOOST_LOG_TRIVIAL(info) << "Pinged client" << ::std::endl;
                return true;
            }

            if (words[0] == "clients") {
                notify_usernames_update(user_manager_.get_authenticated(), error);
                BOOST_LOG_TRIVIAL(info) << "Sent clients to the user" << ::std::endl;
                return true;
            }

            disconnect("invalid_payload", error);
            BOOST_LOG_TRIVIAL(info) << "Disconnecting user due to unknown payload" << ::std::endl;
            return false;
        }

        throw ::std::runtime_error("Incorrect state");
    }

    void Client::ping(error_code& error) {
        send("ping_ok", error);
    }

    void Client::send(::std::string const& payload, error_code& error) {
        socket_.write_some(asio::buffer(payload + DELIMITER), error);
    }


    void Client::notify_usernames_update(UserManager::usernames_t const& usernames, error_code& error) {
        send(::boost::algorithm::join(usernames, ", "), error);
    }

    void Client::disconnect(::std::string const& payload, error_code& error) {
        if (::std::holds_alternative<AuthenticatedState>(state_)) user_manager_.remove_update_listener(
            ::std::get<AuthenticatedState>(state_).name);

        send(payload, error);
        close();

        if (::std::holds_alternative<AuthenticatedState>(state_))
            user_manager_.unauthenticate(::std::get<AuthenticatedState>(state_).name);
    }

    Client::socket_t const& Client::socket() const { return socket_; }

    Client::socket_t& Client::socket() { return socket_; }
} // namespace simple_server_lib
