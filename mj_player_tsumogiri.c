#include "mj_player_tsumogiri.h"

#include "mj_utility.h"
#include <stdlib.h>
#include <assert.h>

/* メモリアラインメント */
#define MJPLAYER_TSUMOGIRI_ALIGNMENT 16

/* ツモ切りくん構造体 */
struct MJPlayerTsumogiri {
  MJWind        wind;       /* 家 */
  struct MJHand hand;       /* 手牌 */
  int32_t       rel_score;  /* 相対スコア */
  int32_t       total_rank;   /* 合計ランク */
  int64_t       total_score;  /* 合計スコア */
};

/* インターフェース名の取得 */
static const char *MJPlayerTsumogiri_GetName(const MJPlayerInterfaceVersion2Tag *version_tag);
/* ワークサイズ計算 */
static int32_t MJPlayerTsumogiri_CalculateWorkSize(void);
/* インスタンス生成 */
static void *MJPlayerTsumogiri_Create(void *work, int32_t work_size);
/* インスタンス破棄 */
static void MJPlayerTsumogiri_Destroy(void *player);
/* 誰かのアクション時の対応 */
static void MJPlayerTsumogiri_OnAction(void *player, MJWind trigger_player, const struct MJPlayerAction *trigger_action, MJWind action_player, struct MJPlayerAction *action);
/* 自摸時の対応 */
static void MJPlayerTsumogiri_OnDraw(void *player, const struct MJHand *hand, MJTile draw_tile, struct MJPlayerAction *player_action);
/* 局開始時の対応 */
static void MJPlayerTsumogiri_OnStartHand(void *player, int32_t hand_no, MJWind player_wind);
/* 局終了時の対応 */
static void MJPlayerTsumogiri_OnEndHand(void *player, MJHandEndReason reason, const int32_t *score_diff);
/* ゲーム開始時の対応 */
static void MJPlayerTsumogiri_OnStartGame(void *player);
/* ゲーム終了時の対応 */
static void MJPlayerTsumogiri_OnEndGame(void *player, int32_t player_rank, int32_t player_score);

/* インターフェース定義 */
static const struct MJPlayerInterface st_tsumogiri_player_interface = {
  MJPlayerTsumogiri_GetName,
  MJPlayerTsumogiri_CalculateWorkSize,
  MJPlayerTsumogiri_Create,
  MJPlayerTsumogiri_Destroy,
  MJPlayerTsumogiri_OnAction,
  MJPlayerTsumogiri_OnDraw,
  MJPlayerTsumogiri_OnStartHand,
  MJPlayerTsumogiri_OnEndHand,
  MJPlayerTsumogiri_OnStartGame,
  MJPlayerTsumogiri_OnEndGame,
};

/* インターフェース取得 */
const struct MJPlayerInterface *MJPlayerTsumogiri_GetInterface(void)
{
  return &st_tsumogiri_player_interface;
}

/* インターフェース名の取得 */
static const char *MJPlayerTsumogiri_GetName(const MJPlayerInterfaceVersion2Tag *version_tag)
{
  MJUTILITY_UNUSED_ARGUMENT(version_tag);
  return "Tsumogiri-Kun";
}

/* ワークサイズ計算 */
static int32_t MJPlayerTsumogiri_CalculateWorkSize(void)
{
  return sizeof(struct MJPlayerTsumogiri) + MJPLAYER_TSUMOGIRI_ALIGNMENT;
}

/* インスタンス生成 */
static void *MJPlayerTsumogiri_Create(void *work, int32_t work_size)
{
  struct MJPlayerTsumogiri *player;
  uint8_t *work_ptr;

  /* 引数チェック */
  if ((work == NULL) || (work_size < MJPlayerTsumogiri_CalculateWorkSize())) {
    return NULL;
  }

  /* メモリアラインメント */
  work_ptr = work;
  work_ptr = (uint8_t *)MJUTILITY_ROUND_UP((uintptr_t)work_ptr, MJPLAYER_TSUMOGIRI_ALIGNMENT);

  /* 構造体配置 */
  player = (struct MJPlayerTsumogiri *)work_ptr;
  work_ptr += sizeof(struct MJPlayerTsumogiri);

  /* トータル情報の初期化 */
  player->total_rank = 0;
  player->total_score = 0;

  return player;
}

/* インスタンス破棄 */
static void MJPlayerTsumogiri_Destroy(void *player)
{
  /* 特に何もしない */
  MJUTILITY_UNUSED_ARGUMENT(player);
}

/* 誰かのアクション時の対応 */
static void MJPlayerTsumogiri_OnAction(void *player, MJWind trigger_player, const struct MJPlayerAction *trigger_action, MJWind action_player, struct MJPlayerAction *action)
{
  MJUTILITY_UNUSED_ARGUMENT(player);
  MJUTILITY_UNUSED_ARGUMENT(trigger_player);
  MJUTILITY_UNUSED_ARGUMENT(trigger_action);
  MJUTILITY_UNUSED_ARGUMENT(action_player);
  MJUTILITY_UNUSED_ARGUMENT(action);

  /* デバッグ向けにアサート */
  assert(player != NULL);
  assert(trigger_action != NULL);
  assert(action != NULL);

  /* 何もしない */
  action->type = MJPLAYER_ACTIONTYPE_NONE;
}

/* 自摸時の対応 */
static void MJPlayerTsumogiri_OnDraw(void *player, const struct MJHand *hand, MJTile draw_tile, struct MJPlayerAction *player_action)
{
  struct MJPlayerTsumogiri *tgiri = (struct MJPlayerTsumogiri *)player;

  assert(player != NULL);
  assert(hand != NULL);
  assert(player_action != NULL);

  /* 手牌の情報取得 */
  tgiri->hand = (*hand);

  /* ツモ切り */
  player_action->type = MJPLAYER_ACTIONTYPE_DISCARD;
  player_action->tile = draw_tile;

  return;
}

/* 局開始時の対応 */
static void MJPlayerTsumogiri_OnStartHand(void *player, int32_t hand_no, MJWind player_wind)
{
  struct MJPlayerTsumogiri *tgiri = (struct MJPlayerTsumogiri *)player;

  MJUTILITY_UNUSED_ARGUMENT(hand_no);

  assert(player != NULL);

  tgiri->wind = player_wind;
}

/* 局終了時の対応 */
static void MJPlayerTsumogiri_OnEndHand(void *player, MJHandEndReason reason, const int32_t *score_diff)
{
  struct MJPlayerTsumogiri *tgiri = (struct MJPlayerTsumogiri *)player;

  MJUTILITY_UNUSED_ARGUMENT(reason);

  assert(player != NULL);

  /* 自分の相対スコア変動を記録 */
  tgiri->rel_score += score_diff[tgiri->wind];
}

/* ゲーム開始時の対応 */
static void MJPlayerTsumogiri_OnStartGame(void *player)
{
  struct MJPlayerTsumogiri *tgiri = (struct MJPlayerTsumogiri *)player;

  /* 相対スコアを0に初期化 */
  tgiri->rel_score = 0;
}

/* ゲーム終了時の対応 */
static void MJPlayerTsumogiri_OnEndGame(void *player, int32_t player_rank, int32_t player_score)
{
  struct MJPlayerTsumogiri *tgiri = (struct MJPlayerTsumogiri *)player;

  assert(player != NULL);

  tgiri->total_rank += player_rank;
  tgiri->total_score += player_score;
}