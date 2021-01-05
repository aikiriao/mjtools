#include "mj_player_wrapper.h"
#include "mj_utility.h"

#include <assert.h>
#include <stdlib.h>

/* メモリアラインメント */
#define MJPLAYER_ALIGNMENT  16

/* プレーヤー構造体 */
struct MJPlayerWrapper {
  const struct MJPlayerInterface *player_interface;
  void *player_instance;
  bool alloced_by_own;
  void *work;
};

/* プレーヤーハンドル作成に必要なワークサイズ計算 */
int32_t MJPlayerWrapper_CalculateWorkSize(const struct MJPlayerWrapperConfig *config)
{
  int32_t ret, work_size;

  /* 引数チェック */
  if ((config == NULL) || (config->player_interface == NULL)) {
    return -1;
  }

  /* ラッパー構造体分 */
  work_size = sizeof(struct MJPlayerWrapper) + MJPLAYER_ALIGNMENT;

  /* ユーザ定義分 */
  if ((ret = config->player_interface->CalculateWorkSize(&config->player_config)) < 0) {
    return -1;
  }
  work_size += ret;

  return work_size;
}

/* プレーヤーハンドル作成 */
struct MJPlayerWrapper *MJPlayerWrapper_Create(const struct MJPlayerWrapperConfig *config, void *work, int32_t work_size)
{
  struct MJPlayerWrapper *player;
  uint8_t *work_ptr;
  bool tmp_alloced_by_own = false;

  /* ワーク領域自前確保の場合 */
  if ((work == NULL) && (work_size == 0)) {
    work_size = MJPlayerWrapper_CalculateWorkSize(config);
    if (work_size < 0) {
      return NULL;
    }
    work = malloc((size_t)work_size);
    tmp_alloced_by_own = true;
  }

  /* 引数チェック */
  if ((work == NULL) || (work_size < MJPlayerWrapper_CalculateWorkSize(config))
      || (config == NULL) || (config->player_interface == NULL)) {
    return NULL;
  }

  /* アドレスをアラインメント */
  work_ptr = work;
  work_ptr = (uint8_t *)MJUTILITY_ROUND_UP((uintptr_t)work_ptr, MJPLAYER_ALIGNMENT);

  /* 構造体配置 */
  player = (struct MJPlayerWrapper *)work_ptr;
  work_ptr += sizeof(struct MJPlayerWrapper);

  /* ユーザ定義のプレーヤー作成 */
  {
    int32_t coreplayer_size = config->player_interface->CalculateWorkSize(&config->player_config);
    void *player_instance = config->player_interface->Create(&config->player_config, work_ptr, coreplayer_size);
    if (player_instance == NULL) {
      return NULL;
    }
    player->player_instance = player_instance;
    player->player_interface = config->player_interface;
    work_ptr += coreplayer_size;
  }

  player->work = work;
  player->alloced_by_own = tmp_alloced_by_own;

  /* バッファオーバーランチェック */
  assert((work_ptr - (uint8_t *)work) <= work_size);

  return player;
}

/* プレーヤーハンドル破棄 */
void MJPlayerWrapper_Destroy(struct MJPlayerWrapper *player)
{
  if (player != NULL) {
    /* インスタンス破棄 */
    player->player_interface->Destroy(player->player_instance);
    player->player_instance = NULL;
    /* 自己割当てならば領域解放 */
    if (player->alloced_by_own) {
      free(player->work);
    }
  }
}

/* インターフェース名取得 */
const char *MJPlayerWrapper_GetName(const struct MJPlayerWrapper *player)
{
  assert(player != NULL);
  return player->player_interface->GetName(NULL);
}

/* 誰かのアクション時 */
void MJPlayerWrapper_OnAction(struct MJPlayerWrapper *player, const struct MJPlayerAction *trigger_action, struct MJPlayerAction *action)
{
  assert(player != NULL);
  assert(trigger_action != NULL);
  assert(action != NULL);
  player->player_interface->OnAction(player->player_instance, trigger_action, action);
}

/* 捨て牌時 */
void MJPlayerWrapper_OnDiscard(struct MJPlayerWrapper *player, MJTile draw_tile, struct MJPlayerAction *player_action)
{
  assert(player != NULL);
  assert(player_action != NULL);
  player->player_interface->OnDiscard(player->player_instance, draw_tile, player_action);
}

/* 局開始時 */
void MJPlayerWrapper_OnStartHand(struct MJPlayerWrapper *player, int32_t hand_no, MJWind player_wind)
{
  assert(player != NULL);
  player->player_interface->OnStartHand(player->player_instance, hand_no, player_wind);
}

/* 局終了時 */
void MJPlayerWrapper_OnEndHand(struct MJPlayerWrapper *player, MJHandEndReason reason, const int32_t *score_diff)
{
  assert(player != NULL);
  assert(score_diff != NULL);
  player->player_interface->OnEndHand(player->player_instance, reason, score_diff);
}

/* 対戦開始時 */
void MJPlayerWrapper_OnStartGame(struct MJPlayerWrapper *player)
{
  assert(player != NULL);
  player->player_interface->OnStartGame(player->player_instance);
}

/* 対戦終了時 */
void MJPlayerWrapper_OnEndGame(struct MJPlayerWrapper *player, int32_t player_rank, int32_t player_score)
{
  assert(player != NULL);
  player->player_interface->OnEndGame(player->player_instance, player_rank, player_score);
}

/* プレーヤーハンドル取得 */
const void *MJPlayerWrapper_GetPlayerHandle(const struct MJPlayerWrapper *player)
{
  assert(player != NULL);
  return player->player_instance;
}
