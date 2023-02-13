cmake_minimum_required(VERSION 3.13.4)
project(TestBed LANGUAGES C CXX VERSION 0.1.0)

file(GLOB_RECURSE TestBed_SOURCES "./*.cpp")

if(MSVC)
  file(GLOB_RECURSE TestBed_HEADERS "./*.h")
  add_executable(TestBed ${TestBed_SOURCES} ${TestBed_HEADERS})
  target_link_options(TestBed PRIVATE "$<$<CONFIG:Release>:/LTCG>")
else()
  add_executable(TestBed ${TestBed_SOURCES})
endif()

set_property(TARGET TestBed PROPERTY C_STANDARD 17)
set_property(TARGET TestBed PROPERTY CXX_STANDARD 20)
set_property(TARGET TestBed PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET TestBed PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET TestBed PROPERTY VERBOSE_MAKEFILE TRUE)

retarget_output(TestBed)
target_include_directories(TestBed PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(TestBed PUBLIC ${PROJECT_SOURCE_DIR})

if(MSVC)
  target_compile_options(TestBed PRIVATE /Zc:preprocessor $<$<CONFIG:Release>:/Oi /Ot /GL> ${CPP_WARNINGS})
else()
  target_compile_options(TestBed PRIVATE -pedantic -fno-exceptions -fno-rtti $<IF:$<CONFIG:Debug>,-g -msse -msse2 -O0,-O3 -msse -msse2 -msse3 -mmmx -m3dnow -mcx16> ${CPP_WARNINGS})
target_compile_definitions(TestBed PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>)
endif()

target_include_directories(TestBed PRIVATE ${OPENGL_INCLUDE_DIRS})

if(WIN32)
  target_link_libraries(TestBed PRIVATE TinyOAL)
else()
  target_link_libraries(TestBed PRIVATE TinyOAL ${CMAKE_DL_LIBS})
endif()