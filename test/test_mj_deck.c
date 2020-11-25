#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

/* テスト対象のモジュール */
#include "../mj_deck.c"

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
    uint32_t i, kyoku, is_ok;
    uint32_t hai_count[MJTILE_MAX];
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
        hai_count[i] = 0;
      }
      /* 牌山 */
      for (i = 0; i < 122; i++) {
        hai_count[deck->deck[i]]++;
      }
      /* 嶺上牌 */
      for (i = 0; i < 4; i++) {
        hai_count[deck->rinshan[i]]++;
      }
      /* ドラ牌 */
      for (i = 0; i < 5; i++) {
        hai_count[deck->dora.omote[i]]++;
        hai_count[deck->dora.ura[i]]++;
      }

      /* 全ての牌が4枚ずつ存在しなければならない */
      for (i = 0; i < MJTILE_MAX; i++) {
        if (MJTILE_IS_SUHAI(i) || MJTILE_IS_JIHAI(i)) {
          if (hai_count[i] != 4) {
            printf("%d %d \n", i, hai_count[i]);
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
    MJTile shufffled_deck[136];
    struct MJDeck *deck;

    deck = MJDeck_Create(NULL, NULL, 0);
    MJDeck_Shuffle(deck);
    memcpy(&shufffled_deck[0],   deck->deck,        sizeof(MJTile) * 122);
    memcpy(&shufffled_deck[122], deck->rinshan,     sizeof(MJTile) *   4);
    memcpy(&shufffled_deck[126], deck->dora.omote,  sizeof(MJTile) *   5);
    memcpy(&shufffled_deck[131], deck->dora.ura,    sizeof(MJTile) *   5);
    MJDeck_Destroy(deck);

    Test_AssertNotEqual(memcmp(shufffled_deck, default_deck, sizeof(MJTile) * 136), 0);
  }

  /* シードを揃えたときに、同一の並びになるか？ */
  {
#define NUM_TRIALS 10
    struct MJDeck *deck;
    uint32_t trial, is_ok;
    MJTile decks[NUM_TRIALS][136];
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
      if (memcmp(decks[0], decks[trial], sizeof(MJTile) * 136) != 0) {
        is_ok = 0;
        break;
      }
    }
    Test_AssertEqual(is_ok, 1);
      
#undef NUM_TRIALS
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
    MJTile hai;
    uint32_t num_hais;

    deck = MJDeck_Create(NULL, NULL, 0);

    /* シャッフル前は自摸できない */
    Test_AssertEqual(MJDeck_Draw(deck, &hai), MJDECK_APIRESULT_NOT_SHUFFLED);
    Test_AssertEqual(MJDeck_GetNumRemainHais(deck, &num_hais), MJDECK_APIRESULT_NOT_SHUFFLED);

    /* シャッフル */
    MJDeck_Shuffle(deck);

    /* 全部残っているので122枚のはず */
    Test_AssertEqual(MJDeck_GetNumRemainHais(deck, &num_hais), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_hais, 122);

    /* 自摸 */
    Test_AssertEqual(MJDeck_Draw(deck, &hai), MJDECK_APIRESULT_OK);
    Test_AssertCondition(MJTILE_IS_JIHAI(hai) || MJTILE_IS_SUHAI(hai));
    Test_AssertEqual(deck->tsumo_pos, 1);

    /* 残り牌数は1枚減るはず */
    Test_AssertEqual(MJDeck_GetNumRemainHais(deck, &num_hais), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_hais, 121);

    MJDeck_Destroy(deck);
  }

  /* 空になるまで自摸る */
  {
    struct MJDeck *deck;
    MJTile hai;
    uint32_t i, is_ok, num_hais;

    deck = MJDeck_Create(NULL, NULL, 0);

    MJDeck_Shuffle(deck);

    /* 自摸を繰り返す */
    is_ok = 1;
    for (i = 0; i < 122; i++) {
      if (MJDeck_Draw(deck, &hai) != MJDECK_APIRESULT_OK) {
        is_ok = 0;
        break;
      }
      if (!MJTILE_IS_JIHAI(hai) && !MJTILE_IS_SUHAI(hai)) {
        is_ok = 0;
        break;
      }
    }
    Test_AssertEqual(is_ok, 1);

    /* 残り0枚でもう一度自摸 */
    Test_AssertEqual(MJDeck_GetNumRemainHais(deck, &num_hais), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_hais, 0);

    /* エラーの発生を確認 */
    Test_AssertEqual(MJDeck_Draw(deck, &hai), MJDECK_APIRESULT_EMPTY_DECK);

    MJDeck_Destroy(deck);
  }

  /* 嶺上自摸基本ケース */
  {
    struct MJDeck *deck;
    MJTile hai;
    uint32_t num_hais;

    deck = MJDeck_Create(NULL, NULL, 0);

    /* シャッフル前は自摸できない */
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &hai), MJDECK_APIRESULT_NOT_SHUFFLED);
    Test_AssertEqual(MJDeck_GetNumRemainRinshanHais(deck, &num_hais), MJDECK_APIRESULT_NOT_SHUFFLED);

    /* シャッフル */
    MJDeck_Shuffle(deck);

    /* 全部残っているので4枚のはず */
    Test_AssertEqual(MJDeck_GetNumRemainRinshanHais(deck, &num_hais), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_hais, 4);

    /* 嶺上自摸 */
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &hai), MJDECK_APIRESULT_OK);
    Test_AssertCondition(MJTILE_IS_JIHAI(hai) || MJTILE_IS_SUHAI(hai));
    Test_AssertEqual(deck->rinshan_pos, 1);

    /* 残り牌数は1枚減るはず */
    Test_AssertEqual(MJDeck_GetNumRemainRinshanHais(deck, &num_hais), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_hais, 3);

    /* ドラ表示数の変化確認 */
    Test_AssertEqual(deck->dora.num_dora, 2);
    /* 牌山の減少確認 */
    Test_AssertEqual(MJDeck_GetNumRemainHais(deck, &num_hais), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_hais, 121);

    MJDeck_Destroy(deck);
  }

  /* 空になるまで嶺上自摸 */
  {
    struct MJDeck *deck;
    MJTile hai;
    uint32_t i, is_ok, num_hais;

    deck = MJDeck_Create(NULL, NULL, 0);

    /* シャッフル */
    MJDeck_Shuffle(deck);

    /* 嶺上自摸を繰り返す */
    is_ok = 1;
    for (i = 0; i < 4; i++) {
      if (MJDeck_RinshanDraw(deck, &hai) != MJDECK_APIRESULT_OK) {
        is_ok = 0;
        break;
      }
      if (!MJTILE_IS_JIHAI(hai) && !MJTILE_IS_SUHAI(hai)) {
        is_ok = 0;
        break;
      }
    }
    Test_AssertEqual(is_ok, 1);

    /* 残り0枚でもう一度嶺上自摸 */
    Test_AssertEqual(MJDeck_GetNumRemainRinshanHais(deck, &num_hais), MJDECK_APIRESULT_OK);
    Test_AssertEqual(num_hais, 0);

    /* エラーの発生を確認 */
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &hai), MJDECK_APIRESULT_EMPTY_DECK);

    MJDeck_Destroy(deck);
  }

  /* 配山が残り0枚の場合は嶺上自摸ができないことの確認 */
  {
    struct MJDeck *deck;
    MJTile hai;
    uint32_t i;

    deck = MJDeck_Create(NULL, NULL, 0);

    /* 空になるまで自摸 */
    MJDeck_Shuffle(deck);
    for (i = 0; i < 122; i++) {
      MJDeck_Draw(deck, &hai);
    }

    /* 嶺上自摸はできない */
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &hai), MJDECK_APIRESULT_EMPTY_DECK);

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
    struct MJDoraHai dora;
    MJTile hai;

    deck = MJDeck_Create(NULL, NULL, 0);

    /* シャッフル前は取得できない */
    Test_AssertEqual(MJDeck_GetDoraHai(deck, &dora), MJDECK_APIRESULT_NOT_SHUFFLED);

    /* シャッフル */
    MJDeck_Shuffle(deck);

    /* ドラ取得 */
    Test_AssertEqual(MJDeck_GetDoraHai(deck, &dora), MJDECK_APIRESULT_OK);
    Test_AssertEqual(dora.num_dora, 1);

    /* 嶺上自摸してドラが増えるはず */
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &hai), MJDECK_APIRESULT_OK);
    Test_AssertEqual(MJDeck_GetDoraHai(deck, &dora), MJDECK_APIRESULT_OK);
    Test_AssertEqual(dora.num_dora, 2);
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &hai), MJDECK_APIRESULT_OK);
    Test_AssertEqual(MJDeck_GetDoraHai(deck, &dora), MJDECK_APIRESULT_OK);
    Test_AssertEqual(dora.num_dora, 3);
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &hai), MJDECK_APIRESULT_OK);
    Test_AssertEqual(MJDeck_GetDoraHai(deck, &dora), MJDECK_APIRESULT_OK);
    Test_AssertEqual(dora.num_dora, 4);
    Test_AssertEqual(MJDeck_RinshanDraw(deck, &hai), MJDECK_APIRESULT_OK);
    Test_AssertEqual(MJDeck_GetDoraHai(deck, &dora), MJDECK_APIRESULT_OK);
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

