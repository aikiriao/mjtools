#include "test.h"

/* テスト対象のモジュール */
#include "../mj_player_shantenman.c"

/* テストのセットアップ関数 */
void MJPlayerShantenmanTest_Setup(void);

static int MJPlayerShantenmanTest_Initialize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

static int MJPlayerShantenmanTest_Finalize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

/* 生成破棄テスト */
static void MJPlayerShantenmanTest_CreateDestroyTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
}

void MJPlayerShantenmanTest_Setup(void)
{
  struct TestSuite *suite
    = Test_AddTestSuite("MJ Player Shanten-Man Test Suite",
        NULL, MJPlayerShantenmanTest_Initialize, MJPlayerShantenmanTest_Finalize);

  Test_AddTest(suite, MJPlayerShantenmanTest_CreateDestroyTest);
}
