cmake_minimum_required(VERSION 3.13)
project(webrtc_video)

include(PrecompiledHeader)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
set(VIDEO_SOURCES
    WebRTCDecoder.h
    WebRTCDecoder.cpp
    YangPlayerBase.cpp
    YangPlayerDecoder.cpp
    YangPlayerHandleImpl.cpp
    YangPlayerHandleImpl.h
    YangPlayerPlay.cpp
    YangRtcReceive.cpp
    YangRtcReceive.h
)
set(VIDEO_INCLUDE_DIRS
    ${CMAKE_PREFIX_PATH}/include/metartc/
    ${CMAKE_PREFIX_PATH}/include/
)
add_library(webrtc_video STATIC ${VIDEO_SOURCES})
target_include_directories(webrtc_video
    PUBLIC
        ${VIDEO_INCLUDE_DIRS}
)


    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

    #find_package(OpenSSL REQUIRED)
    find_library(LIBMETARTC metartc6)
    find_library(LIBMETARTCCORE metartccore6)
    find_package(FFmpeg)
    target_link_libraries(webrtc_video INTERFACE ${OPENSSL_LIBRARIES} boost_libs ${LIBMETARTC} ${LIBMETARTCCORE} usrsctp srtp2 asound FFmpeg::avcodec FFmpeg::avutil FFmpeg::avformat FFmpeg::swresample opus speexdsp lzma x264)
endif()
