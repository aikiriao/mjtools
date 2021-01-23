cmake_minimum_required(VERSION 3.1)

# プロジェクト名
project(mjlibtest C)

# アプリケーション名
set(APP_NAME mjlibtest)

# 外部ライブラリ
find_package(curses REQUIRED)

# 実行ファイル
add_executable(${APP_NAME} test_main.c)

# インクルードパス
target_include_directories(${APP_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../player/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../simulator/include
)

# 依存するサブディレクトリを追加
add_subdirectory(libtest)
add_subdirectory(test_src)

# リンクする内部ライブラリ
target_link_libraries(${APP_NAME} test)

# リンクする外部ライブラリ
target_link_libraries(${APP_NAME} curses)

# コンパイルオプション
set(CMAKE_C_STANDARD 99)
if(MSVC)
  target_compile_options(${APP_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wformat=2 -Wstrict-aliasing=2 -Wconversion -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition)
  set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -DDEBUG")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# ビルド後にテストリソースを持ってくる
add_custom_command(
  TARGET ${APP_NAME}
  POST_BUILD
  COMMAND
  ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test_src/problems $<TARGET_FILE_DIR:${APP_NAME}>/problems)
