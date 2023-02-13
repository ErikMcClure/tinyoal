cmake_minimum_required(VERSION 3.13.4)
project(M4aFile LANGUAGES C CXX VERSION 0.1.0)

file(GLOB_RECURSE M4aFile_SOURCES "./*.cpp")

if(MSVC)
  file(GLOB_RECURSE M4aFile_HEADERS "./*.h")
  add_executable(M4aFile ${M4aFile_SOURCES} ${M4aFile_HEADERS})
  target_link_options(M4aFile PRIVATE "$<$<CONFIG:Release>:/LTCG>")
else()
  add_executable(M4aFile ${M4aFile_SOURCES})
endif()

set_property(TARGET M4aFile PROPERTY C_STANDARD 17)
set_property(TARGET M4aFile PROPERTY CXX_STANDARD 20)
set_property(TARGET M4aFile PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET M4aFile PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET M4aFile PROPERTY VERBOSE_MAKEFILE TRUE)

retarget_output(M4aFile)
target_include_directories(M4aFile PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(M4aFile PUBLIC ${PROJECT_SOURCE_DIR})

if(MSVC)
  target_compile_options(M4aFile PRIVATE /Zc:preprocessor $<$<CONFIG:Release>:/Oi /Ot /GL> ${CPP_WARNINGS})
else()
  target_compile_options(M4aFile PRIVATE -pedantic -fno-exceptions -fno-rtti $<IF:$<CONFIG:Debug>,-g -msse -msse2 -O0,-O3 -msse -msse2 -msse3 -mmmx -m3dnow -mcx16> ${CPP_WARNINGS})
target_compile_definitions(M4aFile PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>)
endif()

target_include_directories(M4aFile PRIVATE ${OPENGL_INCLUDE_DIRS})

if(WIN32)
  target_link_libraries(M4aFile PRIVATE TinyOAL)
else()
  target_link_libraries(M4aFile PRIVATE TinyOAL ${CMAKE_DL_LIBS})
endif()