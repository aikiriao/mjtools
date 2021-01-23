cmake_minimum_required(VERSION 3.1)

# プロジェクト名
project(mjplayer C)

# ライブラリ名
set(LIB_NAME mjplayer)

# 静的ライブラリ （あとで動的に変えるのもよし）
add_library(${LIB_NAME} STATIC)

# ソースディレクトリを指定
add_subdirectory(src)

# インクルードパス
target_include_directories(${LIB_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# コアライブラリのインクルードパスを加える
target_include_directories(${LIB_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
)

# コンパイルオプション
if(MSVC)
  target_compile_options(${LIB_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${LIB_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wformat=2 -Wstrict-aliasing=2 -Wconversion -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition)
  set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -DDEBUG")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()
set_target_properties(${LIB_NAME} PROPERTIES C_STANDARD 99 C_EXTENSIONS OFF)
