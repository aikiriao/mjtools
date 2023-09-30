cmake_minimum_required(VERSION 3.1)

# プロジェクト名
project(sample_simulator C)

# アプリケーション名
set(APP_NAME sample_simulator)

# 必要なパッケージ指定
find_package(Curses REQUIRED)

# 依存するサブディレクトリを追加
set(PROJECT_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
add_subdirectory(${PROJECT_ROOT_PATH}/core ${CMAKE_CURRENT_BINARY_DIR}/core)
add_subdirectory(${PROJECT_ROOT_PATH}/player ${CMAKE_CURRENT_BINARY_DIR}/player)
add_subdirectory(${PROJECT_ROOT_PATH}/simulator ${CMAKE_CURRENT_BINARY_DIR}/simulator)

# 実行ファイル
add_executable(${APP_NAME} sample_simulator.c)

# リンクするライブラリ
target_link_libraries(${APP_NAME} mjcore)
target_link_libraries(${APP_NAME} mjplayer)
target_link_libraries(${APP_NAME} mjsimulator)

# リンクする外部ライブラリ
target_link_libraries(${APP_NAME} m) # math
target_link_libraries(${APP_NAME} ${CURSES_LIBRARIES})

# コンパイルオプション
if(MSVC)
  target_compile_options(${APP_NAME} PRIVATE /W4)
else()
  target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wformat=2 -Wstrict-aliasing=2 -Wconversion -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition)
  set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -DDEBUG")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()
set_target_properties(${LIB_NAME} PROPERTIES C_STANDARD 99 C_EXTENSIONS OFF)
