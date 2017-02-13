/*
    Copyright 2016-2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/internet>
#include <fost/rask/counters.hpp>
#include <fost/rask/connection.hpp>
#include <fost/rask/packet.hpp>

#include <boost/asio/spawn.hpp>


namespace fostlib {


    /// Module for TCP
    extern const module c_rask_proto_tcp;


    /// TCP connection
    class rask_tcp : public rask_connection<boost::asio::ip::tcp::socket> {
    protected:
        rask_tcp(boost::asio::io_service &ios)
        : rask_connection<boost::asio::ip::tcp::socket>(server_side), socket(ios) {
        }

        boost::asio::io_service &get_io_service() override {
            return socket.get_io_service();
        }

    public:
        /// The socket for this connection
        boost::asio::ip::tcp::socket socket;
    };


    /// Counters used for TCP
    extern rask_counters rask_tcp_counters;

    /// Listen for a connection. When a connection is established the factory
    /// is used to create a connection object which is then expected to handle
    /// the ongoing connection.
    void tcp_listen(boost::asio::io_service &ios, fostlib::host netloc,
        std::function<std::shared_ptr<rask_tcp>(boost::asio::ip::tcp::socket)> factory);


    /// A loop implementation for receiving the inbound packets. The
    /// signature for the Dispatch handler (lambda) is:
    ///     (auto decoder, uint8_t control, std::size_t bytes, boost::asio::streambuf &&buffer)
    template<typename Dispatch> inline
    void receive_loop(
        rask_tcp &cnx, boost::asio::yield_context &yield, Dispatch dispatch
    ) {
        while ( cnx.socket.is_open() ) {
            try {
                rask::decoder<boost::asio::ip::tcp::socket> decode(cnx.socket, yield);
                std::size_t packet_size = decode.read_size();
                rask::control_byte control = decode.read_byte();
                decode.transfer(packet_size);
                fostlib::log::debug(fostlib::c_rask_proto)
                    ("", "Got packet")
                    ("connection", cnx.id)
                    ("control", control)
                    ("size", packet_size);
                dispatch(std::move(decode), control, packet_size);
            } catch ( ... ) {
                cnx.socket.close();
                fostlib::log::error(c_rask_proto)
                    ("", "Socket error - exception caught")
                    ("connection", cnx.id);
            }
        }
        fostlib::log::info(fostlib::c_rask_proto)
            ("", "Connection closed")
            ("connection", cnx.id);
    }


}


/// Implementation of TCP data send for outbound packets
template<> inline
void rask::out_packet::operator () (
    boost::asio::ip::tcp::socket &cnx, boost::asio::yield_context &yield
) const {
    boost::asio::streambuf header;
    size_sequence(size(), header);
    header.sputc(control);
    std::array<boost::asio::streambuf::const_buffers_type, 2>
        data{{header.data(), buffer->data()}};
    async_write(cnx, data, yield);
}


/// Implementation for transfer for TCP
template<> inline
void rask::decoder<boost::asio::ip::tcp::socket>::transfer(std::size_t bytes) {
    if ( socket ) {
        boost::asio::async_read(*socket, *input_buffer,
            boost::asio::transfer_exactly(bytes), *yield);
    }
}

