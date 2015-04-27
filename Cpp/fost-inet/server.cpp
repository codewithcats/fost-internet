/*
    Copyright 2015, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/server.hpp>


using namespace fostlib;


struct network_connection::server::state {
    boost::asio::io_service service;
    boost::asio::ip::tcp::acceptor server;
};

