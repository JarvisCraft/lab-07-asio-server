// Copyright 2020 Petr Portnov <me@progrm-jarvis.ru>

#include "simple_server_lib/user_manager.hpp"

#include <mutex>

namespace simple_server_lib {

    bool UserManager::authenticate(::std::string const& username) {
        ::std::unique_lock const lock(authenticated_usernames_mutex_);
        auto const dirty = authenticated_usernames_.insert(username).second;

        if (dirty) {
            notify_update_listeners_();
            return true;
        }

        return false;
    }

    bool UserManager::unauthenticate(::std::string const& username) {
        ::std::unique_lock const lock(authenticated_usernames_mutex_);
        auto const dirty = authenticated_usernames_.erase(username) != 0;

        if (dirty) {
            notify_update_listeners_();
            return true;
        }

        return false;
    }

    UserManager::usernames_t UserManager::get_authenticated() const {
        ::std::shared_lock const lock(authenticated_usernames_mutex_);
        return authenticated_usernames_;
    }

    void UserManager::notify_update_listeners_() const {
        ::std::shared_lock const lock(update_listeners_mutex_);
        for (auto const& listener : update_listeners_) listener.second(authenticated_usernames_);
    }

    void UserManager::add_update_listener(::std::string username, UserManager::listener_t const& listener) {
        ::std::unique_lock const lock(update_listeners_mutex_);
        update_listeners_.insert(::std::make_pair(::std::move(username), listener));
    }

    void UserManager::remove_update_listener(::std::string const& username) {
        ::std::unique_lock const lock(update_listeners_mutex_);
        update_listeners_.erase(username);
    }
} // namespace simple_server_lib
