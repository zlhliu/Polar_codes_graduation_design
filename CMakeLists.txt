cmake_minimum_required(VERSION 3.0.0)
project(polar_codes VERSION 0.1.0)

file (GLOB LIB_SOURCES "*.cpp")

# 添加一个可编译的目标到工程
add_executable(polar_code polar_code.cpp lib/encoder.cpp lib/decoder.cpp lib/demodulation.cpp)
<<<<<<< HEAD

=======
>>>>>>> 936f85f (累了，先这样吧)
# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)