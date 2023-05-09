cmake_minimum_required(VERSION 3.18)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 20)

project(main LANGUAGES CXX)

add_executable(main
    main.cpp
    pbb/core.cpp
    psimd/transpose.cpp
    psimd/adjacent_difference.cpp
    contest/test.cpp
    )

find_package(Threads REQUIRED)
target_link_libraries(main PUBLIC Threads::Threads)

find_package(TBB REQUIRED)
target_link_libraries(main PUBLIC TBB::tbb)

find_package(OpenMP REQUIRED)
target_link_libraries(main PUBLIC OpenMP::OpenMP_CXX)

if (MSVC)
    target_compile_options(main PUBLIC /arch:AVX2)
else()
    target_compile_options(main PUBLIC -march=native)
endif()

find_package(benchmark REQUIRED)
target_link_libraries(main PUBLIC benchmark::benchmark)

find_package(GTest REQUIRED)
target_link_libraries(main PUBLIC GTest::gtest)
# target_link_libraries(main PUBLIC GTest::gtest_main)
