#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

/* テスト対象のモジュール */
#include "../mj_effective_tile.c"

/* 1枚の牌情報 */
struct TileInfo {
  int32_t type;   /* 牌の種類(-1で終わり) */
  int32_t maisu;  /* 所持数               */
};

/* 有効牌テストケース */
struct MJEffectiveTileTestCase {
  struct TileInfo tiles[14];  /* 手牌情報（最大14エントリ） */
  struct MJEffectiveTiles answer; /* 正解 */
};

/* 有効牌取得関数型 */
typedef MJEffectiveTileApiResult (*effective_tiles_getter)(const struct MJHand *hand, struct MJEffectiveTiles *effective_tiles);

/* 牌の文字列変換テーブル */
static const char *tile_string_table[MJTILE_MAX] = {
  "invalid",
  "1M", "2M", "3M", "4M", "5M", "6M", "7M", "8M", "9M",
  "invalid",
  "1P", "2P", "3P", "4P", "5P", "6P", "7P", "8P", "9P",
  "invalid",
  "1S", "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S",
  "invalid",
  "TON", "NAN", "SHA", "PEE", "HAKU", "HATU", "CHUN",
};

/* テストのセットアップ関数 */
void MJEffectiveTileTest_Setup(void);

static int MJEffectiveTileTest_Initialize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

static int MJEffectiveTileTest_Finalize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

/* テストケースを手牌に変換 */
static void MJEffectiveTileTest_ConvertTileInfoToHand(const struct TileInfo *tile_info, struct MJHand *hand)
{
  int32_t i, j, pos;

  assert((tile_info != NULL) && (hand != NULL));

  /* 手牌を変換 */
  pos = 0;
  for (i = 0; i < 14; i++) {
    const struct TileInfo *pinfo = &tile_info[i];
    if (!MJTILE_IS_VALID(pinfo->type)) {
      break;
    }
    for (j = 0; j < pinfo->maisu; j++) {
      hand->hand[pos] = pinfo->type;
      pos++;
    }
  }

  /* 副露はなし */
  hand->meld[0].minhai  = MJTILE_INVALID;
  hand->meld[0].type    = MJMELD_TYPE_INVALID;
  hand->num_meld = 0;
}

/* 引数のテストケース配列に対してテスト 成功時は1, 失敗がある場合は0を返す */
static int MJEffectiveTiles_TestForTestCases(
    const struct MJEffectiveTileTestCase *test_cases, int32_t num_test_cases,
    effective_tiles_getter getter_function)
{
  int32_t i, is_ok;

  assert((test_cases != NULL) && (num_test_cases >= 0) && (getter_function != NULL));

  is_ok = 1;
  for (i = 0; i < num_test_cases; i++) {
    int32_t t;
    struct MJHand hand;
    struct MJEffectiveTiles get;
    MJEffectiveTileApiResult ret;
    const struct MJEffectiveTileTestCase *pcase = &test_cases[i];
    /* 牌出現カウントに変換 */
    MJEffectiveTileTest_ConvertTileInfoToHand(&(pcase->tiles[0]), &hand);
    /* 有効牌取得 */
    if ((ret = getter_function(&hand, &get)) != MJEFFECTIVETILE_APIRESULT_OK) {
      printf("NG at test case index:%d api result:%d \n", i, ret);
      is_ok = 0;
      break;
    }
    /* 回答と合っているかチェック */
    if (get.num_effective_tiles != pcase->answer.num_effective_tiles) {
      printf("NG at test case index:%d get.num:%d answer.num:%d \n", i,
          get.num_effective_tiles, pcase->answer.num_effective_tiles); 
      is_ok = 0;
      break;
    }
    for (t = 0; t < pcase->answer.num_effective_tiles; t++) {
      if (get.tiles[t] != pcase->answer.tiles[t]) {
        is_ok = 0;
        break;
      }
    }
    if (is_ok == 0) {
      printf("NG at test case index:%d \n", i); 
      printf("   Get:");
      for (t = 0; t < get.num_effective_tiles; t++) {
        printf("%s, ", tile_string_table[get.tiles[t]]);
      }
      printf("\nAnswer:");
      for (t = 0; t < pcase->answer.num_effective_tiles; t++) {
        printf("%s, ", tile_string_table[pcase->answer.tiles[t]]);
      }
      printf("\n");
      break;
    }
  }

  return is_ok;
}

/* 有効牌取得テスト */
static void MJEffectiveTileTest_GetNormalEffectiveTilesTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* 通常手 */
  {
    const struct MJEffectiveTileTestCase test_cases[] = {
      {
        {
          { MJTILE_3MAN, 1 }, { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 },
          { MJTILE_9MAN, 1 }, { MJTILE_2SOU, 1 }, { MJTILE_3SOU, 1 },
          { MJTILE_5SOU, 3 }, { MJTILE_6SOU, 1 }, { MJTILE_7SOU, 1 },
          { MJTILE_HATU, 2 }, { -1, 0 },
        },
        {
          { MJTILE_1SOU, MJTILE_4SOU, MJTILE_5SOU, MJTILE_8SOU, MJTILE_HATU, MJTILE_INVALID, },
          5,
        }
      },
      {
        {
          { MJTILE_1MAN, 1 }, { MJTILE_8MAN, 2 }, { MJTILE_3PIN, 2 },
          { MJTILE_4PIN, 1 }, { MJTILE_6PIN, 1 }, { MJTILE_7PIN, 1 },
          { MJTILE_8PIN, 1 }, { MJTILE_9PIN, 1 }, { MJTILE_1SOU, 2 },
          { MJTILE_3SOU, 1 }, { -1, 0 },
        },
        {
          { MJTILE_8MAN, MJTILE_3PIN, MJTILE_5PIN, MJTILE_1SOU, MJTILE_2SOU, MJTILE_INVALID, },
          5,
        }
      },
      {
        {
          { MJTILE_2MAN, 1 }, { MJTILE_4MAN, 1 }, { MJTILE_6MAN, 1 },
          { MJTILE_8MAN, 1 }, { MJTILE_2PIN, 2 }, { MJTILE_9PIN, 1 },
          { MJTILE_2SOU, 1 }, { MJTILE_3SOU, 1 }, { MJTILE_4SOU, 1 },
          { MJTILE_5SOU, 1 }, { MJTILE_7SOU, 1 }, { MJTILE_HAKU, 1 },
          { -1, 0 },
        },
        {
          { MJTILE_3MAN, MJTILE_7MAN, MJTILE_6SOU, MJTILE_INVALID, },
          3,
        }
      },
      {
        {
          { MJTILE_2MAN, 1 }, { MJTILE_4MAN, 1 }, { MJTILE_6MAN, 1 },
          { MJTILE_8MAN, 1 }, { MJTILE_2PIN, 2 }, { MJTILE_5PIN, 1 },
          { MJTILE_2SOU, 1 }, { MJTILE_3SOU, 1 }, { MJTILE_4SOU, 1 },
          { MJTILE_5SOU, 1 }, { MJTILE_7SOU, 1 }, { MJTILE_HAKU, 1 },
          { -1, 0 },
        },
        {
          { MJTILE_3MAN, MJTILE_7MAN, MJTILE_6SOU, MJTILE_INVALID, },
          3,
        }
      },
      {
        {
          { MJTILE_2MAN, 1 }, { MJTILE_4MAN, 1 }, { MJTILE_6MAN, 1 },
          { MJTILE_8MAN, 1 }, { MJTILE_2PIN, 2 }, { MJTILE_5PIN, 1 },
          { MJTILE_9PIN, 1 }, { MJTILE_2SOU, 1 }, { MJTILE_3SOU, 1 },
          { MJTILE_4SOU, 1 }, { MJTILE_5SOU, 1 }, { MJTILE_7SOU, 1 },
          { -1, 0 },
        },
        {
          { MJTILE_3MAN, MJTILE_7MAN, MJTILE_6SOU, MJTILE_INVALID, },
          3,
        }
      },
    };

    /* テストケースに対してテスト実行 */
    Test_AssertEqual(MJEffectiveTiles_TestForTestCases(
          test_cases, sizeof(test_cases) / sizeof(test_cases[0]), MJEffectiveTile_GetNormalEffectiveTiles), 1);
  }

}

/* 七対子手の有効牌取得テスト */
static void MJEffectiveTileTest_GetChitoitsuEffectiveTilesTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* テストケースに対して確認 */
  {
    const struct MJEffectiveTileTestCase test_cases[] = {
      {
        {
          { MJTILE_4MAN, 2 }, { MJTILE_1PIN, 1 }, { MJTILE_4PIN, 1 },
          { MJTILE_1SOU, 1 }, { MJTILE_3SOU, 2 }, { MJTILE_5SOU, 2 },
          { MJTILE_7SOU, 1 }, { MJTILE_8SOU, 1 }, { MJTILE_TON,  1 },
          { MJTILE_SHA,  1 }, { -1, 0 },
        },
        {
          { MJTILE_1PIN, MJTILE_4PIN, MJTILE_1SOU, MJTILE_7SOU, MJTILE_8SOU, MJTILE_TON, MJTILE_SHA, MJTILE_INVALID, },
          7,
        }
      },
      {
        {
          { MJTILE_3MAN, 2 }, { MJTILE_7MAN, 1 }, { MJTILE_9MAN, 2 },
          { MJTILE_3PIN, 1 }, { MJTILE_4PIN, 2 }, { MJTILE_1SOU, 1 },
          { MJTILE_2SOU, 2 }, { MJTILE_5SOU, 1 }, { MJTILE_SHA,  1 },
          { -1, 0 },
        },
        {
          { MJTILE_7MAN, MJTILE_3PIN, MJTILE_1SOU, MJTILE_5SOU, MJTILE_SHA, MJTILE_INVALID, },
          5,
        }
      },
      {
        {
          { MJTILE_2PIN, 1 }, { MJTILE_5PIN, 2 }, { MJTILE_6PIN, 2 },
          { MJTILE_9PIN, 1 }, { MJTILE_1SOU, 1 }, { MJTILE_4SOU, 1 },
          { MJTILE_8SOU, 1 }, { MJTILE_9SOU, 2 }, { MJTILE_SHA,  1 },
          { MJTILE_HAKU, 1 }, { -1, 0 },
        },
        {
          { MJTILE_2PIN, MJTILE_9PIN, MJTILE_1SOU, MJTILE_4SOU, MJTILE_8SOU, MJTILE_SHA, MJTILE_HAKU, MJTILE_INVALID, },
          7,
        }
      },
      {
        {
          { MJTILE_5PIN, 3 }, { MJTILE_6PIN, 2 }, { MJTILE_9PIN, 1 },
          { MJTILE_1SOU, 1 }, { MJTILE_4SOU, 1 }, { MJTILE_8SOU, 1 },
          { MJTILE_9SOU, 2 }, { MJTILE_SHA,  1 }, { MJTILE_HAKU, 1 },
          { -1, 0 },
        },
        {
          { MJTILE_9PIN, MJTILE_1SOU, MJTILE_4SOU, MJTILE_8SOU, MJTILE_SHA, MJTILE_HAKU, MJTILE_INVALID, },
          6,
        }
      },
    };

    /* テストケースに対してテスト実行 */
    Test_AssertEqual(MJEffectiveTiles_TestForTestCases(
          test_cases, sizeof(test_cases) / sizeof(test_cases[0]), MJEffectiveTile_GetChitoitsuEffectiveTiles), 1);
  }

}


/* 国士無双手の有効牌取得テスト */
static void MJEffectiveTileTest_GetKokushimusouEffectiveTilesTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* テストケースに対して確認 */
  {
    const struct MJEffectiveTileTestCase test_cases[] = {
      {
        /* 13面待ち */
        {
          { MJTILE_1MAN, 1 }, { MJTILE_9MAN, 1 }, { MJTILE_1PIN, 1 },
          { MJTILE_9PIN, 1 }, { MJTILE_1SOU, 1 }, { MJTILE_9SOU, 1 },
          { MJTILE_TON,  1 }, { MJTILE_NAN,  1 }, { MJTILE_SHA,  1 },
          { MJTILE_PEE,  1 }, { MJTILE_HAKU, 1 }, { MJTILE_HATU, 1 },
          { MJTILE_CHUN, 1 }, { -1, 0 },
        },
        {
          { MJTILE_1MAN, MJTILE_9MAN, MJTILE_1PIN, MJTILE_9PIN, MJTILE_1SOU,
            MJTILE_9SOU, MJTILE_TON,  MJTILE_NAN,  MJTILE_SHA,  MJTILE_PEE,
            MJTILE_HAKU, MJTILE_HATU, MJTILE_CHUN, MJTILE_INVALID, },
          13,
        }
      },
      {
        {
          { MJTILE_1MAN, 1 }, { MJTILE_9MAN, 1 }, { MJTILE_1PIN, 1 },
          { MJTILE_4PIN, 1 }, { MJTILE_8PIN, 2 }, { MJTILE_1SOU, 1 },
          { MJTILE_7SOU, 1 }, { MJTILE_8SOU, 1 }, { MJTILE_TON,  1 },
          { MJTILE_SHA,  1 }, { MJTILE_HAKU, 2 }, { -1, 0 },
        },
        {
          { MJTILE_9PIN, MJTILE_9SOU, MJTILE_NAN, MJTILE_PEE, MJTILE_HATU, MJTILE_CHUN, MJTILE_INVALID, },
          6,
        }
      },
    };

    /* テストケースに対してテスト実行 */
    Test_AssertEqual(MJEffectiveTiles_TestForTestCases(
          test_cases, sizeof(test_cases) / sizeof(test_cases[0]), MJEffectiveTile_GetKokushimusouEffectiveTiles), 1);
  }
}

void MJEffectiveTileTest_Setup(void)
{
  struct TestSuite *suite
    = Test_AddTestSuite("MJ Effective Tile Test Suite",
        NULL, MJEffectiveTileTest_Initialize, MJEffectiveTileTest_Finalize);

  Test_AddTest(suite, MJEffectiveTileTest_GetNormalEffectiveTilesTest);
  Test_AddTest(suite, MJEffectiveTileTest_GetChitoitsuEffectiveTilesTest);
  Test_AddTest(suite, MJEffectiveTileTest_GetKokushimusouEffectiveTilesTest);
}
