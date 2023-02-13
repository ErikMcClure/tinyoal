cmake_minimum_required(VERSION 3.13.4)
project(Mp3File LANGUAGES C CXX VERSION 0.1.0)

file(GLOB_RECURSE Mp3File_SOURCES "./*.cpp")

if(MSVC)
  file(GLOB_RECURSE Mp3File_HEADERS "./*.h")
  add_executable(Mp3File ${Mp3File_SOURCES} ${Mp3File_HEADERS})
  target_link_options(Mp3File PRIVATE "$<$<CONFIG:Release>:/LTCG>")
else()
  add_executable(Mp3File ${Mp3File_SOURCES})
endif()

set_property(TARGET Mp3File PROPERTY C_STANDARD 17)
set_property(TARGET Mp3File PROPERTY CXX_STANDARD 20)
set_property(TARGET Mp3File PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET Mp3File PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET Mp3File PROPERTY VERBOSE_MAKEFILE TRUE)

retarget_output(Mp3File)
target_include_directories(Mp3File PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Mp3File PUBLIC ${PROJECT_SOURCE_DIR})

if(MSVC)
  target_compile_options(Mp3File PRIVATE /Zc:preprocessor $<$<CONFIG:Release>:/Oi /Ot /GL> ${CPP_WARNINGS})
else()
  target_compile_options(Mp3File PRIVATE -pedantic -fno-exceptions -fno-rtti $<IF:$<CONFIG:Debug>,-g -msse -msse2 -O0,-O3 -msse -msse2 -msse3 -mmmx -m3dnow -mcx16> ${CPP_WARNINGS})
target_compile_definitions(Mp3File PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>)
endif()

target_include_directories(Mp3File PRIVATE ${OPENGL_INCLUDE_DIRS})

if(WIN32)
  target_link_libraries(Mp3File PRIVATE TinyOAL)
else()
  target_link_libraries(Mp3File PRIVATE TinyOAL ${CMAKE_DL_LIBS})
endif()