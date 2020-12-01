// Copyright 2020 Petr Portnov <me@progrm-jarvis.ru>

#include <simple_server_lib/server.hpp>
#include <utility>
#include <thread>
#include <boost/log/trivial.hpp>

namespace simple_server_lib {

    Server::Server(Server::Properties properties) : properties_(std::move(properties)),
                                                    boss_(&Server::handle_client_connections, this),
                                                    workers_(properties.worker_count) {
        for (auto& worker : workers_) worker = ::std::thread(&Server::handle_clients, this);
    }

    void Server::stop() {
        BOOST_LOG_TRIVIAL(info) << "Shutting down..." << ::std::endl;
        shutdown_marker_ = true;

        BOOST_LOG_TRIVIAL(info) << "Stopping IO-service" << ::std::endl;
        service_.stop();
        BOOST_LOG_TRIVIAL(info) << "IO-service has been stopped successfully" << ::std::endl;

        BOOST_LOG_TRIVIAL(info) << "Waiting for boss thread" << ::std::endl;
        boss_.join();
        BOOST_LOG_TRIVIAL(info) << "Boss thread has been shut down successfully" << ::std::endl;

        BOOST_LOG_TRIVIAL(info) << "Waiting for worker threads" << ::std::endl;
        for (auto &worker : workers_) worker.join();
        BOOST_LOG_TRIVIAL(info) << "Worker threads have been shut down successfully" << ::std::endl;

        BOOST_LOG_TRIVIAL(info) << "Shut down successfully" << ::std::endl;
    }

    void Server::handle_client_connections() {
        ip::tcp::acceptor acceptor(service_, properties_.endpoint);
        while (!shutdown_marker_) {
            auto client = ::std::make_shared<Client>(service_, user_manager_);

            {
                ::boost::system::error_code error;
                BOOST_LOG_TRIVIAL(info) << "Waiting for new connections" << ::std::endl;
                acceptor.accept(client->socket(), error); // this will wait until the client can be started
                if (error) break;
            }

            { // add the newly created client to the queue
                ::std::scoped_lock const lock(clients_mutex_);
                clients_.push(::std::move(client));
            }
        }
    }

    void Server::handle_clients() {
        while (!shutdown_marker_) {
            SharedClient client;
            // get lock to attempt to get a client
            {
                ::std::scoped_lock const lock(clients_mutex_);
                if (clients_.empty()) continue; // reattempt making it possible for other worker to get his lock
                client = clients_.front();
                clients_.pop();
            }

            if ((chrono::system_clock::now() - client->last_time_alive()) > properties_.client_timeout) {
                error_code error;
                client->disconnect_inactive(error);
                if (error) BOOST_LOG_TRIVIAL(error)
                               << "An error happened while disconnecting a client " << error << ::std::endl;
            }

            error_code error;
            // add the client to the queue if it still is alive
            if (client->handle(error) && !error) {
                ::std::scoped_lock const lock(clients_mutex_);
                clients_.push(client);
            }
        }
    }
} // namespace simple_server_lib
