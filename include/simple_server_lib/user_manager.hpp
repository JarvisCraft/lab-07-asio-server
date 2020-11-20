// Copyright 2020 Petr Portnov <me@progrm-jarvis.ru>

#ifndef INCLUDE_SIMPLE_SERVER_LIB_USER_MANAGER_HPP_
#define INCLUDE_SIMPLE_SERVER_LIB_USER_MANAGER_HPP_

#include <mutex>
#include <set>
#include <string>

namespace simple_server_lib {

    class UserManager {
        /**
         * @brief Names of authenticated users
         */
        ::std::set<::std::string> authenticated_usernames_;

        /**
         * @brief Mutex for accessing `authenticated_usernames_`
         */
        mutable ::std::mutex authenticated_usernames_mutex_;

    public:
        /**
         * @brief Attempt to authenticate user by the given name
         *
         * @param username name of a user
         * @return {@code true} if the user was authenticated and {@code false} otherwise
         */
        bool authenticate(::std::string const& username);

        /**
         * @brief Attempt to un-authenticate user by the given name
         *
         * @param username name of a user
         * @return {@code true} if the user was un-authenticated and {@code false} otherwise
         */
        bool unauthenticate(::std::string const& username);

        /**
         * @brief Gets a set of usernames
         *
         * @return set of all authenticated usernames
         */
        [[nodiscard]] ::std::set<::std::string> get_authenticated() const;
    };
} // namespace simple_server_lib

#endif // INCLUDE_SIMPLE_SERVER_LIB_USER_MANAGER_HPP_
