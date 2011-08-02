////////////////////////////////////////////////////////////////////////////////
///
/// \file preprocess.cpp
/// --------------------
///
/// Copyright (c) 2008.-2009. Steven Watanabe (preprocess.pl)
/// Copyright (c) 2011.       Domagoj Saric
///
///  Use, modification and distribution is subject to the
///  Boost Software License, Version 1.0.
///  (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#undef BOOST_ENABLE_ASSERT_HANDLER

#define BOOST_XPRESSIVE_USE_C_TRAITS

#include "preprocess.hpp"

#include "boost/assert.hpp"
#include "boost/concept_check.hpp"
#include "boost/interprocess/file_mapping.hpp"
#include "boost/interprocess/mapped_region.hpp"
#include "boost/range/iterator_range_core.hpp"
#include "boost/xpressive/xpressive.hpp"

#include <iterator>
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------

namespace regex
{
    using namespace boost::xpressive;

    cregex make_parens()
    {
        cregex parens; //parens = keep( '(' >> *keep( keep( +keep( ignored | ~(set= '(',')') ) | ( -!by_ref( parens ) ) ) ) >> ')' );
        // Example from Xpressive documentation:
        parens =
        '('                            // is an opening parenthesis ...
         >>                           // followed by ...
          *(                          // zero or more ...
             keep( +~(set='(',')') )  // of a bunch of things that are not parentheses ...
           |                          // or ...
             by_ref(parens)      // a balanced set of parentheses
           )                          //   (ooh, recursion!) ...
         >>                           // followed by ...
        ')'                            // a closing parenthesis
        ;
        return parens;
    }

    //cregex const backslashed_lines = keep( keep( *( '\\' >> keep( _ln ) | ~_ln ) ) ); //...zzz...!?
    cregex const string            = keep( '"' >> *keep( as_xpr( "\\\\" ) | "\\\"" | ~(set='"') ) >> '"' | '\'' >> *keep( as_xpr( "\\\\" ) | "\\'" | ~(set='\'') ) >> '\'' );
    cregex const comment           = keep( "//" /*>> backslashed_lines*/ | "/*" >> keep( *( ~(set='*') | '*' >> ~before('/') ) ) >> "*/" );
    cregex const pp                = keep( '#' >> /*backslashed_lines*/ -*~_ln >> _ln );
    cregex const ignored           = keep( string | comment | pp );
    cregex const parens            = make_parens();
    cregex const ws                = comment | pp | _s | _ln;

    cregex const class_header =
        keep
        (
            keep( _b >> ( as_xpr( "class" ) | "struct" ) ) >>
            keep( +ws >> +_w                             ) >>
            keep( *keep( ~(set= '(',')','{',';','=') | parens | ignored ) ) >>
            '{'
        );

    cregex const control    = ( _b >> ( as_xpr( "__attribute__" ) | "__if_exists" | "__if_not_exists" | "for" | "while" | "if" | "catch" | "switch" ) >> _b );
    cregex const modifiers  = ( _b >> ( as_xpr( "try" ) | "const" | "volatile" ) >> _b );
    cregex const start      = ( bos /*| cregex::compile( "\\G" )*/ | after( (set= '{','}',';') ) ) >> keep( *ws );
    cregex const body       = ~before( control ) >> keep( ignored | ~(set= '{','}',';') );
    cregex const end        = parens | ']';
    cregex const body_start = keep( *ws >> *(modifiers >> *ws) >> '{' );

    cregex const function_header = keep( start >> ( *body >> end ) >> body_start );
} // namespace regex

struct formatter : boost::noncopyable
{
    template<typename Out>
    Out operator()( regex::cmatch const & what, Out out ) const
    {
        using namespace regex;

        typedef cmatch::value_type sub_match;

        BOOST_ASSERT( what.size() == 5 );

        cmatch::const_iterator const p_match( std::find_if( what.begin() + 1, what.end(), []( sub_match const & match ){ return match.matched; } ) );
        BOOST_ASSERT_MSG( p_match != what.end(), "Something should have matched." );
        sub_match const & match( *p_match );

        enum match_type_t
        {
            ignore = 1,
            header,
            open_brace,
            close_brace
        };

        unsigned int const match_type( p_match - what.begin() );
        switch ( match_type )
        {
            case ignore:
                out = std::copy( match.first, match.second, out );
                break;

            case header:
            {
                braces.push_back( " TEMPLATE_PROFILE_EXIT() }" );
                static char const tail[] = " TEMPLATE_PROFILE_ENTER()";
                out = std::copy( match.first         , match.second          , out );
                out = std::copy( boost::begin( tail ), boost::end( tail ) - 1, out );
                break;
            }

            case open_brace:
                braces.push_back( "}" );
                out = std::copy( match.first, match.second, out );
                break;

            case close_brace:
                out = std::copy( braces.back().begin(), braces.back().end(), out );
                braces.pop_back();
                break;

            default:
                BOOST_ASSERT( false );
                break;
        }

        return out;
    }

    mutable std::vector<std::string> braces;
};


void preprocess( char const * const p_filename, std::string & buffer )
{
    using namespace boost;

    interprocess::mapped_region const input_file_view
    (
        interprocess::file_mapping
        (
            p_filename,
            interprocess::read_only
        ),
        interprocess::read_only
    );

    buffer.reserve( input_file_view.get_size() );

    iterator_range<char const *> input
    (
        static_cast<char const *>( input_file_view.get_address() ),
        static_cast<char const *>( input_file_view.get_address() ) + input_file_view.get_size()
    );

    regex::match_results<char const *> search_results;
    using namespace regex;
    cregex const main_regex( (s1= ignored) | (s2=keep( class_header | function_header )) | (s3='{') | (s4='}') );

    BOOST_ASSERT( buffer.empty() );
    buffer = "#include <template_profiler.hpp>\n";

    // Implementation note:
    //   The whole file has to be searched at once in order to handle class/
    // function definitions over several lines.
    //                                    (01.08.2011.) (Domagoj Saric)
    regex_replace
    (
        std::back_insert_iterator<std::string>( buffer ),
        input.begin(),
        input.end  (),
        main_regex,
        formatter()
    );
}

//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------
