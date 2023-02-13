cmake_minimum_required(VERSION 3.13.4)
project(PropTest LANGUAGES C CXX VERSION 0.1.0)

file(GLOB_RECURSE PropTest_SOURCES "./*.cpp")

if(MSVC)
  file(GLOB_RECURSE PropTest_HEADERS "./*.h")
  add_executable(PropTest ${PropTest_SOURCES} ${PropTest_HEADERS})
  target_link_options(PropTest PRIVATE "$<$<CONFIG:Release>:/LTCG>")
else()
  add_executable(PropTest ${PropTest_SOURCES})
endif()

set_property(TARGET PropTest PROPERTY C_STANDARD 17)
set_property(TARGET PropTest PROPERTY CXX_STANDARD 20)
set_property(TARGET PropTest PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET PropTest PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET PropTest PROPERTY VERBOSE_MAKEFILE TRUE)

retarget_output(PropTest)
target_include_directories(PropTest PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(PropTest PUBLIC ${PROJECT_SOURCE_DIR})

if(MSVC)
  target_compile_options(PropTest PRIVATE /Zc:preprocessor $<$<CONFIG:Release>:/Oi /Ot /GL> ${CPP_WARNINGS})
else()
  target_compile_options(PropTest PRIVATE -pedantic -fno-exceptions -fno-rtti $<IF:$<CONFIG:Debug>,-g -msse -msse2 -O0,-O3 -msse -msse2 -msse3 -mmmx -m3dnow -mcx16> ${CPP_WARNINGS})
target_compile_definitions(PropTest PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>)
endif()

target_include_directories(PropTest PRIVATE ${OPENGL_INCLUDE_DIRS})

if(WIN32)
  target_link_libraries(PropTest PRIVATE TinyOAL)
else()
  target_link_libraries(PropTest PRIVATE TinyOAL ${CMAKE_DL_LIBS})
endif()