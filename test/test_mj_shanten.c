#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

/* テスト対象のモジュール */
#include "../mj_shanten.c"

/* 1枚の牌情報 */
struct TileInfo {
  int8_t  type;   /* 牌の種類(-1で終わり) */
  uint8_t maisu;  /* 所持数               */
};

/* テストケース */
struct MJShantenTestCase {
  struct TileInfo tiles[14];  /* 手牌情報（最大14エントリ） */
  uint8_t num_fuuro;          /* 副露数   */
  int32_t answer;             /* 正解     */
};

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
void MJShantenTest_Setup(void);

static int MJShantenTest_Initialize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

static int MJShantenTest_Finalize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

/* テストケースを手牌に変換 */
static void MJShantenTest_ConvertTestCaseToTehai(const struct MJShantenTestCase *test_case, struct Tehai *tehai)
{
  int32_t i;

  assert((test_case != NULL) && (tehai != NULL));

  /* 手牌を一旦クリア */
  memset(tehai, 0, sizeof(struct Tehai));

  /* テストケースから変換 */
  for (i = 0; i < 14; i++) {
    const struct TileInfo *info = &test_case->tiles[i];
    if (info->type < 0) {
      break;
    }
    tehai->tehai[info->type] = info->maisu;
  }
}

/* 七対子の向聴数計算テスト */
static void MJShantenTest_CalculateChitoitsuSyantenTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  {
    uint32_t i, is_ok;

    /* テストケース */
    const struct MJShantenTestCase test_cases[] = {
      /* 和了ケース(向聴数-1) */
      {
        {
          { MJTILE_1MAN, 2 }, { MJTILE_2MAN, 2 }, { MJTILE_3MAN, 2 },
          { MJTILE_4MAN, 2 }, { MJTILE_5MAN, 2 }, { MJTILE_6MAN, 2 },
          { MJTILE_7MAN, 2 }, { -1, 0 },
        },
        0, -1,
      },
      {
        {
          { MJTILE_1PIN, 2 }, { MJTILE_2PIN, 2 }, { MJTILE_3PIN, 2 },
          { MJTILE_4PIN, 2 }, { MJTILE_5PIN, 2 }, { MJTILE_6PIN, 2 },
          { MJTILE_7PIN, 2 }, { -1, 0 },
        },
        0, -1,
      },
      {
        {
          { MJTILE_1SOU, 2 }, { MJTILE_2SOU, 2 }, { MJTILE_3SOU, 2 },
          { MJTILE_4SOU, 2 }, { MJTILE_5SOU, 2 }, { MJTILE_6SOU, 2 },
          { MJTILE_7SOU, 2 }, { -1, 0 },
        },
        0, -1,
      },
      {
        /* 本当は字一色も複合する */
        {
          { MJTILE_TON,  2 }, { MJTILE_NAN,  2 }, { MJTILE_SHA,  2 },
          { MJTILE_PEE,  2 }, { MJTILE_HAKU, 2 }, { MJTILE_HATU, 2 },
          { MJTILE_CHUN, 2 }, { -1, 0 },
        },
        0, -1,
      },
      /* 聴牌ケース(向牌数0) */
      {
        {
          { MJTILE_1MAN, 2 }, { MJTILE_2MAN, 2 }, { MJTILE_3MAN, 2 },
          { MJTILE_4MAN, 2 }, { MJTILE_5MAN, 2 }, { MJTILE_6MAN, 2 },
          { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { -1, 0 },
        },
        0, 0,
      },
      {
        {
          { MJTILE_1MAN, 3 }, { MJTILE_2MAN, 2 }, { MJTILE_3MAN, 2 },
          { MJTILE_4MAN, 2 }, { MJTILE_5MAN, 2 }, { MJTILE_6MAN, 2 },
          { MJTILE_7MAN, 1 }, { -1, 0 },
        },
        0, 0,
      },
      /* 一向聴ケース(向牌数1) */
      {
        {
          { MJTILE_1MAN, 2 }, { MJTILE_2MAN, 2 }, { MJTILE_3MAN, 2 },
          { MJTILE_4MAN, 2 }, { MJTILE_5MAN, 2 }, { MJTILE_6MAN, 1 },
          { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { MJTILE_9MAN, 1 },
          { -1, 0 },
        },
        0, 1,
      },
      {
        {
          { MJTILE_1MAN, 3 }, { MJTILE_2MAN, 2 }, { MJTILE_3MAN, 2 },
          { MJTILE_4MAN, 2 }, { MJTILE_5MAN, 2 }, { MJTILE_6MAN, 1 },
          { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { -1, 0 },
        },
        0, 1,
      },
      {
        {
          { MJTILE_1MAN, 4 }, { MJTILE_2MAN, 2 }, { MJTILE_3MAN, 2 },
          { MJTILE_4MAN, 2 }, { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 },
          { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { -1, 0 },
        },
        0, 2,
      },
      /* 4枚含むケース */
      {
        {
          { MJTILE_1MAN, 4 }, { MJTILE_2MAN, 2 }, { MJTILE_3MAN, 2 },
          { MJTILE_4MAN, 2 }, { MJTILE_5MAN, 2 }, { MJTILE_6MAN, 2 },
          { -1, 0 },
        },
        0, 1,
      },
      {
        {
          { MJTILE_1MAN, 4 }, { MJTILE_2MAN, 4 }, { MJTILE_3MAN, 2 },
          { MJTILE_4MAN, 2 }, { MJTILE_5MAN, 2 }, { -1, 0 },
        },
        0, 3,
      },
      {
        {
          { MJTILE_1MAN, 4 }, { MJTILE_2MAN, 4 }, { MJTILE_3MAN, 4 },
          { MJTILE_5MAN, 2 }, { -1, 0 },
        },
        0, 5,
      },
      {
        {
          { MJTILE_1MAN, 4 }, { MJTILE_2MAN, 2 }, { MJTILE_3MAN, 2 },
          { MJTILE_4MAN, 2 }, { MJTILE_5MAN, 2 }, { MJTILE_6MAN, 1 },
          { MJTILE_7MAN, 1 }, { -1, 0 },
        },
        0, 1,
      },
      /* 六向聴ケース */
      {
        {
          { MJTILE_1MAN, 1 }, { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 },
          { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 },
          { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { MJTILE_9MAN, 1 },
          { MJTILE_1PIN, 1 }, { MJTILE_2PIN, 1 }, { MJTILE_3PIN, 1 },
          { MJTILE_4PIN, 1 }, { MJTILE_5PIN, 1 }, 
        },
        0, 6,
      },
    };
    const uint32_t num_test = sizeof(test_cases) / sizeof(test_cases[0]);

    /* 全ケースで確認 */
    is_ok = 1;
    for (i = 0; i < num_test; i++) {
      int32_t syanten;
      struct Tehai tehai;
      const struct MJShantenTestCase *pcase = &test_cases[i];
      MJShantenTest_ConvertTestCaseToTehai(pcase, &tehai);
      syanten = MJShanten_CalculateChitoitsuSyanten(&tehai);
      if (syanten != pcase->answer) {
        printf("NG at test case index:%d get:%d answer:%d \n",
            i, syanten, pcase->answer);
        is_ok = 0;
      }
    }
    Test_AssertEqual(is_ok, 1);
  }
}

/* 国士無双の向聴数計算テスト */
static void MJShantenTest_CalculateKokushimusouSyantenTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  {
    uint32_t i, is_ok;

    /* テストケース */
    const struct MJShantenTestCase test_cases[] = {
      /* 和了ケース(向聴数-1) */
      {
        {
          { MJTILE_1MAN, 1 }, { MJTILE_9MAN, 1 }, { MJTILE_1PIN, 1 }, { MJTILE_9PIN, 1 },
          { MJTILE_1SOU, 1 }, { MJTILE_9SOU, 1 }, { MJTILE_TON,  1 }, { MJTILE_NAN,  1 },
          { MJTILE_SHA,  1 }, { MJTILE_PEE,  1 }, { MJTILE_HAKU, 1 }, { MJTILE_HATU, 1 },
          { MJTILE_CHUN, 2 }, { -1, 0 },
        },
        0, -1,
      },
      /* 一向聴ケース */
      {
        {
          { MJTILE_1MAN, 1 }, { MJTILE_9MAN, 1 }, { MJTILE_1PIN, 1 }, { MJTILE_9PIN, 1 },
          { MJTILE_1SOU, 1 }, { MJTILE_9SOU, 1 }, { MJTILE_TON,  1 }, { MJTILE_NAN,  1 },
          { MJTILE_SHA,  1 }, { MJTILE_PEE,  1 }, { MJTILE_HAKU, 1 }, { MJTILE_HATU, 1 },
          { MJTILE_CHUN, 1 }, 
          { MJTILE_2MAN, 1 },
        },
        0, 0,
      },
      {
        {
          { MJTILE_1MAN, 2 }, { MJTILE_9MAN, 1 }, { MJTILE_1PIN, 1 }, { MJTILE_9PIN, 1 },
          { MJTILE_1SOU, 1 }, { MJTILE_9SOU, 1 }, { MJTILE_TON,  1 }, { MJTILE_NAN,  1 },
          { MJTILE_SHA,  1 }, { MJTILE_PEE,  1 }, { MJTILE_HAKU, 1 }, { MJTILE_HATU, 1 },
          { MJTILE_2MAN, 1 },
        },
        0, 0,
      },
      /* 一三向聴ケース */
      {
        {
          { MJTILE_2MAN, 4 }, { MJTILE_3MAN, 4 }, { MJTILE_4MAN, 4 }, { MJTILE_5MAN, 2 },
          { -1, 0 },
        },
        0, 13,
      },
    };
    const uint32_t num_test = sizeof(test_cases) / sizeof(test_cases[0]);

    /* 全ケースで確認 */
    is_ok = 1;
    for (i = 0; i < num_test; i++) {
      int32_t syanten;
      struct Tehai tehai;
      const struct MJShantenTestCase *pcase = &test_cases[i];
      MJShantenTest_ConvertTestCaseToTehai(pcase, &tehai);
      syanten = MJShanten_CalculateKokushimusouSyanten(&tehai);
      if (syanten != pcase->answer) {
        printf("NG at test case index:%d get:%d answer:%d \n", i, syanten, pcase->answer);
        is_ok = 0;
        break;
      }
    }
    Test_AssertEqual(is_ok, 1);
  }
}

/* 通常手の向聴数計算テスト */
static void MJShantenTest_CalculateNormalSyantenTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  {
    uint32_t i, is_ok;

    /* テストケース */
    const struct MJShantenTestCase test_cases[] = {
      /* 和了ケース(向聴数-1) */
      /* 四槓子 */
      {
        {
          { MJTILE_1MAN, 4 }, { MJTILE_2MAN, 4 }, { MJTILE_3MAN, 4 },
          { MJTILE_5MAN, 2 }, { -1, 0 },
        },
        0, -1,
      },
      /* 四暗刻 */
      {
        {
          { MJTILE_1MAN, 3 }, { MJTILE_2MAN, 3 }, { MJTILE_3MAN, 3 }, { MJTILE_4MAN, 3 },
          { MJTILE_5MAN, 2 }, { -1, 0 },
        },
        0, -1,
      },
      /* 大三元(2副露) */
      {
        {
          { MJTILE_HAKU, 3 }, { MJTILE_HATU, 3 }, { MJTILE_CHUN, 3 },
          { MJTILE_1PIN, 1 }, { MJTILE_2PIN, 1 }, { MJTILE_3PIN, 1 },
          { MJTILE_TON,  2 }, { -1, 0 },
        },
        2, -1,
      },
      /* 一気通貫(1副露) */
      {
        {
          { MJTILE_1MAN, 1 }, { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 },
          { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 },
          { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { MJTILE_9MAN, 1 },
          { MJTILE_CHUN, 3 }, { MJTILE_HATU, 2 }, { -1, 0 },
        },
        1, -1,
      },
      /* 聴牌ケース */
      /* 九蓮宝燈9面待ち */
      {
        {
          { MJTILE_1MAN, 3 }, { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 },
          { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 },
          { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { MJTILE_9MAN, 3 },
          { MJTILE_CHUN, 1 },
        },
        0, 0,
      },
      /* 一向聴ケース */
      /* 中のみ（のつもり） */
      {
        {
          { MJTILE_1MAN, 1 }, { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 },
          { MJTILE_6MAN, 1 }, { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 },
          { MJTILE_1SOU, 2 }, { MJTILE_2SOU, 1 },
          { MJTILE_SHA,  1 }, { MJTILE_PEE,  1 }, { MJTILE_CHUN, 3 },
        },
        0, 1,
      },
    };
    const uint32_t num_test = sizeof(test_cases) / sizeof(test_cases[0]);

    /* 全ケースで確認 */
    is_ok = 1;
    for (i = 0; i < num_test; i++) {
      int32_t syanten;
      struct Tehai tehai;
      const struct MJShantenTestCase *pcase = &test_cases[i];
      MJShantenTest_ConvertTestCaseToTehai(pcase, &tehai);
      syanten = MJShanten_CalculateNormalSyanten(&tehai);
      if (syanten != pcase->answer) {
        printf("NG at test case index:%d get:%d answer:%d \n", i, syanten, pcase->answer);
        is_ok = 0;
        break;
      }
    }
    Test_AssertEqual(is_ok, 1);
  }
}

/* 問題集の1行をパースし、牌情報と回答を取得 */
static void MJShantenTest_ParseLine(
    const char *string, struct Tehai *tehai,
    int32_t *normal_answer, int32_t *kokushi_answer, int32_t *chitoi_answer)
{
  int32_t i;
  int32_t buf[17];
  /* 問題集の牌種をテストケースの牌種に変換 */
  static const int8_t tiletype_convert_table[34] = {
    MJTILE_1MAN, MJTILE_2MAN, MJTILE_3MAN,
    MJTILE_4MAN, MJTILE_5MAN, MJTILE_6MAN,
    MJTILE_7MAN, MJTILE_8MAN, MJTILE_9MAN,
    MJTILE_1PIN, MJTILE_2PIN, MJTILE_3PIN,
    MJTILE_4PIN, MJTILE_5PIN, MJTILE_6PIN,
    MJTILE_7PIN, MJTILE_8PIN, MJTILE_9PIN,
    MJTILE_1SOU, MJTILE_2SOU, MJTILE_3SOU,
    MJTILE_4SOU, MJTILE_5SOU, MJTILE_6SOU,
    MJTILE_7SOU, MJTILE_8SOU, MJTILE_9SOU,
    MJTILE_TON,  MJTILE_NAN,  MJTILE_SHA,  MJTILE_PEE,
    MJTILE_HAKU, MJTILE_HATU, MJTILE_CHUN,
  };

  assert((string != NULL) && (tehai != NULL));
  assert((normal_answer != NULL) && (kokushi_answer != NULL) && (chitoi_answer != NULL));

  /* 17数字を一気に読み取り */
  sscanf(string, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
      &buf[0], &buf[1], &buf[2],  &buf[3],  &buf[4],  &buf[5],  &buf[6],  &buf[7],
      &buf[8], &buf[9], &buf[10], &buf[11], &buf[12], &buf[13], &buf[14], &buf[15], &buf[16]);

  /* 牌情報を一旦クリア */
  memset(tehai, 0, sizeof(struct Tehai));

  /* 牌情報をセット */
  for (i = 0; i < 14; i++) {
    tehai->tehai[tiletype_convert_table[buf[i]]]++;
  }

  /* 回答をセット */
  (*normal_answer)  = buf[14];
  (*kokushi_answer) = buf[15];
  (*chitoi_answer)  = buf[16];
}

/* 問題集のファイル名指定でテスト実行 */
static int32_t MJShantenTest_CalculateSyantenForProblemFile(const char *filename)
{
  FILE *fp;
  char linebuf[256];
  struct Tehai tehai;
  uint32_t total, normal_ok, kokusi_ok, chitoi_ok;
  int32_t normal_answer, kokushi_answer, chitoi_answer;
  int32_t normal_syanten, kokushi_syanten, chitoi_syanten;

  /* ファイルオープン */
  fp = fopen(filename, "r");
  assert(fp != NULL);

  total = 0;
  normal_ok = kokusi_ok = chitoi_ok = 0;
  while (fgets(linebuf, sizeof(linebuf), fp) != NULL) {
    MJShantenTest_ParseLine(linebuf, &tehai, &normal_answer, &kokushi_answer, &chitoi_answer);
    normal_syanten  = MJShanten_CalculateNormalSyanten(&tehai);
    chitoi_syanten  = MJShanten_CalculateChitoitsuSyanten(&tehai);
    kokushi_syanten = MJShanten_CalculateKokushimusouSyanten(&tehai);
    if (normal_syanten == normal_answer)   { normal_ok++; };
    if (chitoi_syanten == chitoi_answer)   { chitoi_ok++; };
    if (kokushi_syanten == kokushi_answer) { kokusi_ok++; };
    if (normal_syanten != normal_answer) {
      int i, j;
      printf("get:%d ans:%d \n", normal_syanten, normal_answer);
      for (i = 0; i < MJTILE_MAX; i++) {
        for (j = 0; j < tehai.tehai[i]; j++) {
          printf("%s ", tile_string_table[i]);
        }
      }
      puts("");
    }
    total++;
  }
  printf("[%30s] Total:%d Normal: %d, Chitoi: %d, Kokushi: %d \n", 
      filename, total, normal_ok, chitoi_ok, kokusi_ok);

  fclose(fp);

  /* 全問正解すべし */
  if ((normal_ok != total) || (chitoi_ok != total) || (kokusi_ok != total)) {
    return 0;
  }

  return 1;
}

/* 問題集の向聴数計算テスト */
static void MJShantenTest_CalculateSyantenForProblems(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  {
    /* 通常手 */
    Test_AssertEqual(MJShantenTest_CalculateSyantenForProblemFile("./problems/p_normal_10000.txt"), 1);
    /* 国士無双手 */
    Test_AssertEqual(MJShantenTest_CalculateSyantenForProblemFile("./problems/p_koku_10000.txt"), 1);
    /* 混一手 */
    Test_AssertEqual(MJShantenTest_CalculateSyantenForProblemFile("./problems/p_hon_10000.txt"), 1);
    /* 清一手 */
    Test_AssertEqual(MJShantenTest_CalculateSyantenForProblemFile("./problems/p_tin_10000.txt"), 1);
  }
}

void MJShantenTest_Setup(void)
{
  struct TestSuite *suite
    = Test_AddTestSuite("MJ Syanten Test Suite",
        NULL, MJShantenTest_Initialize, MJShantenTest_Finalize);

  Test_AddTest(suite, MJShantenTest_CalculateChitoitsuSyantenTest);
  Test_AddTest(suite, MJShantenTest_CalculateKokushimusouSyantenTest);
  Test_AddTest(suite, MJShantenTest_CalculateNormalSyantenTest);
  Test_AddTest(suite, MJShantenTest_CalculateSyantenForProblems);
}
