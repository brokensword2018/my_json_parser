PROJECT(my_json_parser)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g3 -ggdb")


set(LIB_SRC JSON.cpp parser.cpp print.cpp)
add_library(myjson ${LIB_SRC})

set(SRC test.cpp)

add_executable(test ${SRC})
target_link_libraries(test myjson)


