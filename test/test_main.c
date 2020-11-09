#include "test.h"

/* 各テストスイートのセットアップ関数宣言 */
void MJShantenTest_Setup(void);
void MJTileTest_Setup(void);
void MJScoreTest_Setup(void);

/* テスト実行 */
int main(int argc, char **argv)
{
  int ret;

  TEST_UNUSED_PARAMETER(argc);
  TEST_UNUSED_PARAMETER(argv);

  Test_Initialize();

  MJShantenTest_Setup();
  MJTileTest_Setup();
  MJScoreTest_Setup();

  ret = Test_RunAllTestSuite();

  Test_PrintAllFailures();

  Test_Finalize();

  return ret;
}
