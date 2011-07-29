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
# Add the target to build filter and postprocess
################################################################################
FIND_FILE(PROFILER_PATH profiler ${CMAKE_MODULE_PATH})

ADD_EXECUTABLE(template.profiler.filter ${PROFILER_PATH}/filter.cpp)
set_property(TARGET template.profiler.filter PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tools)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
ADD_EXECUTABLE(template.profiler.postprocess ${PROFILER_PATH}/postprocess.cpp)
if (NOT MSVC)
    target_link_libraries(template.profiler.postprocess ${Boost_LIBRARIES})
endif()
set_property(TARGET template.profiler.postprocess PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tools)

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
    list(APPEND profiler_cxx_flags -D${preprocessor_definition})
  endforeach()

  get_directory_property(INCLUDES INCLUDE_DIRECTORIES)
  foreach(INCLUDE ${INCLUDES})
    list(APPEND INCLUDE_DIRECTORIES -I"${INCLUDE}")
  endforeach()
  
  add_custom_command(OUTPUT ${target}.preprocessed.cpp
                     COMMAND ${CMAKE_CXX_COMPILER} ${profiler_cxx_flags} ${INCLUDE_DIRECTORIES} -E ${CMAKE_CURRENT_SOURCE_DIR}/${src}
                           | ${PERL_EXECUTABLE} ${PROFILER_PATH}/preprocess.pl > ${target}.preprocessed.cpp
                     DEPENDS ${src}
                   )
  add_custom_command(OUTPUT ${target}.template_profile
                     COMMAND ${CMAKE_CXX_COMPILER} ${profiler_cxx_flags} -I${PROFILER_PATH} ${INCLUDE_DIRECTORIES} -c -DPROFILE_TEMPLATES ${target}.preprocessed.cpp 2>&1
                           | \"${PROJECT_BINARY_DIR}/tools/${CMAKE_CFG_INTDIR}/template.profiler.filter\" ${ARGN} > ${target}.filtered && \"${PROJECT_BINARY_DIR}/tools/${CMAKE_CFG_INTDIR}/template.profiler.postprocess\" ${ARGN} ${target}.filtered > ${target}.template_profile
                     DEPENDS ${target}.preprocessed.cpp
                             template.profiler.filter
                             template.profiler.postprocess 
                    )
  add_custom_target (${target}
                     DEPENDS ${target}.template_profile
                    )
endfunction()
