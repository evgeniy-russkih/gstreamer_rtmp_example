find_package(PkgConfig)
find_package(spdlog CONFIG)

pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.4
        gstreamer-sdp-1.0>=1.4
        gstreamer-video-1.0>=1.4
        gstreamer-app-1.0>=1.4)

set(SOURCES
        RtmpRelay.cpp)

set(HEADERS
        RtmpRelay.h)

add_library(relay ${SOURCES} ${HEADERS})

target_include_directories(relay
        PUBLIC
        ${GST_INCLUDE_DIRS}
        )


target_link_libraries(relay
        PUBLIC
        spdlog::spdlog
        ${GST_LIBRARIES}
        )