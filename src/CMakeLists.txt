cmake_minimum_required(VERSION 2.6)

PROJECT(left_hand_of_god)

ADD_DEFINITIONS(-O1 -Wall -g3)
ADD_DEFINITIONS(-Xlinker --gc-sections -Wl,-Map)

#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=gnu99")

#INCLUDE_DIRECTORIES(/usr/local/include)
#LINK_DIRECTORIES(/usr/local/lib)

#FIND_LIBRARY(json NAMES json-c json pthread)
FILE(GLOB SOURCES *.c)
#FILE(GLOB SOURCES *.cpp)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCES)

ADD_EXECUTABLE(lhog ${SOURCES})

TARGET_LINK_LIBRARIES(lhog ubox ubus blobmsg_json mosquitto pthread crypto cjson cjson_utils)

#install(TARGETS preiot RUNTIME DESTINATION bin)
