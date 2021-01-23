cmake_minimum_required(VERSION 3.1)

# プロジェクト名
project(test C)

# ライブラリ名
set(LIB_NAME test)

# 静的ライブラリ指定
add_library(${LIB_NAME} STATIC)

# ソース
target_sources(${LIB_NAME} PRIVATE test.c)

# インクルードパス
target_include_directories(${LIB_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# コンパイルオプション
set(CMAKE_C_STANDARD 99)
if(MSVC)
  target_compile_options(${LIB_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${LIB_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wformat=2 -Wstrict-aliasing=2 -Wconversion -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition)
  set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -DDEBUG")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()
