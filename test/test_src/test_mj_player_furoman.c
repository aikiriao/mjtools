#include "test.h"

/* テスト対象のモジュール */
#include "../../player/src/mj_player_furoman.c"

/* テストのセットアップ関数 */
void MJPlayerFuromanTest_Setup(void);

static int MJPlayerFuromanTest_Initialize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

static int MJPlayerFuromanTest_Finalize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

/* 生成破棄テスト */
static void MJPlayerFuromanTest_CreateDestroyTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
}

void MJPlayerFuromanTest_Setup(void)
{
  struct TestSuite *suite
    = Test_AddTestSuite("MJ Player Furo-Man Test Suite",
        NULL, MJPlayerFuromanTest_Initialize, MJPlayerFuromanTest_Finalize);

  Test_AddTest(suite, MJPlayerFuromanTest_CreateDestroyTest);
}
