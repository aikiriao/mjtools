cmake_minimum_required(VERSION 3.1)

# プロジェクト名
project(make_shanten_table C)

# アプリケーション名
set(APP_NAME make_shanten_table)

# 実行ファイル
add_executable(${APP_NAME} make_shanten_table.c)

# コンパイルオプション
set(CMAKE_C_STANDARD 99)
if(MSVC)
  target_compile_options(${APP_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wformat=2 -Wstrict-aliasing=2 -Wconversion -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition)
  set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -DDEBUG")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# ビルド後に必要なファイルをコピー
add_custom_command(
  TARGET ${APP_NAME}
  POST_BUILD
  COMMAND
  ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/shanten.dat $<TARGET_FILE_DIR:${APP_NAME}>/shanten.dat)
