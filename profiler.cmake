################################################################################
#         Copyright 2003 & onward LASMEA UMR 6602 CNRS/Univ. Clermont II
#         Copyright 2009 & onward LRI    UMR 8623 CNRS/Univ Paris Sud XI
#         Copyright 2011 & onward Domagoj Saric
#
#   Based on the original from the NT2 library.
#   (https://github.com/MetaScale/nt2)
#
#          Distributed under the Boost Software License, Version 1.0.
#                 See accompanying file LICENSE.txt or copy at
#                     http://www.boost.org/LICENSE_1_0.txt
################################################################################

################################################################################
# Add the target to build profiler
################################################################################
find_path( PROFILER_PATH profiler.cmake ${CMAKE_MODULE_PATH} )

include_directories( ${Boost_INCLUDE_DIRS} )
add_executable(
    template.profiler
    EXCLUDE_FROM_ALL
    ${PROFILER_PATH}/src/filter.cpp
    ${PROFILER_PATH}/src/filter.hpp
    ${PROFILER_PATH}/src/postprocess.cpp
    ${PROFILER_PATH}/src/postprocess.hpp
    ${PROFILER_PATH}/src/preprocess.cpp
    ${PROFILER_PATH}/src/preprocess.hpp
    ${PROFILER_PATH}/src/profiler.cpp
)
set_property( TARGET template.profiler PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tools )
set_property( TARGET template.profiler PROPERTY EXCLUDE_FROM_DEFAULT_BUILD 1 )
if ( MSVC )
    # A workaround for Xpressive stack overflows
    set_property( TARGET template.profiler APPEND PROPERTY LINK_FLAGS /STACK:32000000 )
endif()
if ( NOT MSVC ) # Use autolink for MSVC
    target_link_libraries( template.profiler ${Boost_LIBRARIES} )
endif()

################################################################################
# Make a function to compile foo and profile it
################################################################################
function(template_profile target src)
    if(CMAKE_GENERATOR MATCHES "Make")
        set( profiler_cxx_flags ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}} )
    else()
        set( profiler_cxx_flags ${CMAKE_CXX_FLAGS_DEBUG} )
    endif()
  
    # Suppress linking.
    set( profiler_cxx_flags "${profiler_cxx_flags} -c" )

    get_directory_property( preprocessor_definitions COMPILE_DEFINITIONS )
    foreach(preprocessor_definition ${preprocessor_definitions})
        set(profiler_cxx_flags "${profiler_cxx_flags} -D${preprocessor_definition}")
    endforeach()

    get_directory_property(INCLUDES INCLUDE_DIRECTORIES)
    foreach(INCLUDE ${INCLUDES})
        set(INCLUDE_DIRECTORIES "${INCLUDE_DIRECTORIES} -I\"${INCLUDE}\"" )
    endforeach()
    set(INCLUDE_DIRECTORIES "${INCLUDE_DIRECTORIES} -I\"${PROFILER_PATH}\"" )

    if ( NOT EXISTS ${CMAKE_CXX_COMPILER} )
        if ( MSVC )
            #http://web.archiveorange.com/archive/v/5y7PkgJS75BcD7zzR0t4
            get_filename_component( devenv_path ${CMAKE_MAKE_PROGRAM} PATH )
            set( full_compiler_path "${devenv_path}/../../VC/bin/${CMAKE_CXX_COMPILER}" )
            file( TO_NATIVE_PATH ${full_compiler_path} full_compiler_path )
        elseif()
            find_program( full_compiler_path "${CMAKE_CXX_COMPILER}" )
        endif()
    endif()

    file( TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/${target}.compiler_options.rsp" response_file_path )

    add_custom_command(
        OUTPUT ${target}.template_profile
        COMMAND echo "${profiler_cxx_flags} ${INCLUDE_DIRECTORIES}" > "${PROJECT_BINARY_DIR}/${target}.compiler_options.rsp"
        COMMAND "${PROJECT_BINARY_DIR}/tools/${CMAKE_CFG_INTDIR}/template.profiler" "${full_compiler_path}" "${response_file_path}" "${CMAKE_CURRENT_SOURCE_DIR}/${src}" "${target}.template_profile"
        DEPENDS ${src}
    )
    
    add_custom_target( ${target} DEPENDS ${target}.template_profile )
    
    add_dependencies( ${target} template.profiler )
endfunction()
