#include "mj_player_shanten.h"

#include "mj_utility.h"
#include "mj_shanten.h"

#include <stdlib.h>
#include <assert.h>

/* メモリアラインメント */
#define MJPLAYER_SHANTEN_ALIGNMENT 16

/* 向聴マン構造体 */
struct MJPlayerShanten {
  MJWind        wind;       /* 家 */
  struct MJHand hand;       /* 手牌 */
  int32_t       rel_score;  /* 相対スコア */
  int32_t       total_rank;   /* 合計ランク */
  int64_t       total_score;  /* 合計スコア */
  const struct MJGameStateGetterInterface *game_state_getter; /* ゲーム状態取得インターフェース */
};

/* インターフェース名の取得 */
static const char *MJPlayerShanten_GetName(const MJPlayerInterfaceVersion3Tag *version_tag);
/* ワークサイズ計算 */
static int32_t MJPlayerShanten_CalculateWorkSize(const struct MJPlayerConfig *config);
/* インスタンス生成 */
static void *MJPlayerShanten_Create(const struct MJPlayerConfig *config, void *work, int32_t work_size);
/* インスタンス破棄 */
static void MJPlayerShanten_Destroy(void *player);
/* 誰かのアクション時の対応 */
static void MJPlayerShanten_OnAction(void *player, MJWind trigger_player, const struct MJPlayerAction *trigger_action, MJWind action_player, struct MJPlayerAction *action);
/* 自摸時の対応 */
static void MJPlayerShanten_OnDraw(void *player, MJTile draw_tile, struct MJPlayerAction *player_action);
/* 局開始時の対応 */
static void MJPlayerShanten_OnStartHand(void *player, int32_t hand_no, MJWind player_wind);
/* 局終了時の対応 */
static void MJPlayerShanten_OnEndHand(void *player, MJHandEndReason reason, const int32_t *score_diff);
/* ゲーム開始時の対応 */
static void MJPlayerShanten_OnStartGame(void *player);
/* ゲーム終了時の対応 */
static void MJPlayerShanten_OnEndGame(void *player, int32_t player_rank, int32_t player_score);

/* インターフェース定義 */
static const struct MJPlayerInterface st_tsumogiri_player_interface = {
  MJPlayerShanten_GetName,
  MJPlayerShanten_CalculateWorkSize,
  MJPlayerShanten_Create,
  MJPlayerShanten_Destroy,
  MJPlayerShanten_OnAction,
  MJPlayerShanten_OnDraw,
  MJPlayerShanten_OnStartHand,
  MJPlayerShanten_OnEndHand,
  MJPlayerShanten_OnStartGame,
  MJPlayerShanten_OnEndGame,
};

/* インターフェース取得 */
const struct MJPlayerInterface *MJPlayerShanten_GetInterface(void)
{
  return &st_tsumogiri_player_interface;
}

/* インターフェース名の取得 */
static const char *MJPlayerShanten_GetName(const MJPlayerInterfaceVersion3Tag *version_tag)
{
  MJUTILITY_UNUSED_ARGUMENT(version_tag);
  return "Shanten-Man";
}

/* ワークサイズ計算 */
static int32_t MJPlayerShanten_CalculateWorkSize(const struct MJPlayerConfig *config)
{
  /* 引数チェック */
  if (config == NULL) {
    return -1;
  } else if (config->game_state_getter_if == NULL) {
    return -1;
  }

  return sizeof(struct MJPlayerShanten) + MJPLAYER_SHANTEN_ALIGNMENT;
}

/* インスタンス生成 */
static void *MJPlayerShanten_Create(const struct MJPlayerConfig *config, void *work, int32_t work_size)
{
  struct MJPlayerShanten *player;
  uint8_t *work_ptr;

  /* 引数チェック */
  if ((work == NULL) || (work_size < MJPlayerShanten_CalculateWorkSize(config))) {
    return NULL;
  }

  /* メモリアラインメント */
  work_ptr = work;
  work_ptr = (uint8_t *)MJUTILITY_ROUND_UP((uintptr_t)work_ptr, MJPLAYER_SHANTEN_ALIGNMENT);

  /* 構造体配置 */
  player = (struct MJPlayerShanten *)work_ptr;
  work_ptr += sizeof(struct MJPlayerShanten);

  /* トータル情報の初期化 */
  player->total_rank = 0;
  player->total_score = 0;

  /* 状態取得インターフェース取得 */
  player->game_state_getter = config->game_state_getter_if;

  return player;
}

/* インスタンス破棄 */
static void MJPlayerShanten_Destroy(void *player)
{
  /* 特に何もしない */
  MJUTILITY_UNUSED_ARGUMENT(player);
}

/* 誰かのアクション時の対応 */
static void MJPlayerShanten_OnAction(void *player,
    MJWind trigger_player, const struct MJPlayerAction *trigger_action,
    MJWind action_player, struct MJPlayerAction *action)
{
  struct MJPlayerShanten *shan = (struct MJPlayerShanten *)player;

  MJUTILITY_UNUSED_ARGUMENT(trigger_player);
  MJUTILITY_UNUSED_ARGUMENT(action_player);
  MJUTILITY_UNUSED_ARGUMENT(action);

  /* デバッグ向けにアサート */
  assert(player != NULL);
  assert(trigger_action != NULL);
  assert(action != NULL);

  /* 自分自身のアクションには反応しない */
  if (action_player == shan->wind) {
    action->type = MJPLAYER_ACTIONTYPE_NONE;
    return;
  }

  switch (trigger_action->type) {
    case MJPLAYER_ACTIONTYPE_DISCARD:
      {
        struct MJTileCount tile_count;
        /* 手牌取得 */
        shan->game_state_getter->GetHand(player, shan->wind, &shan->hand);
        MJShanten_ConvertHandToTileCount(&shan->hand, &tile_count);
        tile_count.count[trigger_action->tile]++;
        /* 和了っていたらチョンボ気にせず倒す */
        if (MJShanten_CalculateShanten(&tile_count) == -1) {
          action->type = MJPLAYER_ACTIONTYPE_RON;
          action->tile = trigger_action->tile;
          return;
        }
      }
      break;
    default:
      /* 何もしない */
      action->type = MJPLAYER_ACTIONTYPE_NONE;
      return;
  }
}

/* 自摸時の対応 */
static void MJPlayerShanten_OnDraw(void *player, MJTile draw_tile, struct MJPlayerAction *player_action)
{
  int32_t t, min_shanten;
  MJTile min_tile;
  struct MJPlayerShanten *shan = (struct MJPlayerShanten *)player;
  struct MJTileCount tile_count;

  assert(player != NULL);
  assert(player_action != NULL);

  /* 手牌取得 */
  shan->game_state_getter->GetHand(player, shan->wind, &shan->hand);

  /* カウントに直し、自摸牌を加える */
  MJShanten_ConvertHandToTileCount(&shan->hand, &tile_count);
  tile_count.count[draw_tile]++;

  /* 自摸和了 */
  if (MJShanten_CalculateShanten(&tile_count) == -1) {
    player_action->type = MJPLAYER_ACTIONTYPE_TSUMO;
    player_action->tile = draw_tile;
    return;
  }

  /* 捨て牌選択: 1枚ずつ牌を抜き向聴数が最も低い牌姿を選ぶ */
  min_shanten = 8;
  min_tile = draw_tile;
  for (t = 0; t < MJTILE_MAX; t++) {
    if (tile_count.count[t] > 0) {
      int32_t shanten;
      tile_count.count[t]--;
      shanten = MJShanten_CalculateShanten(&tile_count);
      tile_count.count[t]++;
      if (shanten < min_shanten) {
        min_shanten = shanten;
        min_tile = (MJTile)t;
      }
    }
  }

  player_action->type = MJPLAYER_ACTIONTYPE_DISCARD;
  player_action->tile = min_tile;
}

/* 局開始時の対応 */
static void MJPlayerShanten_OnStartHand(void *player, int32_t hand_no, MJWind player_wind)
{
  struct MJPlayerShanten *shan = (struct MJPlayerShanten *)player;

  MJUTILITY_UNUSED_ARGUMENT(hand_no);

  assert(player != NULL);

  shan->wind = player_wind;
}

/* 局終了時の対応 */
static void MJPlayerShanten_OnEndHand(void *player, MJHandEndReason reason, const int32_t *score_diff)
{
  struct MJPlayerShanten *shan = (struct MJPlayerShanten *)player;

  MJUTILITY_UNUSED_ARGUMENT(reason);

  assert(player != NULL);

  /* 自分の相対スコア変動を記録 */
  shan->rel_score += score_diff[shan->wind];
}

/* ゲーム開始時の対応 */
static void MJPlayerShanten_OnStartGame(void *player)
{
  struct MJPlayerShanten *shan = (struct MJPlayerShanten *)player;

  /* 相対スコアを0に初期化 */
  shan->rel_score = 0;
}

/* ゲーム終了時の対応 */
static void MJPlayerShanten_OnEndGame(void *player, int32_t player_rank, int32_t player_score)
{
  struct MJPlayerShanten *shan = (struct MJPlayerShanten *)player;

  assert(player != NULL);

  shan->total_rank += player_rank;
  shan->total_score += player_score;
}