#include "test.h"

/* 各テストスイートのセットアップ関数宣言 */
void MJShantenTest_Setup(void);
void MJTileTest_Setup(void);
void MJScoreTest_Setup(void);
void MJRandomXoshiro256ppTest_Setup(void);
void MJDeckTest_Setup(void);
void MJPlayerTsumogiriTest_Setup(void);
void MJPlayerShantenmanTest_Setup(void);
void MJPlayerFuromanTest_Setup(void);
void MJPlayerCUITest_Setup(void);
void MJPlayerWrapperTest_Setup(void);
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
  MJPlayerTsumogiriTest_Setup();
  MJPlayerShantenmanTest_Setup();
  MJPlayerFuromanTest_Setup();
  MJPlayerCUITest_Setup();
  MJPlayerWrapperTest_Setup();
  MJRiverTest_Setup();

  ret = Test_RunAllTestSuite();

  Test_PrintAllFailures();

  Test_Finalize();

  return ret;
}
