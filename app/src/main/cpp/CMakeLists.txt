cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -UNDEBUG")

add_library(native-codec-jni SHARED
            looper.cpp
            native-codec-jni.cpp
            player/IsmartvBase.c
            player/IsmartvPlayer.c
            player/IsmartvPlayerJni.c)

# Include libraries needed for native-codec-jni lib
target_link_libraries(native-codec-jni
                      android
                      log
                      mediandk
                      OpenMAXAL)

