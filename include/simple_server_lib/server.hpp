// Copyright 2020 Petr Portnov <me@progrm-jarvis.ru>

#ifndef INCLUDE_SIMPLE_SERVER_LIB_SERVER_HPP_
#define INCLUDE_SIMPLE_SERVER_LIB_SERVER_HPP_

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <simple_server_lib/client.hpp>
#include <simple_server_lib/definitions.hpp>
#include <simple_server_lib/user_manager.hpp>
#include <thread>
#include <vector>

namespace simple_server_lib {

    /**
     * @brief Client wrapped in a shared pointer
     */
    using SharedClient = ::std::shared_ptr<Client>;

    /**
     * @brief Basic synchronous server
     */
    class Server final {
    public:
        /**
         * @brief Properties to be used by the server
         */
        struct Properties {
            /**
             * @brief Server endpoint
             */
            ip::tcp::endpoint endpoint;

            /**
             * @brief Amount of worker threads
             */
            ::std::size_t worker_count;

            /**
             * @brief Timeout after which the client should be considered dead
             */
            chrono::seconds client_timeout;
        };

    private:
        /**
         * @brief Configuration of this server
         */
        Properties const properties_;

        /**
         * @brief Marker being set to {@code true} once the server should get shut down
         */
        ::std::atomic_bool shutdown_marker_{false};

        /**
         * @brief Internal IO-service
         */
        asio::io_service service_;

        /**
         * @brief Queue of handled clients
         */
        ::std::queue<SharedClient> clients_;

        /**
         * @brief Lock under which access to `clients_` should happen
         */
        mutable ::std::mutex clients_mutex_;

        /**
         * @brief Boss thread
         */
        ::std::thread boss_;

        /**
         * @brief Worker thread
         */
        ::std::vector<::std::thread> workers_;

        /**
         * @brief Manager of authenticated users
         */
        UserManager user_manager_;

    public:
        /**
         * @brief Creates a new server with the given properties
         *
         * @param properties properties of the server
         */
        explicit Server(Properties properties);

        /**
         * @brief Stops this server gracefully
         */
        void stop();

    private:
        /**
         * @brief Handles incoming client connections synchronously
         */
        void handle_client_connections();

        /**
         * @brief Handles connected clients synchronously
         */
        void handle_clients();
    };
} // namespace simple_server_lib

#endif // INCLUDE_SIMPLE_SERVER_LIB_SERVER_HPP_
