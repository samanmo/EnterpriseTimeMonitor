cmake_minimum_required(VERSION 3.16)
project(HRTimeTracker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Handle Qt meta-object compilers invisibly in backend build trees
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

include_directories(include)

# CRITICAL FIX: Explicitly discover headers alongside source definitions
# This ensures CMAKE_AUTOMOC intercepts Q_OBJECT inside TimeTracker.h
file(GLOB_RECURSE SOURCES 
    #"main.cpp"
    "src/*.cpp"
)

file(GLOB_RECURSE HEADERS
    "include/*.h"
)

# Pass both SOURCES and HEADERS to the executable target
add_executable(HRTimeTracker ${SOURCES} ${HEADERS})
target_link_libraries(HRTimeTracker PRIVATE Qt6::Widgets Qt6::Sql)

if(WIN32)
    set_target_properties(HRTimeTracker PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
