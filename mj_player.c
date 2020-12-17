#include "mj_player.h"
#include "mj_utility.h"

#include <assert.h>
#include <stdlib.h>

/* メモリアラインメント */
#define MJPLAYER_ALIGNMENT  16

/* プレーヤー構造体 */
struct MJPlayer {
  const struct MJPlayerInterface *player_interface;
  void *player_instance;
  bool alloced_by_own;
  void *work;
};

/* プレーヤーハンドル作成に必要なワークサイズ計算 */
int32_t MJPlayer_CalculateWorkSize(const struct MJPlayerConfig *config)
{
  int32_t work_size;

  /* 引数チェック */
  if ((config == NULL) || (config->player_interface == NULL)) {
    return -1;
  }

  work_size = sizeof(struct MJPlayer) + MJPLAYER_ALIGNMENT;
  work_size += config->player_interface->CalculateWorkSize();

  return work_size;
}

/* プレーヤーハンドル作成 */
struct MJPlayer *MJPlayer_Create(const struct MJPlayerConfig *config, void *work, int32_t work_size)
{
  struct MJPlayer *player;
  uint8_t *work_ptr;
  bool tmp_alloced_by_own = false;

  /* ワーク領域自前確保の場合 */
  if ((work == NULL) && (work_size == 0)) {
    work_size = MJPlayer_CalculateWorkSize(config);
    if (work_size < 0) {
      return NULL;
    }
    work = malloc((size_t)work_size);
    tmp_alloced_by_own = true;
  }

  /* 引数チェック */
  if ((work == NULL) || (work_size < MJPlayer_CalculateWorkSize(config))
      || (config == NULL) || (config->player_interface == NULL)) {
    return NULL;
  }

  /* アドレスをアラインメント */
  work_ptr = work;
  work_ptr = (uint8_t *)MJUTILITY_ROUND_UP((uintptr_t)work_ptr, MJPLAYER_ALIGNMENT);

  /* 構造体配置 */
  player = (struct MJPlayer *)work_ptr;
  work_ptr += sizeof(struct MJPlayer);

  /* ユーザ定義のプレーヤー作成 */
  {
    int32_t coreplayer_size = config->player_interface->CalculateWorkSize();
    void *player_instance = config->player_interface->Create(work_ptr, coreplayer_size);
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
void MJPlayer_Destroy(struct MJPlayer *player)
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
const char *MJPlayer_GetName(const struct MJPlayer *player)
{
  assert(player != NULL);
  return player->player_interface->GetName(NULL);
}

/* 誰かのアクション時 */
void MJPlayer_OnAction(struct MJPlayer *player, MJWind trigger_player, const struct MJPlayerAction *trigger_action, MJWind target_player, struct MJPlayerAction *player_action)
{
  assert(player != NULL);
  assert(trigger_action != NULL);
  assert(player_action != NULL);
  player->player_interface->OnAction(player->player_instance,
      trigger_player, trigger_action, target_player, player_action);
}

/* 自摸時 */
void MJPlayer_OnDraw(struct MJPlayer *player, const struct MJHand *hand, MJTile draw_tile, struct MJPlayerAction *player_action)
{
  assert(player != NULL);
  assert(hand != NULL);
  assert(player_action != NULL);
  player->player_interface->OnDraw(player->player_instance, hand, draw_tile, player_action);
}

/* 局開始時 */
void MJPlayer_OnStartHand(struct MJPlayer *player, int32_t hand_no, MJWind player_wind)
{
  assert(player != NULL);
  player->player_interface->OnStartHand(player->player_instance, hand_no, player_wind);
}

/* 局終了時 */
void MJPlayer_OnEndHand(struct MJPlayer *player, MJHandEndReason reason, const int32_t *score_diff)
{
  assert(player != NULL);
  assert(score_diff != NULL);
  player->player_interface->OnEndHand(player->player_instance, reason, score_diff);
}

/* 対戦開始時 */
void MJPlayer_OnStartGame(struct MJPlayer *player)
{
  assert(player != NULL);
  player->player_interface->OnStartGame(player->player_instance);
}

/* 対戦終了時 */
void MJPlayer_OnEndGame(struct MJPlayer *player, int32_t player_rank, int32_t player_score)
{
  assert(player != NULL);
  player->player_interface->OnEndGame(player->player_instance, player_rank, player_score);
}
