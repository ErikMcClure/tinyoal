cmake_minimum_required(VERSION 3.13.4)
project(FlacFile LANGUAGES C CXX VERSION 0.1.0)

file(GLOB_RECURSE FlacFile_SOURCES "./*.cpp")

if(MSVC)
  file(GLOB_RECURSE FlacFile_HEADERS "./*.h")
  add_executable(FlacFile ${FlacFile_SOURCES} ${FlacFile_HEADERS})
  target_link_options(FlacFile PRIVATE "$<$<CONFIG:Release>:/LTCG>")
else()
  add_executable(FlacFile ${FlacFile_SOURCES})
endif()

set_property(TARGET FlacFile PROPERTY C_STANDARD 17)
set_property(TARGET FlacFile PROPERTY CXX_STANDARD 20)
set_property(TARGET FlacFile PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET FlacFile PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET FlacFile PROPERTY VERBOSE_MAKEFILE TRUE)

retarget_output(FlacFile)
target_include_directories(FlacFile PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(FlacFile PUBLIC ${PROJECT_SOURCE_DIR})

if(MSVC)
  target_compile_options(FlacFile PRIVATE /Zc:preprocessor $<$<CONFIG:Release>:/Oi /Ot /GL> ${CPP_WARNINGS})
else()
  target_compile_options(FlacFile PRIVATE -pedantic -fno-exceptions -fno-rtti $<IF:$<CONFIG:Debug>,-g -msse -msse2 -O0,-O3 -msse -msse2 -msse3 -mmmx -m3dnow -mcx16> ${CPP_WARNINGS})
target_compile_definitions(FlacFile PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>)
endif()

target_include_directories(FlacFile PRIVATE ${OPENGL_INCLUDE_DIRS})

if(WIN32)
  target_link_libraries(FlacFile PRIVATE TinyOAL)
else()
  target_link_libraries(FlacFile PRIVATE TinyOAL ${CMAKE_DL_LIBS})
endif()