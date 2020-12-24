#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

/* テスト対象のモジュール */
#include "../mj_player_wrapper.c"

/* 仮のプレーヤーインターフェースにツモ切りくんを使用 */
#include "../mj_player_tsumogiri.h"

/* ゲーム状態取得関数のスタブ */
static uint32_t MJGameStateGetterStub_GetVersion(const MJGameStateGetterInterfaceVersion1Tag *version_tag)
{
  MJUTILITY_UNUSED_ARGUMENT(version_tag);
  return MJGAMESTATEGETTER_INTERFACE_VERSION;
}
static void MJGameStateGetterStub_GetHand(MJWind player, struct MJHand *hand)
{
  MJUTILITY_UNUSED_ARGUMENT(player);
  MJUTILITY_UNUSED_ARGUMENT(hand);
}
static void MJGameStateGetterStub_GetRiver(MJWind player, struct MJPlayerRiver *river)
{
  MJUTILITY_UNUSED_ARGUMENT(player);
  MJUTILITY_UNUSED_ARGUMENT(river);
}
static void MJGameStateGetterStub_GetDora(struct MJDoraTile *dora)
{
  MJUTILITY_UNUSED_ARGUMENT(dora);
}
static int32_t MJGameStateGetterStub_GetScore(MJWind player)
{
  MJUTILITY_UNUSED_ARGUMENT(player);
  return 0;
}
static int32_t MJGameStateGetterStub_GetHandNumber(void)
{
  return 0;
}
static int32_t MJGameStateGetterStub_GetHonba(void)
{
  return 0;
}
static int32_t MJGameStateGetterStub_GetNumRiichibou(void)
{
  return 0;
}
static int32_t MJGameStateGetterStub_GetNumRemainTilesInDeck(void)
{
  return 0;
}
static int32_t MJGameStateGetterStub_GetNumVisibleTiles(MJTile tile)
{
  MJUTILITY_UNUSED_ARGUMENT(tile);
  return 0;
}

/* ゲーム状態取得のインターフェースのスタブ */
static const struct MJGameStateGetterInterface st_stub_game_state_if = {
  MJGameStateGetterStub_GetVersion,
  MJGameStateGetterStub_GetHand,
  MJGameStateGetterStub_GetRiver,
  MJGameStateGetterStub_GetDora,
  MJGameStateGetterStub_GetScore,
  MJGameStateGetterStub_GetHandNumber,
  MJGameStateGetterStub_GetHonba,
  MJGameStateGetterStub_GetNumRiichibou,
  MJGameStateGetterStub_GetNumRemainTilesInDeck,
  MJGameStateGetterStub_GetNumVisibleTiles
};

/* テストのセットアップ関数 */
void MJPlayerWrapperTest_Setup(void);

static int MJPlayerWrapperTest_Initialize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

static int MJPlayerWrapperTest_Finalize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

/* 生成破棄テスト */
static void MJPlayerWrapperTest_CreateDestroyTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* ワークサイズ計算テスト */
  {
    struct MJPlayerWrapperConfig config;

    /* NULL指定 */
    Test_AssertCondition(MJPlayerWrapper_CalculateWorkSize(NULL) < 0);
    /* インターフェースがNULL */
    config.player_interface = NULL;
    Test_AssertCondition(MJPlayerWrapper_CalculateWorkSize(&config) < 0);

    /* インターフェース設定 */
    config.player_interface = MJPlayerTsumogiri_GetInterface();
    Test_AssertCondition(MJPlayerWrapper_CalculateWorkSize(&config) >= 0);
  }

  /* ワーク領域渡しによる生成・破棄（コンフィグ指定） */
  {
    int32_t work_size;
    void *work;
    struct MJPlayerWrapper *player = NULL;
    struct MJPlayerWrapperConfig config;

    /* コンフィグ作成 */
    config.player_interface = MJPlayerTsumogiri_GetInterface();
    config.player_config.game_state_getter_if = &st_stub_game_state_if; 

    work_size = MJPlayerWrapper_CalculateWorkSize(&config);
    Test_AssertCondition(work_size >= 0);

    work = malloc(work_size);
    player = MJPlayerWrapper_Create(&config, work, work_size);
    Test_AssertCondition(player != NULL);
    Test_AssertEqual(player->alloced_by_own, false);
    Test_AssertCondition(player->player_instance != NULL);
    Test_AssertCondition(player->work != NULL);
    Test_AssertCondition(player->player_interface == config.player_interface);

    MJPlayerWrapper_Destroy(player);
    free(work);
  }

  /* ワーク領域自前確保による生成・破棄（コンフィグ指定） */
  {
    struct MJPlayerWrapper *player = NULL;
    struct MJPlayerWrapperConfig config;

    /* コンフィグ作成 */
    config.player_interface = MJPlayerTsumogiri_GetInterface();
    config.player_config.game_state_getter_if = &st_stub_game_state_if; 

    player = MJPlayerWrapper_Create(&config, NULL, 0);
    Test_AssertCondition(player != NULL);
    Test_AssertEqual(player->alloced_by_own, true);
    Test_AssertCondition(player->work != NULL);
    Test_AssertCondition(player->player_instance != NULL);
    Test_AssertCondition(player->player_interface == config.player_interface);

    MJPlayerWrapper_Destroy(player);
  }

  /* 作成失敗ケース */
  {
    int32_t work_size;
    void *work;
    struct MJPlayerWrapperConfig config;

    /* 正常なワークサイズとワーク領域を用意 */
    config.player_interface = MJPlayerTsumogiri_GetInterface();
    work_size = MJPlayerWrapper_CalculateWorkSize(&config);
    work = malloc(work_size);

    /* 不正な引数 */
    Test_AssertCondition(MJPlayerWrapper_Create(NULL, work, work_size) == NULL);
    Test_AssertCondition(MJPlayerWrapper_Create(&config, NULL, work_size) == NULL);
    Test_AssertCondition(MJPlayerWrapper_Create(&config, work, -1) == NULL);
    /* ワークサイズ不足 */
    Test_AssertCondition(MJPlayerWrapper_Create(&config, work, work_size - 1) == NULL);

    /* インターフェースがNULL */
    config.player_interface = NULL;
    Test_AssertCondition(MJPlayerWrapper_Create(&config, NULL, 0) == NULL);
    Test_AssertCondition(MJPlayerWrapper_Create(&config, work, work_size) == NULL);
    config.player_interface = MJPlayerTsumogiri_GetInterface();

    free(work);
  }

}

void MJPlayerWrapperTest_Setup(void)
{
  struct TestSuite *suite
    = Test_AddTestSuite("MJ Player Wrapper Test Suite",
        NULL, MJPlayerWrapperTest_Initialize, MJPlayerWrapperTest_Finalize);

  Test_AddTest(suite, MJPlayerWrapperTest_CreateDestroyTest);
}

