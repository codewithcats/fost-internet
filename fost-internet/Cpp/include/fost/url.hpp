/*
    Copyright 1999-2011, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_URL_HPP
#define FOST_URL_HPP
#pragma once


#include <fost/unicode>
#include <fost/host.hpp>
#include <fost/detail/tagged-string.hpp>


namespace fostlib {


    /// Represents a URL/URI
    class FOST_INET_DECLSPEC url {
    public:
        /// Tag type for file specifications
        struct FOST_INET_DECLSPEC filepath_string_tag {
            static void do_encode( fostlib::nliteral from, ascii_printable_string &into );
            static void do_encode(
                const ascii_printable_string &from, ascii_printable_string &into,
                const bool encode_slash = false
            );
            static void check_encoded( const ascii_printable_string &s );
        };
        /// File specification type
        typedef tagged_string< filepath_string_tag, ascii_printable_string >
            filepath_string;


        /// Represents a query string as generated by a form
        class FOST_INET_DECLSPEC query_string {
        public:
            /// Construct an empty query string
            query_string();
            /// Construct a query string by parsing a string
            query_string( const ascii_printable_string & );

            /// Append a new value to the query string
            void append( const string &name, const nullable< string > &value );
            /// Remove a key from the query string
            void remove( const string &name );

            nullable< ascii_printable_string > as_string() const;

        private:
            nullable< ascii_printable_string > m_string;
            std::map< string, std::list< nullable< string > > > m_query;
        };
        enum t_form { e_pathname, e_encoded };

        /// Construct a default URL
        url();
        /// Construct a URL by parsing a string
        explicit url( const string & );
        /// Construct a URL from a base and a character literal path
        url( const url &base, const char *new_path );
        /// Construct a URL from a base and a new path
        url( const url &base, const filepath_string &new_path );
        /// Construct a URL from a base and a new path
        url( const url &base, const boost::filesystem::wpath &new_path );
        url( const t_form, const string & );
        explicit url( const host &,
            const nullable< string > &username = null,
            const nullable< string > &password = null
        );
        url( const ascii_printable_string &protocol, const host &,
            const nullable< string > &username = null,
            const nullable< string > &password = null
        );

        accessors< ascii_printable_string > protocol;
        accessors< host > server;
        port_number port() const;
        accessors< nullable< string > > user;
        accessors< nullable< string > > password;
        const filepath_string &pathspec() const;
        void pathspec( const filepath_string &pathName );
        accessors< nullable< ascii_printable_string > > anchor;
        accessors< query_string, fostlib::lvalue > query;

        static setting< string > s_default_host;

        ascii_printable_string as_string() const;
        ascii_printable_string as_string( const url &relative_from ) const;

    private:
        filepath_string m_pathspec;
    };


    namespace exceptions {


        /// Thrown when a relative path can't be determined
        class FOST_INET_DECLSPEC relative_path_error : public exception {
        public:
            /// Construct a relative path exception
            relative_path_error(
                const string &base, const string &rel, const string &error
            ) throw ();

        protected:
            wliteral const message() const throw ();
        };


    }


    /// Allow a URL to be coerced to a string
    template<>
    struct FOST_INET_DECLSPEC coercer< string, url > {
        /// Performs the coercion
        string coerce( const url &u ) {
            return fostlib::coerce< string >( u.as_string() );
        }
    };
    /// Allow a string to be coerced (parsed) to a URL
    template<>
    struct FOST_INET_DECLSPEC coercer< url, string > {
        /// Performs the coercion
        url coerce( const string &u ) {
            return url( u );
        }
    };

    /// Allow a URL to be coerced to JSON
    template<>
    struct FOST_INET_DECLSPEC coercer< json, url > {
        /// Performs the coercion
        json coerce( const url &u ) {
            return json( fostlib::coerce< string >( u.as_string() ) );
        }
    };
    /// Allow JSON to be interpreted as a URL
    template<>
    struct FOST_INET_DECLSPEC coercer< url, json > {
        /// Performs the coercion
        url coerce( const json &u ) {
            return url( fostlib::coerce< string >( u ) );
        }
    };

    /// Allow a string to be turned into a file specification
    template<>
    struct FOST_INET_DECLSPEC coercer< url::filepath_string, string > {
        /// Performs the coercion
        url::filepath_string coerce( const string &s );
    };
    /// Allow a path to be turned into a file specification.
    template<>
    struct FOST_INET_DECLSPEC coercer<
        url::filepath_string, boost::filesystem::wpath
    > {
        /// Performs the coercion
        url::filepath_string coerce( const boost::filesystem::wpath &s );
    };
    /// Allow a file specification to be turned into a path.
    template<>
    struct FOST_INET_DECLSPEC coercer<
        boost::filesystem::wpath, url::filepath_string
    > {
        /// Performs the coercion
        boost::filesystem::wpath coerce( const url::filepath_string &s );
    };
    /// Allow a file specification to be turned into a string.
    template<>
    struct FOST_INET_DECLSPEC coercer<
        string, url::filepath_string
    > {
        /// Performs the coercion
        string coerce( const url::filepath_string &s ) {
            return fostlib::coerce<string>(
                fostlib::coerce<boost::filesystem::wpath>(s));
        }
    };

    /// Allow a string to be turned into a URL query object
    template<>
    struct coercer< url::query_string, string > {
        /// Performs the coercion
        url::query_string coerce(const string &s) {
            return url::query_string(fostlib::coerce<ascii_printable_string>(s));
        }
    };


}


namespace std {


    /// Allow a URL to output to a stream
    inline fostlib::ostream &operator << (
        fostlib::ostream &o, const fostlib::url &u
    ) {
        return o << fostlib::coerce< fostlib::string >( u );
    }

    /// Allow URLs to be used as keys in STL containers
    template<>
    struct less<fostlib::url> : binary_function <fostlib::url, fostlib::url, bool> {
        /// Performs the comparison of two URL instances
        bool operator () ( const fostlib::url &l, const fostlib::url &r ) const {
            return l.as_string().underlying() < r.as_string().underlying();
        }
    };


}


#endif // FOST_URL_HPP

