#include "test.h"

/* テスト対象のモジュール */
#include "../mj_player_cui.c"

/* テストのセットアップ関数 */
void MJPlayerCUITest_Setup(void);

static int MJPlayerCUITest_Initialize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

static int MJPlayerCUITest_Finalize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

/* 生成破棄テスト */
static void MJPlayerCUITest_CreateDestroyTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
}

void MJPlayerCUITest_Setup(void)
{
  struct TestSuite *suite
    = Test_AddTestSuite("MJ Player CUI Client Test Suite",
        NULL, MJPlayerCUITest_Initialize, MJPlayerCUITest_Finalize);

  Test_AddTest(suite, MJPlayerCUITest_CreateDestroyTest);
}
