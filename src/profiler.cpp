////////////////////////////////////////////////////////////////////////////////
///
/// profiler.cpp
/// ------------
///
/// Copyright (c) 2008-2009 Steven Watanabe
/// Copyright (c) 2011      Domagoj Saric
///
///  Use, modification and distribution is subject to the Boost Software License, Version 1.0.
///  (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "profiler.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------

#undef BOOST_ENABLE_ASSERT_HANDLER

#include <cstdio>
#include <cstdlib>
#include <exception>
#include "process.h"

extern "C"
int main( int const argc, char const * const argv[] )
{
    for ( unsigned i( 0 ); i < argc; ++i )
        std::puts( argv[ i ] );
    if ( argc != ( 1 + 3 ) )
    {
        std::puts
        (
            "Incorrect number of arguments.\n"
            "Use:\n"
            "profiler\n"
                "\t<compiler binary path>\n"
                "\t<compiler response file with options to build the target source with>\n"
                "\t<source to profile>."
        );
        return EXIT_FAILURE;
    }

    {
        char const * const compiler_argv[] = { "-E", argv[ 3 ], NULL };
        int const result( /*std*/::execv( argv[ 1 ], compiler_argv ) );
        if ( result != 0 )
        {
            std::puts( "Failed generating compiler preprocessed file." );
            return EXIT_FAILURE;
        }
    }

    try
    {
        return EXIT_SUCCESS;
    }
    catch ( std::exception const & e )
    {
        std::puts( e.what() );
        return EXIT_FAILURE;
    }
    catch ( ... )
    {
        return EXIT_FAILURE;
    }
}

//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------
