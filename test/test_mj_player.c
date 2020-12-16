#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

/* テスト対象のモジュール */
#include "../mj_player.c"

/* 仮のプレーヤーインターフェースにツモ切りくんを使用 */
#include "../mj_player_tsumogiri.h"

/* テストのセットアップ関数 */
void MJPlayerTest_Setup(void);

static int MJPlayerTest_Initialize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

static int MJPlayerTest_Finalize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

/* 生成破棄テスト */
static void MJPlayerTest_CreateDestroyTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* ワークサイズ計算テスト */
  {
    struct MJPlayerConfig config;

    /* NULL指定 */
    Test_AssertCondition(MJPlayer_CalculateWorkSize(NULL) < 0);
    /* インターフェースがNULL */
    config.player_interface = NULL;
    Test_AssertCondition(MJPlayer_CalculateWorkSize(&config) < 0);

    /* インターフェース設定 */
    config.player_interface = MJPlayerTsumogiri_GetInterface();
    Test_AssertCondition(MJPlayer_CalculateWorkSize(&config) >= 0);
  }

  /* ワーク領域渡しによる生成・破棄（コンフィグ指定） */
  {
    int32_t work_size;
    void *work;
    struct MJPlayer *player = NULL;
    struct MJPlayerConfig config;

    /* コンフィグ作成 */
    config.player_interface = MJPlayerTsumogiri_GetInterface();

    work_size = MJPlayer_CalculateWorkSize(&config);
    Test_AssertCondition(work_size >= 0);

    work = malloc(work_size);
    player = MJPlayer_Create(&config, work, work_size);
    Test_AssertCondition(player != NULL);
    Test_AssertEqual(player->alloced_by_own, false);
    Test_AssertCondition(player->player_instance != NULL);
    Test_AssertCondition(player->work != NULL);
    Test_AssertCondition(player->player_interface == config.player_interface);

    MJPlayer_Destroy(player);
    free(work);
  }

  /* ワーク領域自前確保による生成・破棄（コンフィグ指定） */
  {
    struct MJPlayer *player = NULL;
    struct MJPlayerConfig config;

    /* コンフィグ作成 */
    config.player_interface = MJPlayerTsumogiri_GetInterface();

    player = MJPlayer_Create(&config, NULL, 0);
    Test_AssertCondition(player != NULL);
    Test_AssertEqual(player->alloced_by_own, true);
    Test_AssertCondition(player->work != NULL);
    Test_AssertCondition(player->player_instance != NULL);
    Test_AssertCondition(player->player_interface == config.player_interface);

    MJPlayer_Destroy(player);
  }

  /* 作成失敗ケース */
  {
    int32_t work_size;
    void *work;
    struct MJPlayerConfig config;

    /* 正常なワークサイズとワーク領域を用意 */
    config.player_interface = MJPlayerTsumogiri_GetInterface();
    work_size = MJPlayer_CalculateWorkSize(&config);
    work = malloc(work_size);

    /* 不正な引数 */
    Test_AssertCondition(MJPlayer_Create(NULL, work, work_size) == NULL);
    Test_AssertCondition(MJPlayer_Create(&config, NULL, work_size) == NULL);
    Test_AssertCondition(MJPlayer_Create(&config, work, -1) == NULL);
    /* ワークサイズ不足 */
    Test_AssertCondition(MJPlayer_Create(&config, work, work_size - 1) == NULL);

    /* インターフェースがNULL */
    config.player_interface = NULL;
    Test_AssertCondition(MJPlayer_Create(&config, NULL, 0) == NULL);
    Test_AssertCondition(MJPlayer_Create(&config, work, work_size) == NULL);
    config.player_interface = MJPlayerTsumogiri_GetInterface();

    free(work);
  }

}

void MJPlayerTest_Setup(void)
{
  struct TestSuite *suite
    = Test_AddTestSuite("MJ Player Test Suite",
        NULL, MJPlayerTest_Initialize, MJPlayerTest_Finalize);

  Test_AddTest(suite, MJPlayerTest_CreateDestroyTest);
}

