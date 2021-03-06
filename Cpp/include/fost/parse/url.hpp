/*
    Copyright 2007-2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_PARSE_URL_HPP
#define FOST_PARSE_URL_HPP


#include <fost/url.hpp>
#include <fost/parse/host.hpp>

#include <boost/fusion/include/std_pair.hpp>


namespace fostlib {


    template<typename Iterator>
    struct url_hostpart_parser : public boost::spirit::qi::grammar <Iterator, url()> {
        boost::spirit::qi::rule<Iterator, url()> top;
        boost::spirit::qi::rule<Iterator, std::string()> moniker;
        host_parser<Iterator> host_p;

        url_hostpart_parser()
        : url_hostpart_parser::base_type(top) {
            using boost::spirit::qi::_1;
            using boost::spirit::qi::_2;
            using boost::spirit::qi::_3;
            using boost::spirit::qi::_val;

            top = (moniker >> boost::spirit::qi::string("://") >> host_p)
                [boost::phoenix::bind([&](auto &u, auto &m, auto &, auto &h) {
                        u = url(ascii_printable_string(m), h);
                    }, _val, _1, _2, _3)];

            moniker = +boost::spirit::qi::char_("a-zA-Z0-9+");
        }
    };

    template<typename I> inline
    auto url_hostpart_p(I &begin, I end, url &into) {
        url_hostpart_parser<I> rule;
        return boost::spirit::qi::parse(begin, end, rule, into);
    }


    template<typename Iterator>
    struct url_filespec_parser : public boost::spirit::qi::grammar <Iterator, ascii_printable_string()> {
        boost::spirit::qi::rule<Iterator, ascii_printable_string()> top;
        boost::spirit::qi::rule<Iterator, std::string()> line;

        url_filespec_parser()
        : url_filespec_parser::base_type(top) {
            using boost::spirit::qi::_1;
            using boost::spirit::qi::_val;

            top = line[boost::phoenix::bind(
                [](auto &v, auto s) {
                    v = ascii_printable_string(s);
                }, _val, _1)];

            line = +boost::spirit::qi::char_("_@$&~!a-zA-Z0-9/.,:'()+%*-");
        }
    };

    template<typename I> inline
    auto url_filespec_p(I &begin, I end, ascii_printable_string &into) {
        url_filespec_parser<I> rule;
        return boost::spirit::qi::parse(begin, end, rule, into);
    }


    template<typename Iterator>
    struct query_string_parser : public boost::spirit::qi::grammar<Iterator, url::query_string()> {
        using pair_type = std::pair<std::string, std::string>;
        using vector_type = std::vector<pair_type>;
        boost::spirit::qi::rule<Iterator, url::query_string()> top;
        boost::spirit::qi::rule<Iterator, std::string()> key, value;
        boost::spirit::qi::rule<Iterator, pair_type()> pair;
        boost::spirit::qi::rule<Iterator, vector_type()> query;
        boost::spirit::qi::rule<Iterator, std::string()> alternative;
        hex_char<Iterator> hex;
        boost::spirit::qi::rule<Iterator, uint8_t> plus;

        query_string_parser()
        : query_string_parser::base_type(top) {
            using boost::spirit::qi::_1;
            using boost::spirit::qi::_val;

            top = (query | alternative)[boost::phoenix::bind(
                [](auto &v, auto r) {
                    if ( auto *pv = boost::get<vector_type>(&r) ) {
                        for ( auto &&p : *pv ) {
                            if ( p.second.empty() ) {
                                v.append(p.first, null);
                            } else {
                                v.append(p.first, p.second);
                            }
                        }
                    } else {
                        auto s = boost::get<std::string>(r);
                        if ( not s.empty() ) {
                            v = url::query_string(ascii_printable_string(s));
                        }
                    }
                }, _val, _1)];

            plus = boost::spirit::qi::char_('+')[_val = ' '];
            key = (hex | plus | boost::spirit::qi::char_("_@a-zA-Z0-9..*!-"))
                >> *(hex | plus | boost::spirit::qi::char_("_@a-zA-Z0-9..*!-"));
            value = *(hex | plus | boost::spirit::qi::char_("/:_@a-zA-Z0-9().,*!=-"));
            pair = key >> boost::spirit::qi::lit('=') >> value;
            query = pair >> *(boost::spirit::qi::lit('&') >> pair);

            alternative = *boost::spirit::qi::char_("&/:_@a-zA-Z0-9.,+*%=!-");
        }
    };

    template<typename I> inline
    auto query_string_p(I &begin, I end, url::query_string &into) {
        query_string_parser<I> rule;
        return boost::spirit::qi::parse(begin, end, rule, into);
    }


    template<typename Iterator>
    struct url_parser : public boost::spirit::qi::grammar<Iterator, url()> {
        boost::spirit::qi::rule<Iterator, url()> top;
        url_hostpart_parser<Iterator> hostpart;
        url_filespec_parser<Iterator> filespec;
        query_string_parser<Iterator> query;

        url_parser()
        : url_parser::base_type(top) {
            using boost::spirit::qi::_1;
            using boost::spirit::qi::_2;
            using boost::spirit::qi::_3;
            using boost::spirit::qi::_val;

            top = (hostpart
                    >> -(boost::spirit::qi::lit('/') >> -filespec)
                    >> -(boost::spirit::qi::lit('?') >> -query))
                [boost::phoenix::bind(
                    [](auto &v, auto h, auto fs, auto qs) {
                        v = url(h, fs.value_or(ascii_printable_string()));
                        if ( qs ) v.query(qs.value());
                    }, _val, _1, _2, _3)];
        }
    };

    template<typename I> inline
    auto url_p(I &begin, I end, url &into) {
        url_parser<I> rule;
        return boost::spirit::qi::parse(begin, end, rule, into);
    }


}


#endif // FOST_PARSE_URL_HPP
