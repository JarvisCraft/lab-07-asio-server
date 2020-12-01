// Copyright 2020 Petr Portnov <me@progrm-jarvis.ru>

#ifndef INCLUDE_SIMPLE_SERVER_LIB_CLIENT_HPP_
#define INCLUDE_SIMPLE_SERVER_LIB_CLIENT_HPP_

#include <chrono>
#include <optional>
#include <simple_server_lib/definitions.hpp>
#include <string>
#include <variant>

#include "user_manager.hpp"

namespace simple_server_lib {

    /**
     * @brief Timeout after which the client should be considered dead
     */
    auto constexpr TIMEOUT = ::std::chrono::seconds(5);

    /**
     * @brief Client whose connection is handled
     */
    class Client final {
    public:
        /**
         * @brief Type of socket object associated with this client
         */
        using socket_t = ip::tcp::socket;

    private:
        /**
         * @brief Socket associated with the client
         */
        socket_t socket_;

        /**
         * @brief Manager of authenticated users
         */
        UserManager& user_manager_;

        /**
         * @brief Last time when the client was proved to be alive
         */
        chrono::system_clock::time_point last_time_alive_;

        struct NotAuthenticatedState final {};
        struct AuthenticatedState final {
            /**
             * @brief Name of the user
             */
            ::std::string name;

            /**
             * @brief Associated user listener
             */
            UserManager::listener_t listener_;
        };

        /**
         * @brief Current state of this client
         */
        ::std::variant<NotAuthenticatedState, AuthenticatedState> state_;

    public:
        /**
         * @brief Creates a new client
         *
         * @param executor executor used by this client's socket
         * @param user_manager manager of the authenticated users
         */
        explicit Client(asio::io_service& executor, UserManager& user_manager);

        ~Client();

        /**
         * @brief Handles this client
         *
         * @param error reference to filled error code
         * @return {@code true} if this client is still alive and {@code false} otherwise
         */
        bool handle(error_code& error);

        /**
         * @brief Pings this client
         *
         * @param error reference to filled error code
         */
        void ping(error_code& error);

        /**
         * @brief Closes this client gracefully
         */
        void close();

        /**
         * @brief Gets a constant reference to the socket associated with the client
         *
         * @return constant reference to the socket associated with the client
         */
        [[nodiscard]] socket_t const& socket() const;

        /**
         * @brief Gets a reference to the socket associated with the client
         *
         * @return reference to the socket associated with the client
         */
        [[nodiscard]] socket_t& socket();

        // TODO void refresh_clients_list(::std::quconst&);

    private:
        /**
         * @brief Sends the given payload to the client
         *
         * @param payload payload to send to the client
         * @param error reference to filled error code
         */
        void send(::std::string const& payload, error_code& error);

        /**
         * @brief Send the list of users to the client
         *
         * @param usernames usernames to be sent to the client
         * @param error reference to filled error code
         */
        void notify_usernames_update(UserManager::usernames_t const& usernames, error_code& error);

        /**
         * @brief Disconnects the client sending the given payload
         *
         * @param payload payload to send to the client
         * @param error reference to filled error code
         */
        void disconnect(::std::string const& payload, error_code& error);
    };
} // namespace simple_server_lib

#endif // INCLUDE_SIMPLE_SERVER_LIB_CLIENT_HPP_
