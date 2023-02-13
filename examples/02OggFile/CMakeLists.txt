cmake_minimum_required(VERSION 3.13.4)
project(OggFile LANGUAGES C CXX VERSION 0.1.0)

file(GLOB_RECURSE OggFile_SOURCES "./*.cpp")

if(MSVC)
  file(GLOB_RECURSE OggFile_HEADERS "./*.h")
  add_executable(OggFile ${OggFile_SOURCES} ${OggFile_HEADERS})
  target_link_options(OggFile PRIVATE "$<$<CONFIG:Release>:/LTCG>")
else()
  add_executable(OggFile ${OggFile_SOURCES})
endif()

set_property(TARGET OggFile PROPERTY C_STANDARD 17)
set_property(TARGET OggFile PROPERTY CXX_STANDARD 20)
set_property(TARGET OggFile PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET OggFile PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET OggFile PROPERTY VERBOSE_MAKEFILE TRUE)

retarget_output(OggFile)
target_include_directories(OggFile PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(OggFile PUBLIC ${PROJECT_SOURCE_DIR})

if(MSVC)
  target_compile_options(OggFile PRIVATE /Zc:preprocessor $<$<CONFIG:Release>:/Oi /Ot /GL> ${CPP_WARNINGS})
else()
  target_compile_options(OggFile PRIVATE -pedantic -fno-exceptions -fno-rtti $<IF:$<CONFIG:Debug>,-g -msse -msse2 -O0,-O3 -msse -msse2 -msse3 -mmmx -m3dnow -mcx16> ${CPP_WARNINGS})
target_compile_definitions(OggFile PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>)
endif()

target_include_directories(OggFile PRIVATE ${OPENGL_INCLUDE_DIRS})

if(WIN32)
  target_link_libraries(OggFile PRIVATE TinyOAL)
else()
  target_link_libraries(OggFile PRIVATE TinyOAL ${CMAKE_DL_LIBS})
endif()