cmake_minimum_required(VERSION 3.13.4)
project(TinyOAL LANGUAGES C CXX VERSION 0.1.0)

find_package(OpenAL CONFIG REQUIRED)
find_package(mpg123 CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)
find_package(flac CONFIG REQUIRED)

file(GLOB_RECURSE TinyOAL_SOURCES "./*.cpp")

if(MSVC)
  file(GLOB_RECURSE TinyOAL_HEADERS "./*.h")
  add_library(TinyOAL SHARED ${TinyOAL_SOURCES} ${TinyOAL_HEADERS})
  target_link_options(TinyOAL PRIVATE "$<$<CONFIG:Release>:/LTCG>")
else()
  add_library(TinyOAL SHARED ${TinyOAL_SOURCES})
endif()

set_property(TARGET TinyOAL PROPERTY C_STANDARD 17)
set_property(TARGET TinyOAL PROPERTY CXX_STANDARD 20)
set_property(TARGET TinyOAL PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET TinyOAL PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET TinyOAL PROPERTY VERBOSE_MAKEFILE TRUE)

retarget_output(TinyOAL)
target_include_directories(TinyOAL PUBLIC ${CMAKE_SOURCE_DIR}/bss-util/include)
target_include_directories(TinyOAL PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(TinyOAL PUBLIC ${PROJECT_SOURCE_DIR})

if(MSVC)
  target_compile_options(TinyOAL PRIVATE /Zc:preprocessor $<$<CONFIG:Release>:/Oi /Ot /GL> ${CPP_WARNINGS})
else()
  target_compile_options(TinyOAL PRIVATE -pedantic -fno-exceptions -fno-rtti $<IF:$<CONFIG:Debug>,-g -msse -msse2 -O0,-O3 -msse -msse2 -msse3 -mmmx -m3dnow -mcx16> ${CPP_WARNINGS})
target_compile_definitions(TinyOAL PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>)
endif()

target_compile_definitions(TinyOAL PRIVATE TINYOAL_EXPORTS)
target_include_directories(TinyOAL PRIVATE ${OPENGL_INCLUDE_DIRS})

if(WIN32)
  target_link_libraries(TinyOAL PRIVATE OpenAL::OpenAL MPG123::libmpg123 Vorbis::vorbisfile FLAC::FLAC++ )
else()
  target_link_libraries(TinyOAL PRIVATE OpenAL::OpenAL ${CMAKE_DL_LIBS})
endif()