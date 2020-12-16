#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

/* テスト対象のモジュール */
#include "../mj_river.c"

/* テストのセットアップ関数 */
void MJRiverTest_Setup(void);

static int MJRiverTest_Initialize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

static int MJRiverTest_Finalize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

/* 牌を捨てるテスト */
static void MJRiverTest_DiscardTileTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* 基本ケース */
  {
    struct MJRiver river;

    /* 初期化 */
    MJRiver_Initialize(&river);
    Test_AssertEqual(river.player_river[MJWIND_TON].num_tiles, 0);
    Test_AssertEqual(river.player_river[MJWIND_NAN].num_tiles, 0);
    Test_AssertEqual(river.player_river[MJWIND_SHA].num_tiles, 0);
    Test_AssertEqual(river.player_river[MJWIND_PEE].num_tiles, 0);

    /* 東家が捨てる */
    Test_AssertEqual(MJRiver_DiscardTile(&river, MJWIND_TON, MJTILE_1MAN, false), MJRIVER_APIRESULT_OK);

    /* 捨てた内容が反映されているか */
    Test_AssertEqual(river.player_river[MJWIND_TON].tiles[0].tile, MJTILE_1MAN);
    Test_AssertEqual(river.last_discard_tile, MJTILE_1MAN);
    Test_AssertEqual(river.last_discard_player, MJWIND_TON);

    /* 河の枚数チェック */
    Test_AssertEqual(river.player_river[MJWIND_TON].num_tiles, 1);
    Test_AssertEqual(river.player_river[MJWIND_NAN].num_tiles, 0);
    Test_AssertEqual(river.player_river[MJWIND_SHA].num_tiles, 0);
    Test_AssertEqual(river.player_river[MJWIND_PEE].num_tiles, 0);

    /* 南家が捨てる */
    Test_AssertEqual(MJRiver_DiscardTile(&river, MJWIND_NAN, MJTILE_2MAN, false), MJRIVER_APIRESULT_OK);

    /* 捨てた内容が反映されているか */
    Test_AssertEqual(river.player_river[MJWIND_NAN].tiles[0].tile, MJTILE_2MAN);
    Test_AssertEqual(river.last_discard_tile, MJTILE_2MAN);
    Test_AssertEqual(river.last_discard_player, MJWIND_NAN);

    /* 河の枚数チェック */
    Test_AssertEqual(river.player_river[MJWIND_TON].num_tiles, 1);
    Test_AssertEqual(river.player_river[MJWIND_NAN].num_tiles, 1);
    Test_AssertEqual(river.player_river[MJWIND_SHA].num_tiles, 0);
    Test_AssertEqual(river.player_river[MJWIND_PEE].num_tiles, 0);

    /* 西家が捨てる */
    Test_AssertEqual(MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_3MAN, false), MJRIVER_APIRESULT_OK);

    /* 捨てた内容が反映されているか */
    Test_AssertEqual(river.player_river[MJWIND_SHA].tiles[0].tile, MJTILE_3MAN);
    Test_AssertEqual(river.last_discard_tile, MJTILE_3MAN);
    Test_AssertEqual(river.last_discard_player, MJWIND_SHA);

    /* 河の枚数チェック */
    Test_AssertEqual(river.player_river[MJWIND_TON].num_tiles, 1);
    Test_AssertEqual(river.player_river[MJWIND_NAN].num_tiles, 1);
    Test_AssertEqual(river.player_river[MJWIND_SHA].num_tiles, 1);
    Test_AssertEqual(river.player_river[MJWIND_PEE].num_tiles, 0);

    /* 北家が捨てる */
    Test_AssertEqual(MJRiver_DiscardTile(&river, MJWIND_PEE, MJTILE_4MAN, false), MJRIVER_APIRESULT_OK);

    /* 捨てた内容が反映されているか */
    Test_AssertEqual(river.player_river[MJWIND_PEE].tiles[0].tile, MJTILE_4MAN);
    Test_AssertEqual(river.last_discard_tile, MJTILE_4MAN);
    Test_AssertEqual(river.last_discard_player, MJWIND_PEE);

    /* 河の枚数チェック */
    Test_AssertEqual(river.player_river[MJWIND_TON].num_tiles, 1);
    Test_AssertEqual(river.player_river[MJWIND_NAN].num_tiles, 1);
    Test_AssertEqual(river.player_river[MJWIND_SHA].num_tiles, 1);
    Test_AssertEqual(river.player_river[MJWIND_PEE].num_tiles, 1);
  }

  /* 捨てながら立直したケース */
  {
    struct MJRiver river;

    /* 初期化 */
    MJRiver_Initialize(&river);

    /* 東家が立直しながら捨てる */
    Test_AssertEqual(MJRiver_DiscardTile(&river, MJWIND_TON, MJTILE_1MAN, true), MJRIVER_APIRESULT_OK);

    /* 捨てた内容が反映されているか */
    Test_AssertEqual(river.player_river[MJWIND_TON].tiles[0].tile, MJTILE_1MAN);
    Test_AssertEqual(river.player_river[MJWIND_TON].tiles[0].riichi, true);
    Test_AssertEqual(river.last_discard_tile, MJTILE_1MAN);
    Test_AssertEqual(river.last_discard_player, MJWIND_TON);

    /* もう一回立直しながら捨てる（エラー発生を期待） */
    Test_AssertEqual(MJRiver_DiscardTile(&river, MJWIND_TON, MJTILE_1MAN, true), MJRIVER_APIRESULT_ALREADY_RIICHI);
    Test_AssertEqual(river.player_river[MJWIND_TON].num_tiles, 1);
  }

  /* 失敗ケース */
  {
    struct MJRiver river;

    /* 初期化 */
    MJRiver_Initialize(&river);

    /* 不正な引数を渡す */
    Test_AssertEqual(MJRiver_DiscardTile(NULL, MJWIND_TON, MJTILE_1MAN, false), MJRIVER_APIRESULT_INVALID_ARGUMENT);
    Test_AssertEqual(MJRiver_DiscardTile(&river, MJWIND_INVALID, MJTILE_1MAN, false), MJRIVER_APIRESULT_INVALID_ARGUMENT);
    Test_AssertEqual(MJRiver_DiscardTile(&river, MJWIND_TON, MJTILE_INVALID, false), MJRIVER_APIRESULT_INVALID_ARGUMENT);
  }

}

/* ポンテスト */
static void MJRiverTest_PungTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* ポンしてみる */
  {
    struct MJRiver river, river_backup;
    struct MJHand hand = { { 0, }, { 0, }, 0 };

    MJRiver_Initialize(&river);

    /* 一萬を2枚持っている */
    hand.hand[0] = MJTILE_1MAN;
    hand.hand[1] = MJTILE_1MAN;
    /* 一萬が捨てられた */
    MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_1MAN, false);

    /* 各家が鳴こうとする */
    Test_AssertEqual(MJRiver_CanPung(&river, MJWIND_TON, &hand), true);
    Test_AssertEqual(MJRiver_CanPung(&river, MJWIND_NAN, &hand), true);
    Test_AssertEqual(MJRiver_CanPung(&river, MJWIND_SHA, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanPung(&river, MJWIND_PEE, &hand), true);

    river_backup = river;

    /* 実際に鳴いてみる */
    river = river_backup;
    Test_AssertEqual(MJRiver_Pung(&river, MJWIND_TON, &hand), MJRIVER_APIRESULT_OK);
    Test_AssertEqual(river.player_river[MJWIND_SHA].tiles[0].meld_player, MJWIND_TON);
    river = river_backup;
    Test_AssertEqual(MJRiver_Pung(&river, MJWIND_NAN, &hand), MJRIVER_APIRESULT_OK);
    Test_AssertEqual(river.player_river[MJWIND_SHA].tiles[0].meld_player, MJWIND_NAN);
    river = river_backup;
    Test_AssertEqual(MJRiver_Pung(&river, MJWIND_SHA, &hand), MJRIVER_APIRESULT_CANNOT_MELD);  /* 捨てた本人は鳴けない */
    river = river_backup;
    Test_AssertEqual(MJRiver_Pung(&river, MJWIND_PEE, &hand), MJRIVER_APIRESULT_OK);
    Test_AssertEqual(river.player_river[MJWIND_SHA].tiles[0].meld_player, MJWIND_PEE);
  }

  /* ポンAPIに不正な引数を渡す */
  {
    struct MJRiver river;
    struct MJHand hand = { { 0, }, { 0, }, 0 };

    MJRiver_Initialize(&river);

    /* 一萬を2枚持っている */
    hand.hand[0] = MJTILE_1MAN;
    hand.hand[1] = MJTILE_1MAN;
    /* 一萬が捨てられた */
    MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_1MAN, false);

    /* 不正な引数を渡す */
    Test_AssertEqual(MJRiver_Pung(NULL, MJWIND_TON, &hand), MJRIVER_APIRESULT_INVALID_ARGUMENT);
    Test_AssertEqual(MJRiver_Pung(&river, MJWIND_INVALID, &hand), MJRIVER_APIRESULT_INVALID_ARGUMENT);
    Test_AssertEqual(MJRiver_Pung(&river, MJWIND_TON, NULL), MJRIVER_APIRESULT_INVALID_ARGUMENT);
  }

}

/* チーテスト */
static void MJRiverTest_ChowTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* チーしてみる */
  {
    struct MJRiver river;
    struct MJHand hand = { { 0, }, { 0, }, 0 };

    /* 辺張ケース1 */
    hand.hand[0] = MJTILE_1MAN;
    hand.hand[1] = MJTILE_2MAN;
    MJRiver_Initialize(&river);
    MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_3MAN, false);
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_TON, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_NAN, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_SHA, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_PEE, &hand), true);
    /* このケースのみ他の家が捨てたケースも調べておく */
    MJRiver_Initialize(&river);
    MJRiver_DiscardTile(&river, MJWIND_TON, MJTILE_3MAN, false);
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_TON, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_NAN, &hand), true);
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_SHA, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_PEE, &hand), false);  /* 上家ではない */
    MJRiver_Initialize(&river);
    MJRiver_DiscardTile(&river, MJWIND_NAN, MJTILE_3MAN, false);
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_TON, &hand), false);  /* 上家ではない */  
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_NAN, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_SHA, &hand), true);
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_PEE, &hand), false);  /* 上家ではない */
    MJRiver_Initialize(&river);
    MJRiver_DiscardTile(&river, MJWIND_PEE, MJTILE_3MAN, false);
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_TON, &hand), true);  
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_NAN, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_SHA, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_PEE, &hand), false);  /* 上家ではない */

    /* 辺張ケース2 */
    hand.hand[0] = MJTILE_8MAN;
    hand.hand[1] = MJTILE_9MAN;
    MJRiver_Initialize(&river);
    MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_7MAN, false);
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_TON, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_NAN, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_SHA, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_PEE, &hand), true);

    /* 嵌張ケース1 */
    hand.hand[0] = MJTILE_1MAN;
    hand.hand[1] = MJTILE_3MAN;
    MJRiver_Initialize(&river);
    MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_2MAN, false);
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_TON, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_NAN, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_SHA, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_PEE, &hand), true);

    /* 嵌張ケース2 */
    hand.hand[0] = MJTILE_2MAN;
    hand.hand[1] = MJTILE_4MAN;
    MJRiver_Initialize(&river);
    MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_3MAN, false);
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_TON, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_NAN, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_SHA, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_PEE, &hand), true);

    /* 両面ケース1 */
    hand.hand[0] = MJTILE_2MAN;
    hand.hand[1] = MJTILE_3MAN;
    MJRiver_Initialize(&river);
    MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_1MAN, false);
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_TON, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_NAN, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_SHA, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_PEE, &hand), true);

    /* 両面ケース2 */
    hand.hand[0] = MJTILE_2MAN;
    hand.hand[1] = MJTILE_3MAN;
    MJRiver_Initialize(&river);
    MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_4MAN, false);
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_TON, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_NAN, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_SHA, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_PEE, &hand), true);

    /* 両面ケース3 */
    hand.hand[0] = MJTILE_7MAN;
    hand.hand[1] = MJTILE_8MAN;
    MJRiver_Initialize(&river);
    MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_6MAN, false);
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_TON, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_NAN, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_SHA, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_PEE, &hand), true);

    /* 両面ケース4 */
    hand.hand[0] = MJTILE_7MAN;
    hand.hand[1] = MJTILE_8MAN;
    MJRiver_Initialize(&river);
    MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_9MAN, false);
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_TON, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_NAN, &hand), false);  /* 上家ではない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_SHA, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanChow(&river, MJWIND_PEE, &hand), true);
  }

  /* チーAPIに不正な引数を渡す */
  {
    struct MJRiver river;
    struct MJHand hand = { { 0, }, { 0, }, 0 };

    hand.hand[0] = MJTILE_1MAN;
    hand.hand[1] = MJTILE_2MAN;
    MJRiver_Initialize(&river);
    MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_3MAN, false);

    /* 不正な引数を渡す */
    Test_AssertEqual(MJRiver_Chow(NULL, MJWIND_TON, &hand), MJRIVER_APIRESULT_INVALID_ARGUMENT);
    Test_AssertEqual(MJRiver_Chow(&river, MJWIND_INVALID, &hand), MJRIVER_APIRESULT_INVALID_ARGUMENT);
    Test_AssertEqual(MJRiver_Chow(&river, MJWIND_TON, NULL), MJRIVER_APIRESULT_INVALID_ARGUMENT);
  }
}

/* カンテスト */
static void MJRiverTest_KanTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* カンしてみる */
  {
    struct MJRiver river, river_backup;
    struct MJHand hand = { { 0, }, { 0, }, 0 };

    MJRiver_Initialize(&river);

    /* 中を3枚持っている */
    hand.hand[0] = MJTILE_CHUN;
    hand.hand[1] = MJTILE_CHUN;
    hand.hand[2] = MJTILE_CHUN;
    /* 中が捨てられた */
    MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_CHUN, false);

    /* 各家が鳴こうとする */
    Test_AssertEqual(MJRiver_CanKan(&river, MJWIND_TON, &hand), true);
    Test_AssertEqual(MJRiver_CanKan(&river, MJWIND_NAN, &hand), true);
    Test_AssertEqual(MJRiver_CanKan(&river, MJWIND_SHA, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanKan(&river, MJWIND_PEE, &hand), true);
    /* ポンもできる */
    Test_AssertEqual(MJRiver_CanPung(&river, MJWIND_TON, &hand), true);
    Test_AssertEqual(MJRiver_CanPung(&river, MJWIND_NAN, &hand), true);
    Test_AssertEqual(MJRiver_CanPung(&river, MJWIND_SHA, &hand), false);  /* 捨てた本人は鳴けない */
    Test_AssertEqual(MJRiver_CanPung(&river, MJWIND_PEE, &hand), true);

    river_backup = river;

    /* 実際に鳴いてみる */
    river = river_backup;
    Test_AssertEqual(MJRiver_Kan(&river, MJWIND_TON, &hand), MJRIVER_APIRESULT_OK);
    Test_AssertEqual(river.player_river[MJWIND_SHA].tiles[0].meld_player, MJWIND_TON);
    river = river_backup;
    Test_AssertEqual(MJRiver_Kan(&river, MJWIND_NAN, &hand), MJRIVER_APIRESULT_OK);
    Test_AssertEqual(river.player_river[MJWIND_SHA].tiles[0].meld_player, MJWIND_NAN);
    river = river_backup;
    Test_AssertEqual(MJRiver_Kan(&river, MJWIND_SHA, &hand), MJRIVER_APIRESULT_CANNOT_MELD);  /* 捨てた本人は鳴けない */
    river = river_backup;
    Test_AssertEqual(MJRiver_Kan(&river, MJWIND_PEE, &hand), MJRIVER_APIRESULT_OK);
    Test_AssertEqual(river.player_river[MJWIND_SHA].tiles[0].meld_player, MJWIND_PEE);
  }

  /* カンAPIに不正な引数を渡す */
  {
    struct MJRiver river;
    struct MJHand hand = { { 0, }, { 0, }, 0 };

    MJRiver_Initialize(&river);

    /* 中を3枚持っている */
    hand.hand[0] = MJTILE_CHUN;
    hand.hand[1] = MJTILE_CHUN;
    hand.hand[2] = MJTILE_CHUN;
    /* 中が捨てられた */
    MJRiver_DiscardTile(&river, MJWIND_SHA, MJTILE_CHUN, false);

    /* 不正な引数を渡す */
    Test_AssertEqual(MJRiver_Kan(NULL, MJWIND_TON, &hand), MJRIVER_APIRESULT_INVALID_ARGUMENT);
    Test_AssertEqual(MJRiver_Kan(&river, MJWIND_INVALID, &hand), MJRIVER_APIRESULT_INVALID_ARGUMENT);
    Test_AssertEqual(MJRiver_Kan(&river, MJWIND_TON, NULL), MJRIVER_APIRESULT_INVALID_ARGUMENT);
  }
}
 
/* テストセットアップ */
void MJRiverTest_Setup(void)
{
  struct TestSuite *suite
    = Test_AddTestSuite("MJ River Test Suite",
        NULL, MJRiverTest_Initialize, MJRiverTest_Finalize);

  Test_AddTest(suite, MJRiverTest_DiscardTileTest);
  Test_AddTest(suite, MJRiverTest_PungTest);
  Test_AddTest(suite, MJRiverTest_ChowTest);
  Test_AddTest(suite, MJRiverTest_KanTest);
}

