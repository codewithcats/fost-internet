/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_INTERNET_HTTP_USERAGENT_HPP
#define FOST_INTERNET_HTTP_USERAGENT_HPP
#pragma once


#include <fost/detail/http.hpp>


namespace fostlib {


    namespace http {


        class FOST_INET_DECLSPEC user_agent : boost::noncopyable {
        public:
            user_agent();

            class response : public mime {
                friend class user_agent;
                response(
                    std::auto_ptr< network_connection > connection,
                    const string &m, const url &u
                );
            public:
                accessors< const string > method;
                accessors< const url > location;

                std::auto_ptr< mime > body();
            private:
                std::auto_ptr< network_connection > m_cnx;
            };
            class request : public text_body {
                friend class user_agent;
                request(const user_agent &ua, const string &method, const url &url, const nullable< string > &data  = null);
            public:
                accessors< const string > method;
                accessors< const url > address;

                std::auto_ptr< response > operator () ();
            };

            accessors< nullable< boost::function< void ( request & ) > > > authentication;

            std::auto_ptr< response > operator () ( const string &method, const url &url, const nullable< string > &data  = null );

            std::auto_ptr< response > get( const url &url ) {
                return (*this)( L"GET", url, null );
            }
            std::auto_ptr< response > post( const url &url, const string &data ) {
                return (*this)( L"POST", url, data );
            }

        private:
            boost::asio::io_service m_service;
        };


    }


}


#endif // FOST_INTERNET_HTTP_USERAGENT_HPP
