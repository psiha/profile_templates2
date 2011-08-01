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
find_file(PROFILER_PATH profiler ${CMAKE_MODULE_PATH})

include_directories( ${Boost_INCLUDE_DIRS} )
add_executable(
    template.profiler
    ${PROFILER_PATH}/src/preprocess.cpp
    ${PROFILER_PATH}/src/preprocess.hpp
    ${PROFILER_PATH}/src/filter.cpp
    ${PROFILER_PATH}/src/filter.hpp
    ${PROFILER_PATH}/src/postprocess.cpp
    ${PROFILER_PATH}/src/postprocess.hpp
    ${PROFILER_PATH}/src/profiler.cpp
)
set_property(TARGET template.profiler PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tools)
if (NOT MSVC) # Use autolink for MSVC
    target_link_libraries(template.profiler ${Boost_LIBRARIES})
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

  get_directory_property( preprocessor_definitions COMPILE_DEFINITIONS )
  foreach(preprocessor_definition ${preprocessor_definitions})
    set(profiler_cxx_flags "${profiler_cxx_flags} -D${preprocessor_definition}")
  endforeach()

  get_directory_property(INCLUDES INCLUDE_DIRECTORIES)
  foreach(INCLUDE ${INCLUDES})
    set(INCLUDE_DIRECTORIES "${INCLUDE_DIRECTORIES} -I\"${INCLUDE}\"" )
  endforeach()

  if ( NOT EXISTS ${CMAKE_CXX_COMPILER} )
    if ( MSVC )
      get_filename_component( devenv_path ${CMAKE_MAKE_PROGRAM} PATH )
      set( full_compiler_path "${devenv_path}/../../VS/bin/${CMAKE_CXX_COMPILER}" )
      file( TO_NATIVE_PATH ${full_compiler_path} full_compiler_path )
      set( ENV{PATH} "$ENV{PATH};$(ExecutablePath)" )
    elseif()
      find_program( full_compiler_path "${CMAKE_CXX_COMPILER}" )
    endif()
  endif()

  add_custom_command(OUTPUT ${target}.template_profile
                     COMMAND echo "${profiler_cxx_flags} ${INCLUDE_DIRECTORIES}" > ${target}.compiler_options.rsp
                     COMMAND "${PROJECT_BINARY_DIR}/tools/${CMAKE_CFG_INTDIR}/template.profiler" "${full_compiler_path}" ${target}.compiler_options.rsp "${CMAKE_CURRENT_SOURCE_DIR}/${src}"
                     DEPENDS ${src}
                   )
  add_custom_target( ${target} DEPENDS ${target}.template_profile )
endfunction()
