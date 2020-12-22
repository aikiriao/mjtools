#include "test.h"

/* 各テストスイートのセットアップ関数宣言 */
void MJShantenTest_Setup(void);
void MJTileTest_Setup(void);
void MJScoreTest_Setup(void);
void MJRandomXoshiro256ppTest_Setup(void);
void MJDeckTest_Setup(void);
void MJPlayerTest_Setup(void);
void MJPlayerTsumogiriTest_Setup(void);
void MJRiverTest_Setup(void);

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
  MJRandomXoshiro256ppTest_Setup();
  MJDeckTest_Setup();
  MJPlayerTest_Setup();
  MJPlayerTsumogiriTest_Setup();
  MJRiverTest_Setup();

  ret = Test_RunAllTestSuite();

  Test_PrintAllFailures();

  Test_Finalize();

  return ret;
}
