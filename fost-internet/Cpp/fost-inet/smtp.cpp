/*
    Copyright 1999-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/detail/connection.hpp>
#include <fost/detail/smtp.hpp>
#include <fost/parse/parse.hpp>

#include <fost/exception/null.hpp>
#include <fost/exception/parse_error.hpp>


using namespace fostlib;


/*
    fostlib::rfc822_address_tag
*/


void fostlib::rfc822_address_tag::do_encode( fostlib::nliteral from, ascii_printable_string &into ) {
    throw exceptions::not_implemented("fostlib::rfc822_address_tag::do_encode( fostlib::nliteral from, ascii_string &into )");
}
void fostlib::rfc822_address_tag::do_encode( const ascii_printable_string &from, ascii_printable_string &into ) {
    throw exceptions::not_implemented("fostlib::rfc822_address_tag::do_encode( const ascii_string &from, ascii_string &into )");
}
void fostlib::rfc822_address_tag::check_encoded( const ascii_printable_string &s ) {
    if ( s.empty() )
        throw exceptions::null( L"Email address is empty" );
    if ( s.underlying().find( '@' ) == string::npos )
        throw exceptions::parse_error( L"Email address doesn't contain @ symbol", coerce< string >(s) );
}


/*
    fostlib::email_address
*/


fostlib::email_address::email_address() {
}

fostlib::email_address::email_address( const rfc822_address &address, const nullable<string> &name )
: email( address ), name( name ) {
}

fostlib::email_address::email_address(const ascii_printable_string &address, const nullable< string > &name)
: email(rfc822_address(address)),
    name(name) {
}


string fostlib::coercer< string, email_address >::coerce( const email_address &e ) {
    if ( e.name().isnull() )
        return L"<" + fostlib::coerce< string >( e.email().underlying() ) + ">";
    else
        return e.name().value() + L" <" + fostlib::coerce< string >( e.email().underlying() ) + L">";
}
email_address fostlib::coercer< email_address, string >::coerce( const string &s ) {
    string name, address;
    if ( !boost::spirit::parse(s.c_str(),
        (+boost::spirit::chset< wchar_t >( L"a-zA-Z0-9_@\\.\\+-" ))[
            phoenix::var( address ) = phoenix::construct_< string >( phoenix::arg1, phoenix::arg2 )
        ]
    ).full )
        throw exceptions::not_implemented("fostlib::coercer< email_address, string >::coerce( const string &s ) -- could not parse", s);
    if ( name.empty() )
        return rfc822_address( fostlib::coerce< ascii_printable_string >( address ) );
    else
        return email_address( rfc822_address( fostlib::coerce< ascii_printable_string >( address ) ), name );
}


/*
    fostlib::smtp_server
*/


struct fostlib::smtp_client::implementation {
    network_connection cnx;
    implementation( const host &h )
    : cnx( h, 25 ) {
    }
    void check( int code, const string &command ) {
        utf8_string response, number = coerce< utf8_string >( coerce< string >( code ) );
        cnx >> response;
        if ( response.underlying().substr( 0, number.underlying().length() ) != number.underlying() ) {
            exceptions::not_implemented exception(L"SMTP response was not the one expected");
            exception.info()
                << L"Expected " << code << " but got "
                << response.underlying().substr( 0, number.underlying().length() )
                << "\nHandling command: " << command << std::endl
            ;
            throw exception;
        }
    }
};


fostlib::smtp_client::smtp_client( const host &server )
: m_impl( new implementation( server ) ) {
    m_impl->check(220, L"Initial connection");
    m_impl->cnx << "HELO FSIP\r\n";
    m_impl->check(250, L"HELO");
}

fostlib::smtp_client::~smtp_client()
try {
    m_impl->cnx << "QUIT\r\n";
    m_impl->check(221, L"QUIT");
    delete m_impl;
} catch ( ... ) {
    absorbException();
}


void fostlib::smtp_client::send(const mime &email, const rfc822_address &to, const rfc822_address &from) {
    m_impl->cnx << "MAIL FROM:<" + from.underlying().underlying() + ">\r\n";
    m_impl->check(250, L"MAIL FROM");
    m_impl->cnx << "RCPT TO:<" + to.underlying().underlying() + ">\r\n";
    m_impl->check(250, L"RCPT TO");
    m_impl->cnx << "DATA\r\n";
    m_impl->check(354, L"DATA");
    for ( mime::mime_headers::const_iterator h( email.headers().begin() ); h != email.headers().end(); ++h ) {
        std::stringstream ss;
        ss << h->first << ": " << h->second << "\r\n";
        if ( ss.str().length() > 82 )
            throw exceptions::not_implemented(
                "Header is too wide for SMTP. Folding not implemented",
                coerce< string >( utf8_string(ss.str()) )
            );
        m_impl->cnx << ss;
    }
    m_impl->cnx << "\r\n";
    for ( mime::const_iterator d( email.begin() ); d != email.end(); ++d )
        m_impl->cnx << *d;
    m_impl->cnx << "\r\n.\r\n";
    m_impl->check(250, L"Data spooling");
}
