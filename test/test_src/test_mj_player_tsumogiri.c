#include "test.h"

/* テスト対象のモジュール */
#include "../../player/src/mj_player_tsumogiri.c"

/* テストのセットアップ関数 */
void MJPlayerTsumogiriTest_Setup(void);

static int MJPlayerTsumogiriTest_Initialize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

static int MJPlayerTsumogiriTest_Finalize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

/* 生成破棄テスト */
static void MJPlayerTsumogiriTest_CreateDestroyTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
}

void MJPlayerTsumogiriTest_Setup(void)
{
  struct TestSuite *suite
    = Test_AddTestSuite("MJ Player Tsumogiri Test Suite",
        NULL, MJPlayerTsumogiriTest_Initialize, MJPlayerTsumogiriTest_Finalize);

  Test_AddTest(suite, MJPlayerTsumogiriTest_CreateDestroyTest);
}
