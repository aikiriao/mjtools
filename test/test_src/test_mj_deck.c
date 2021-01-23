#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

/* テスト対象のモジュール */
#include "../../simulator/src/mj_deck.c"

/* テストのセットアップ関数 */
void MJDeckTest_Setup(void);

static int MJDeckTest_Initialize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

static int MJDeckTest_Finalize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

/* 生成破棄テスト */
static void MJDeckTest_CreateDestroyTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* ワークサイズ計算テスト */
  {
    struct MJDeckConfig config;

    /* NULL指定 */
    Test_AssertCondition(MJDeck_CalculateWorkSize(NULL) > 0);

    /* インターフェース設定 */
    MJDeck_SetDefaultConfig(&config);
    config.random_if = MJRandomXoshiro256pp_GetInterface();
    Test_AssertCondition(MJDeck_CalculateWorkSize(&config) > 0);
  }

  /* ワーク領域渡しによる生成・破棄（コンフィグNULL指定） */
  {
    int32_t work_size;
    void *work;
    struct MJDeck *deck = NULL;

    work_size = MJDeck_CalculateWorkSize(NULL);
    Test_AssertCondition(work_size > 0);

    work = malloc(work_size);
    deck = MJDeck_Create(NULL, work, work_size);
    Test_AssertCondition(deck != NULL);
    Test_AssertEqual(deck->alloced_by_own, false);
    Test_AssertCondition(deck->work == work);
    Test_AssertCondition(deck->random != NULL);
    Test_AssertEqual(deck->shufffled, false);

    /* デフォルトでXoshiro256++がセットされるはず */
    Test_AssertCondition(deck->random_if == MJRandomXoshiro256pp_GetInterface());

    MJDeck_Destroy(deck);
    free(work);
  }

  /* ワーク領域渡しによる生成・破棄（コンフィグ指定） */
  {
    int32_t work_size;
    void *work;
    struct MJDeck *deck = NULL;
    struct MJDeckConfig config;

    /* デフォルトのコンフィグを一旦セット */
    MJDeck_SetDefaultConfig(&config);

    /* インターフェース設定 */
    config.random_if = MJRandomXoshiro256pp_GetInterface();

    work_size = MJDeck_CalculateWorkSize(&config);
    Test_AssertCondition(work_size > 0);

    work = malloc(work_size);
    deck = MJDeck_Create(&config, work, work_size);
    Test_AssertCondition(deck != NULL);
    Test_AssertEqual(deck->alloced_by_own, false);
    Test_AssertCondition(deck->work == work);
    Test_AssertCondition(deck->random != NULL);
    Test_AssertEqual(deck->shufffled, false);
    Test_AssertCondition(deck->random_if == config.random_if);

    MJDeck_Destroy(deck);
    free(work);
  }

  /* ワーク領域自前確保による生成・破棄（コンフィグNULL指定） */
  {
    struct MJDeck *deck = NULL;

    deck = MJDeck_Create(NULL, NULL, 0);
    Test_AssertCondition(deck != NULL);
    Test_AssertEqual(deck->alloced_by_own, true);
    Test_AssertCondition(deck->work != NULL);
    Test_AssertCondition(deck->random != NULL);
    Test_AssertEqual(deck->shufffled, false);
    Test_AssertCondition(deck->random_if == MJRandomXoshiro256pp_GetInterface());

    MJDeck_Destroy(deck);
  }

  /* ワーク領域自前確保による生成・破棄（コンフィグ指定） */
  {
    struct MJDeck *deck = NULL;
    struct MJDeckConfig config;

    /* デフォルトのコンフィグを一旦セット */
    MJDeck_SetDefaultConfig(&config);

    /* インターフェース設定 */
    config.random_if = MJRandomXoshiro256pp_GetInterface();

    deck = MJDeck_Create(&config, NULL, 0);
    Test_AssertCondition(deck != NULL);
    Test_AssertEqual(deck->alloced_by_own, true);
    Test_AssertCondition(deck->work != NULL);
    Test_AssertCondition(deck->random != NULL);
    Test_AssertEqual(deck->shufffled, false);
    Test_AssertCondition(deck->random_if == config.random_if);

    MJDeck_Destroy(deck);
  }

  /* ワークサイズ計算失敗ケース */
  {
    void *work;
    int32_t work_size;
    struct MJDeckConfig config;

    MJDeck_SetDefaultConfig(&config);

    /* 有効なワークサイズと領域を確保 */
    work_size = MJDeck_CalculateWorkSize(&config);
    work = malloc(work_size);

    /* イカれた赤ドラ枚数設定: 負値 */
    config.akadora_count[MJTILE_1MAN] = -1;
    /* 失敗を確認 */
    Test_AssertCondition(MJDeck_CalculateWorkSize(&config) < 0);
    Test_AssertCondition(MJDeck_Create(&config, NULL, 0) == NULL);
    Test_AssertCondition(MJDeck_Create(&config, work, work_size) == NULL);

    /* イカれた赤ドラ枚数設定: 5枚以上 */
    config.akadora_count[MJTILE_1MAN] = 5;
    /* 失敗を確認 */
    Test_AssertCondition(MJDeck_CalculateWorkSize(&config) < 0);
    Test_AssertCondition(MJDeck_Create(&config, NULL, 0) == NULL);
    Test_AssertCondition(MJDeck_Create(&config, work, work_size) == NULL);

    free(work);
  }

}

/* 牌の比較関数 */
static int compare_tile(const void *a, const void *b)
{
  const int32_t ia = (int32_t)(*(MJTile *)a);
  const int32_t ib = (int32_t)(*(MJTile *)b);
  return ia - ib;
}

/* シャッフルテスト */
static void MJDeckTest_ShuffleTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  {
    struct MJDeck *deck;

    deck = MJDeck_Create(NULL, NULL, 0);

    /* とりあえずシャッフルしてみる */
    Test_AssertEqual(MJDeck_Shuffle(deck), MJDECK_APIRESULT_OK);

    /* シャッフル後の状態チェック */
    Test_AssertEqual(deck->shufffled, true);
    Test_AssertEqual(deck->tsumo_pos, 0);
    Test_AssertEqual(deck->rinshan_pos, 0);
    Test_AssertEqual(deck->dora.num_dora, 1);

    /* もう一度シャッフルしても壊れることはないか？ */
    Test_AssertEqual(MJDeck_Shuffle(deck), MJDECK_APIRESULT_OK);
    Test_AssertEqual(deck->shufffled, true);
    Test_AssertEqual(deck->tsumo_pos, 0);
    Test_AssertEqual(deck->rinshan_pos, 0);
    Test_AssertEqual(deck->dora.num_dora, 1);

    MJDeck_Destroy(deck);
  }

  /* シャッフルで消失したり増えた牌がないかチェック */
  {
    int32_t i, kyoku, is_ok;
    int32_t tile_count[MJTILE_MAX];
    struct MJDeck *deck;

    deck = MJDeck_Create(NULL, NULL, 0);

    /* 半荘戦(8局)を想定して8回繰り返す */
    is_ok = 1;
    for (kyoku = 0; kyoku < 8; kyoku++) {
      /* シャッフルしてみる */
      if (MJDeck_Shuffle(deck) != MJDECK_APIRESULT_OK) {
        is_ok = 0;
        goto CHECK_END;
      }

      /* 牌数カウントをリセット */
      for (i = 0; i < MJTILE_MAX; i++) {
        tile_count[i] = 0;
      }
      /* 牌山 */
      for (i = 0; i < 122; i++) {
        tile_count[deck->deck[i]]++;
      }
      /* 嶺上牌 */
      for (i = 0; i < 4; i++) {
        tile_count[deck->rinshan[i]]++;
      }
      /* ドラ牌 */
      for (i = 0; i < 5; i++) {
        tile_count[deck->dora.omote[i]]++;
        tile_count[deck->dora.ura[i]]++;
      }

      /* 全ての牌が4枚ずつ存在しなければならない */
      for (i = 0; i < MJTILE_MAX; i++) {
        if (MJTILE_IS_SUHAI(i) || MJTILE_IS_JIHAI(i)) {
          if (tile_count[i] != 4) {
            printf("%d %d \n", i, tile_count[i]);
            is_ok = 0;
            goto CHECK_END;
          }
        }
      }

    }

CHECK_END:
    Test_AssertEqual(is_ok, 1);
    MJDeck_Destroy(deck);
  }

  /* 生成直後にシャッフルしても、並びが変わってほしい */
  {
    MJTile shufffled_deck[MJDECK_NUM_TILES];
    struct MJDeck *deck;

    deck = MJDeck_Create(NULL, NULL, 0);
    MJDeck_Shuffle(deck);
    memcpy(&shufffled_deck[0],   deck->deck,        sizeof(MJTile) * 122);
    memcpy(&shufffled_deck[122], deck->rinshan,     sizeof(MJTile) *   4);
    memcpy(&shufffled_deck[126], deck->dora.omote,  sizeof(MJTile) *   5);
    memcpy(&shufffled_deck[131], deck->dora.ura,    sizeof(MJTile) *   5);
    MJDeck_Destroy(deck);

    Test_AssertNotEqual(memcmp(shufffled_deck, initial_deck, sizeof(MJTile) * MJDECK_NUM_TILES), 0);
  }

  /* シードを揃えたときに、同一の並びになるか？ */
  {
#define NUM_TRIALS 10
    struct MJDeck *deck;
    int32_t trial, is_ok;
    MJTile decks[NUM_TRIALS][MJDECK_NUM_TILES];
    struct MJRandomXoshiro256ppSeed xor_seed = {
      .seed = { ~1, 2, ~3, 4 },
    };

    /* 同一の種をセットしてシャッフル */
    deck = MJDeck_Create(NULL, NULL, 0);
    for (trial = 0; trial < NUM_TRIALS; trial++) {
      MJDeck_SetRandomSeed(deck, &xor_seed);
      MJDeck_Shuffle(deck);
      memcpy(&decks[trial][0],   deck->deck,        sizeof(MJTile) * 122);
      memcpy(&decks[trial][122], deck->rinshan,     sizeof(MJTile) *   4);
      memcpy(&decks[trial][126], deck->dora.omote,  sizeof(MJTile) *   5);
      memcpy(&decks[trial][131], deck->dora.ura,    sizeof(MJTile) *   5);
    }
    MJDeck_Destroy(deck);

    /* 全て同じ並びになることを期待 */
    is_ok = 1;
    for (trial = 1; trial < NUM_TRIALS; trial++) {
      if (memcmp(decks[0], decks[trial], sizeof(MJTile) * MJDECK_NUM_TILES) != 0) {
        is_ok = 0;
        break;
      }
    }
    Test_AssertEqual(is_ok, 1);
      
#undef NUM_TRIALS
  }

  /* 赤ドラ注入ケース: 1枚だけ */
  {
    int32_t t, count;
    struct MJDeck *deck;
    struct MJDeckConfig config;
    MJTile deck_tile[MJDECK_NUM_TILES];
    MJTile akadora_tile;

    /* 5筒を1枚赤ドラにしてみる */
    MJDeck_SetDefaultConfig(&config);
    config.akadora_count[MJTILE_5PIN] = 1;

    deck = MJDeck_Create(&config, NULL, 0);

    /* シャッフルしてから牌を配列として取得 */
    Test_AssertEqual(MJDeck_Shuffle(deck), MJDECK_APIRESULT_OK);
    memcpy(&deck_tile[0],   deck->deck,        sizeof(MJTile) * 122);
    memcpy(&deck_tile[122], deck->rinshan,     sizeof(MJTile) *   4);
    memcpy(&deck_tile[126], deck->dora.omote,  sizeof(MJTile) *   5);
    memcpy(&deck_tile[131], deck->dora.ura,    sizeof(MJTile) *   5);

    /* どこかに赤ドラが1枚入っているはず */
    count = 0;
    for (t = 0; t < MJDECK_NUM_TILES; t++) {
      if (MJTILE_IS_AKADORA(deck_tile[t])) {
        akadora_tile = deck_tile[t];
        count++;
      }
    }
    Test_AssertEqual(MJTILE_FLAG_MASK(akadora_tile), MJTILE_5PIN);
    Test_AssertEqual(count, 1);

    MJDeck_Destroy(deck);
  }

  /* 赤ドラ注入ケース: 一般的な3枚設定 */
  {
    int32_t t, count;
    struct MJDeck *deck;
    struct MJDeckConfig config;
    MJTile deck_tile[MJDECK_NUM_TILES];
    MJTile akadora_tiles[3];

    /* 3枚赤ドラにしてみる */
    MJDeck_SetDefaultConfig(&config);
    config.akadora_count[MJTILE_5MAN] = 1;
    config.akadora_count[MJTILE_5PIN] = 1;
    config.akadora_count[MJTILE_5SOU] = 1;

    deck = MJDeck_Create(&config, NULL, 0);

    /* シャッフルしてから牌を配列として取得 */
    Test_AssertEqual(MJDeck_Shuffle(deck), MJDECK_APIRESULT_OK);
    memcpy(&deck_tile[0],   deck->deck,        sizeof(MJTile) * 122);
    memcpy(&deck_tile[122], deck->rinshan,     sizeof(MJTile) *   4);
    memcpy(&deck_tile[126], deck->dora.omote,  sizeof(MJTile) *   5);
    memcpy(&deck_tile[131], deck->dora.ura,    sizeof(MJTile) *   5);

    /* 赤ドラ3枚を確認 */
    count = 0;
    for (t = 0; t < MJDECK_NUM_TILES; t++) {
      if (MJTILE_IS_AKADORA(deck_tile[t])) {
        if (count < 3) {
          akadora_tiles[count] = deck_tile[t];
        }
        count++;
      }
    }
    Test_AssertEqual(count, 3);
    qsort(akadora_tiles, 3, sizeof(MJTile), compare_tile);
    Test_AssertEqual(MJTILE_FLAG_MASK(akadora_tiles[0]), MJTILE_5MAN);
    Test_AssertEqual(MJTILE_FLAG_MASK(akadora_tiles[1]), MJTILE_5PIN);
    Test_AssertEqual(MJTILE_FLAG_MASK(akadora_tiles[2]), MJTILE_5SOU);

    MJDeck_Destroy(deck);
  }

  /* 赤ドラ注入ケース: 4枚設定 */
  {
    int32_t t, count;
    struct MJDeck *deck;
    struct MJDeckConfig config;
    MJTile deck_tile[MJDECK_NUM_TILES];
    MJTile akadora_tiles[4];

    /* 4枚赤ドラにしてみる */
    MJDeck_SetDefaultConfig(&config);
    config.akadora_count[MJTILE_5MAN] = 1;
    config.akadora_count[MJTILE_5PIN] = 2;
    config.akadora_count[MJTILE_5SOU] = 1;

    deck = MJDeck_Create(&config, NULL, 0);

    /* シャッフルしてから牌を配列として取得 */
    Test_AssertEqual(MJDeck_Shuffle(deck), MJDECK_APIRESULT_OK);
    memcpy(&deck_tile[0],   deck->deck,        sizeof(MJTile) * 122);
    memcpy(&deck_tile[122], deck->rinshan,     sizeof(MJTile) *   4);
    memcpy(&deck_tile[126], deck->dora.omote,  sizeof(MJTile) *   5);
    memcpy(&deck_tile[131], deck->dora.ura,    sizeof(MJTile) *   5);

    /* 赤ドラ4枚を確認 */
    count = 0;
    for (t = 0; t < MJDECK_NUM_TILES; t++) {
      if (MJTILE_IS_AKADORA(deck_tile[t])) {
        if (count < 4) {
          akadora_tiles[count] = deck_tile[t];
        }
        count++;
      }
    }
    Test_AssertEqual(count, 4);
    qsort(akadora_tiles, 4, sizeof(MJTile), compare_tile);
    Test_AssertEqual(MJTILE_FLAG_MASK(akadora_tiles[0]), MJTILE_5MAN);
    Test_AssertEqual(MJTILE_FLAG_MASK(akadora_tiles[1]), MJTILE_5PIN);
    Test_AssertEqual(MJTILE_FLAG_MASK(akadora_tiles[2]), MJTILE_5PIN);
    Test_AssertEqual(MJTILE_FLAG_MASK(akadora_tiles[3]), MJTILE_5SOU);

    MJDeck_Destroy(deck);
  }

  /* 赤ドラ注入ケース: 全て赤ドラ */
  {
    int32_t t, count;
    struct MJDeck *deck;
    struct MJDeckConfig config;
    MJTile deck_tile[MJDECK_NUM_TILES];

    MJDeck_SetDefaultConfig(&config);
    for (t = 0; t < MJTILE_MAX; t++) {
      config.akadora_count[t] = 4;
    }

    deck = MJDeck_Create(&config, NULL, 0);

    /* シャッフルしてから牌を配列として取得 */
    Test_AssertEqual(MJDeck_Shuffle(deck), MJDECK_APIRESULT_OK);
    memcpy(&deck_tile[0],   deck->deck,        sizeof(MJTile) * 122);
    memcpy(&deck_tile[122], deck->rinshan,     sizeof(MJTile) *   4);
    memcpy(&deck_tile[126], deck->dora.omote,  sizeof(MJTile) *   5);
    memcpy(&deck_tile[131], deck->dora.ura,    sizeof(MJTile) *   5);

    /* 赤ドラ枚数を数え上げ */
    count = 0;
    for (t = 0; t < MJDECK_NUM_TILES; t++) {
      if (MJTILE_IS_AKADORA(deck_tile[t])) {
        count++;
      }
    }
    Test_AssertEqual(count, MJDECK_NUM_TILES);

    MJDeck_Destroy(deck);
  }

  /* 失敗ケース */
  {
    /* 引数が不正 */
    Test_AssertEqual(MJDeck_Shuffle(NULL), MJDECK_APIRESULT_INVALID_ARGUMENT);
  }
}

/* 自摸/嶺上自摸テスト */
static void MJDeckTest_DrawTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* 基本ケース */
  {
    struct MJDeck *deck;
    MJTile tile;
    int32_t num_tiles;

    deck = MJDeck_Create(NULL, NULL, 0);

    /* シャッフル前は自摸できない */
    Test_AssertEqual(MJDeck_Draw(deck, &tile), MJDECK_APIRESULT_NOT_SHUFFLED);
    Test_AssertEqual(MJDeck_GetNumRemainTiles(deck, &num_tiles), MJDECK_APIRESULT_NOT_SHUFFLED);

    /* シャッフル */
    MJDeck_Shuffle(deck);

    /* 全部残っているので122枚のはず */
    Test_AssertEqual(MJDeck_GetNumRemainTiles(deck, &num_tiles), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_tiles, 122);

    /* 自摸 */
    Test_AssertEqual(MJDeck_Draw(deck, &tile), MJDECK_APIRESULT_OK);
    Test_AssertCondition(MJTILE_IS_JIHAI(tile) || MJTILE_IS_SUHAI(tile));
    Test_AssertEqual(deck->tsumo_pos, 1);

    /* 残り牌数は1枚減るはず */
    Test_AssertEqual(MJDeck_GetNumRemainTiles(deck, &num_tiles), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_tiles, 121);

    MJDeck_Destroy(deck);
  }

  /* 空になるまで自摸る */
  {
    struct MJDeck *deck;
    MJTile tile;
    int32_t i, is_ok, num_tiles;

    deck = MJDeck_Create(NULL, NULL, 0);

    MJDeck_Shuffle(deck);

    /* 自摸を繰り返す */
    is_ok = 1;
    for (i = 0; i < 122; i++) {
      if (MJDeck_Draw(deck, &tile) != MJDECK_APIRESULT_OK) {
        is_ok = 0;
        break;
      }
      if (!MJTILE_IS_JIHAI(tile) && !MJTILE_IS_SUHAI(tile)) {
        is_ok = 0;
        break;
      }
    }
    Test_AssertEqual(is_ok, 1);

    /* 残り0枚でもう一度自摸 */
    Test_AssertEqual(MJDeck_GetNumRemainTiles(deck, &num_tiles), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_tiles, 0);

    /* エラーの発生を確認 */
    Test_AssertEqual(MJDeck_Draw(deck, &tile), MJDECK_APIRESULT_EMPTY_DECK);

    MJDeck_Destroy(deck);
  }

  /* 嶺上自摸基本ケース */
  {
    struct MJDeck *deck;
    MJTile tile;
    int32_t num_tiles;

    deck = MJDeck_Create(NULL, NULL, 0);

    /* シャッフル前は自摸できない */
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &tile), MJDECK_APIRESULT_NOT_SHUFFLED);
    Test_AssertEqual(MJDeck_GetNumRemainRinshanTiles(deck, &num_tiles), MJDECK_APIRESULT_NOT_SHUFFLED);

    /* シャッフル */
    MJDeck_Shuffle(deck);

    /* 全部残っているので4枚のはず */
    Test_AssertEqual(MJDeck_GetNumRemainRinshanTiles(deck, &num_tiles), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_tiles, 4);

    /* 嶺上自摸 */
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &tile), MJDECK_APIRESULT_OK);
    Test_AssertCondition(MJTILE_IS_JIHAI(tile) || MJTILE_IS_SUHAI(tile));
    Test_AssertEqual(deck->rinshan_pos, 1);

    /* 残り牌数は1枚減るはず */
    Test_AssertEqual(MJDeck_GetNumRemainRinshanTiles(deck, &num_tiles), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_tiles, 3);

    /* ドラ表示数の変化確認 */
    Test_AssertEqual(deck->dora.num_dora, 2);
    /* 牌山の減少確認 */
    Test_AssertEqual(MJDeck_GetNumRemainTiles(deck, &num_tiles), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_tiles, 121);

    MJDeck_Destroy(deck);
  }

  /* 空になるまで嶺上自摸 */
  {
    struct MJDeck *deck;
    MJTile tile;
    int32_t i, is_ok, num_tiles;

    deck = MJDeck_Create(NULL, NULL, 0);

    /* シャッフル */
    MJDeck_Shuffle(deck);

    /* 嶺上自摸を繰り返す */
    is_ok = 1;
    for (i = 0; i < 4; i++) {
      if (MJDeck_RinshanDraw(deck, &tile) != MJDECK_APIRESULT_OK) {
        is_ok = 0;
        break;
      }
      if (!MJTILE_IS_JIHAI(tile) && !MJTILE_IS_SUHAI(tile)) {
        is_ok = 0;
        break;
      }
    }
    Test_AssertEqual(is_ok, 1);

    /* 残り0枚でもう一度嶺上自摸 */
    Test_AssertEqual(MJDeck_GetNumRemainRinshanTiles(deck, &num_tiles), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_tiles, 0);

    /* エラーの発生を確認 */
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &tile), MJDECK_APIRESULT_EMPTY_DECK);

    MJDeck_Destroy(deck);
  }

  /* 配山が残り0枚の場合は嶺上自摸ができないことの確認 */
  {
    struct MJDeck *deck;
    MJTile tile;
    int32_t i;

    deck = MJDeck_Create(NULL, NULL, 0);

    /* 空になるまで自摸 */
    MJDeck_Shuffle(deck);
    for (i = 0; i < 122; i++) {
      MJDeck_Draw(deck, &tile);
    }

    /* 嶺上自摸はできない */
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &tile), MJDECK_APIRESULT_EMPTY_DECK);

    MJDeck_Destroy(deck);
  }
}

/* ドラ取得テスト */
static void MJDeckTest_GetDoraTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* 基本ケース */
  {
    struct MJDeck *deck;
    struct MJDoraTile dora;
    MJTile tile;

    deck = MJDeck_Create(NULL, NULL, 0);

    /* シャッフル前は取得できない */
    Test_AssertEqual(MJDeck_GetDoraTile(deck, &dora), MJDECK_APIRESULT_NOT_SHUFFLED);

    /* シャッフル */
    MJDeck_Shuffle(deck);

    /* ドラ取得 */
    Test_AssertEqual(MJDeck_GetDoraTile(deck, &dora), MJDECK_APIRESULT_OK);
    Test_AssertEqual(dora.num_dora, 1);

    /* 嶺上自摸してドラが増えるはず */
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &tile), MJDECK_APIRESULT_OK);
    Test_AssertEqual(MJDeck_GetDoraTile(deck, &dora), MJDECK_APIRESULT_OK);
    Test_AssertEqual(dora.num_dora, 2);
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &tile), MJDECK_APIRESULT_OK);
    Test_AssertEqual(MJDeck_GetDoraTile(deck, &dora), MJDECK_APIRESULT_OK);
    Test_AssertEqual(dora.num_dora, 3);
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &tile), MJDECK_APIRESULT_OK);
    Test_AssertEqual(MJDeck_GetDoraTile(deck, &dora), MJDECK_APIRESULT_OK);
    Test_AssertEqual(dora.num_dora, 4);
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &tile), MJDECK_APIRESULT_OK);
    Test_AssertEqual(MJDeck_GetDoraTile(deck, &dora), MJDECK_APIRESULT_OK);
    Test_AssertEqual(dora.num_dora, 5);

    MJDeck_Destroy(deck);
  }
}
 

void MJDeckTest_Setup(void)
{
  struct TestSuite *suite
    = Test_AddTestSuite("MJ Deck Test Suite",
        NULL, MJDeckTest_Initialize, MJDeckTest_Finalize);

  Test_AddTest(suite, MJDeckTest_CreateDestroyTest);
  Test_AddTest(suite, MJDeckTest_ShuffleTest);
  Test_AddTest(suite, MJDeckTest_DrawTest);
  Test_AddTest(suite, MJDeckTest_GetDoraTest);
}

