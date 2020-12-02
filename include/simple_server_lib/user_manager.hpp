// Copyright 2020 Petr Portnov <me@progrm-jarvis.ru>

#ifndef INCLUDE_SIMPLE_SERVER_LIB_USER_MANAGER_HPP_
#define INCLUDE_SIMPLE_SERVER_LIB_USER_MANAGER_HPP_

#include <functional>
#include <map>
#include <set>
#include <shared_mutex>
#include <string>

namespace simple_server_lib {

    class UserManager {
    public:
        /**
         * @brief Type of set of usernames
         */
        using usernames_t = ::std::set<::std::string>;

        /**
         * @brief Type of listener used for notification on usernames' updates
         */
        using listener_t = ::std::function<void(::std::set<::std::string> const&)>;

    private:
        /**
         * @brief Names of authenticated users
         */
        usernames_t authenticated_usernames_;

        /**
         * @brief Mutex for accessing `authenticated_usernames_`
         */
        mutable ::std::shared_mutex authenticated_usernames_mutex_;

        /**
         * @brief Listeners notified on updates to {@code authenticated_usernames_}
         */
        ::std::map<::std::string, listener_t> update_listeners_;

        /**
         * @brief Mutex for accessing `update_listeners_`
         */
        mutable ::std::shared_mutex update_listeners_mutex_;

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
        [[nodiscard]] usernames_t get_authenticated() const;

        /**
         * @brief Adds an update listener
         *
         * @param username name of the user whose client is owning the listener
         * @param listener update listener to be added
         */
        void add_update_listener(::std::string username, listener_t const& listener);

        /**
         * @brief Removes an update listener
         *
         * @param username name of the user whose client is owning the listener
         */
        void remove_update_listener(::std::string const& username);

    private:
        /**
         * @brief Notifies all of {@code update_listeners_}
         *
         * @note does not acquire {@code authenticated_usernames_mutex_} when reading {@code authenticated_usernames_}
         */
        void notify_update_listeners_() const;
    };
} // namespace simple_server_lib

#endif // INCLUDE_SIMPLE_SERVER_LIB_USER_MANAGER_HPP_
