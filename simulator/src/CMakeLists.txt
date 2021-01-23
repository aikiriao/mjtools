target_sources(${LIB_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mj_deck.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mj_river.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mj_random_xoshiro256pp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mj_player_wrapper.c
)
