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
#undef BOOST_ENABLE_ASSERT_HANDLER

#include "filter.hpp"
#include "postprocess.hpp"
#include "preprocess.hpp"

#include "boost/assert.hpp"
#include "boost/config.hpp"

// POSIX implementation
#if defined( BOOST_HAS_UNISTD_H )
    #include "unistd.h"
#elif defined( BOOST_MSVC )
    #pragma warning ( disable : 4996 ) // "The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name."
    #include "io.h"
#else
    #error unknown or no POSIX implementation
#endif // BOOST_HAS_UNISTD_H
#include "fcntl.h"
#include "sys/stat.h"
#include "sys/types.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include "process.h"
#include <string>
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------

extern "C"
int main( int const argc, char const * const argv[] )
{
    if ( argc != ( 1 + 4 ) )
    {
        std::puts
        (
            "Incorrect number of arguments.\n"
            "Use:\n"
            "profiler\n"
                "\t<compiler binary path>\n"
                "\t<compiler response file with options to build the target source with>\n"
                "\t<source to profile>\n"
                "\t<result file name>."
        );
        return EXIT_FAILURE;
    }

    char const * const compiler_binary       ( argv[ 1 ] );
    char const * const compiler_response_file( argv[ 2 ] );
    char const * const source_to_profile     ( argv[ 3 ] );
    char const * const result_file           ( argv[ 4 ] );

    try
    {

        static char const compiler_preprocessed_file[] = "template_profiler.compiler_preprocessed.i";

        {
            std::string const full_command_line( std::string( compiler_binary ) + " " + source_to_profile + " @" + compiler_response_file + " -E > " + compiler_preprocessed_file );
            int const result( /*std*/::system( full_command_line.c_str() ) );
            if ( result != 0 )
            {
                std::puts( "Failed generating compiler preprocessed file." );
                return EXIT_FAILURE;
            }
        }

        static char const prepared_file_to_compile[] = "template_profiler.preprocessed.cpp";
        {
            std::string preprocessed_input;
            std::string filtered_input    ;
            preprocess     ( compiler_preprocessed_file, preprocessed_input );
            copy_call_graph( preprocessed_input        , filtered_input     );

            int const file_id( /*std*/::open( prepared_file_to_compile, O_CREAT | O_TRUNC | O_WRONLY, S_IREAD | S_IWRITE ) );
            if ( file_id < 0 )
            {
                std::puts( "Failed creating an intermediate file." );
                return EXIT_FAILURE;
            }
            int const write_result( /*std*/::write( file_id, &filtered_input[ 0 ], filtered_input.size() ) );
            BOOST_VERIFY( /*std*/::close( file_id ) == 0 );
            if ( write_result < 0 )
            {
                std::puts( "Failed writing to an intermediate file." );
                return EXIT_FAILURE;
            }
        }

        static char const final_compiler_output[] = "template_profiler.final_compiler_output.txt";
        {
            std::string const full_command_line( std::string( compiler_binary ) + " " + prepared_file_to_compile + " @" + compiler_response_file + " > " + final_compiler_output );
            int const result( /*std*/::system( full_command_line.c_str() ) );
            if ( result != 0 )
            {
                std::puts( "Failed compiling an intermediate file." );
                return EXIT_FAILURE;
            }
        }

        postprocess( final_compiler_output, result_file );

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
