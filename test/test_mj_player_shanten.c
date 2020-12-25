#include "test.h"

/* テスト対象のモジュール */
#include "../mj_player_shanten.c"

/* テストのセットアップ関数 */
void MJPlayerShantenTest_Setup(void);

static int MJPlayerShantenTest_Initialize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

static int MJPlayerShantenTest_Finalize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

/* 生成破棄テスト */
static void MJPlayerShantenTest_CreateDestroyTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
}

void MJPlayerShantenTest_Setup(void)
{
  struct TestSuite *suite
    = Test_AddTestSuite("MJ Player Shanten Test Suite",
        NULL, MJPlayerShantenTest_Initialize, MJPlayerShantenTest_Finalize);

  Test_AddTest(suite, MJPlayerShantenTest_CreateDestroyTest);
}
