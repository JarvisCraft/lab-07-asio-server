// Copyright 2020 Petr Portnov <me@progrm-jarvis.ru>

#include "simple_server_lib/user_manager.hpp"

namespace simple_server_lib {

    bool UserManager::authenticate(::std::string const& username) {
        ::std::lock_guard const lock(authenticated_usernames_mutex_);
        return authenticated_usernames_.insert(username).second;
    }

    bool UserManager::unauthenticate(::std::string const& username) {
        ::std::lock_guard const lock(authenticated_usernames_mutex_);
        return authenticated_usernames_.erase(username) != 0;
    }

    ::std::set<::std::string> UserManager::get_authenticated() const {
        ::std::lock_guard const lock(authenticated_usernames_mutex_);
        return authenticated_usernames_;
    }
} // namespace simple_server_lib
