cmake_minimum_required(VERSION 3.13.4)
project(WavFile LANGUAGES C CXX VERSION 0.1.0)

file(GLOB_RECURSE WavFile_SOURCES "./*.cpp")

if(MSVC)
  file(GLOB_RECURSE WavFile_HEADERS "./*.h")
  add_executable(WavFile ${WavFile_SOURCES} ${WavFile_HEADERS})
  target_link_options(WavFile PRIVATE "$<$<CONFIG:Release>:/LTCG>")
else()
  add_executable(WavFile ${WavFile_SOURCES})
endif()

set_property(TARGET WavFile PROPERTY C_STANDARD 17)
set_property(TARGET WavFile PROPERTY CXX_STANDARD 20)
set_property(TARGET WavFile PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET WavFile PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET WavFile PROPERTY VERBOSE_MAKEFILE TRUE)

retarget_output(WavFile)
target_include_directories(WavFile PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(WavFile PUBLIC ${PROJECT_SOURCE_DIR})

if(MSVC)
  target_compile_options(WavFile PRIVATE /Zc:preprocessor $<$<CONFIG:Release>:/Oi /Ot /GL> ${CPP_WARNINGS})
else()
  target_compile_options(WavFile PRIVATE -pedantic -fno-exceptions -fno-rtti $<IF:$<CONFIG:Debug>,-g -msse -msse2 -O0,-O3 -msse -msse2 -msse3 -mmmx -m3dnow -mcx16> ${CPP_WARNINGS})
target_compile_definitions(WavFile PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>)
endif()

target_include_directories(WavFile PRIVATE ${OPENGL_INCLUDE_DIRS})

if(WIN32)
  target_link_libraries(WavFile PRIVATE TinyOAL)
else()
  target_link_libraries(WavFile PRIVATE TinyOAL ${CMAKE_DL_LIBS})
endif()