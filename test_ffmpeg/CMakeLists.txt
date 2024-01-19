cmake_minimum_required (VERSION 3.20)

project(test_ffmpeg)

find_package(PkgConfig REQUIRED)

pkg_check_modules(avformat REQUIRED IMPORTED_TARGET libavformat)
pkg_check_modules(avcodec REQUIRED IMPORTED_TARGET libavcodec)
pkg_check_modules(avutil REQUIRED IMPORTED_TARGET libavutil)
pkg_check_modules(swscale REQUIRED IMPORTED_TARGET libswscale)
pkg_check_modules(swresample REQUIRED IMPORTED_TARGET libswresample)

add_executable(main main.cc)
target_link_libraries(main PkgConfig::avformat PkgConfig::avcodec PkgConfig::avutil PkgConfig::swresample PkgConfig::swscale)
