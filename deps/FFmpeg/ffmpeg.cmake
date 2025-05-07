if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
set(_conf_cmd ./configure)
set(_make_cmd make -j${NPROC})
set(_install_cmd make -j${NPROC} install)
ExternalProject_Add(dep_FFmpeg
    #EXCLUDE_FROM_ALL ON
    URL "https://ffmpeg.org/releases/ffmpeg-4.2.tar.bz2"
    URL_HASH SHA256=306bde5f411e9ee04352d1d3de41bd3de986e42e2af2a4c44052dce1ada26fb8
    DOWNLOAD_DIR ${DEP_DOWNLOAD_DIR}/FFmpeg
	CONFIGURE_COMMAND ${_conf_cmd}
        "--prefix=${DESTDIR}"
        "--enable-gpl"
        "--enable-libx264"
        "--enable-static"
        "--disable-x86asm"
    BUILD_IN_SOURCE ON
    BUILD_COMMAND ${_make_cmd}
    INSTALL_COMMAND ${_install_cmd}
)
endif()