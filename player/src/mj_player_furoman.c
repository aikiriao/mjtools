#include "mj_player_furoman.h"

#include "mj_shanten.h"
#include "mj_utility.h"

#include <stdlib.h>
#include <assert.h>

/* メモリアラインメント */
#define MJPLAYER_FUROMAN_ALIGNMENT 16

/* 向聴マン構造体 */
struct MJPlayerFuroman {
  MJWind        wind;       /* 家 */
  struct MJHand hand;       /* 手牌 */
  int32_t       rel_score;  /* 相対スコア */
  int32_t       total_rank;   /* 合計ランク */
  int64_t       total_score;  /* 合計スコア */
  const struct MJGameStateGetterInterface *game_state_getter; /* ゲーム状態取得インターフェース */
};

/* インターフェース名の取得 */
static const char *MJPlayerFuroman_GetName(const void *player, const MJPlayerInterfaceVersion5Tag *version_tag);
/* ワークサイズ計算 */
static int32_t MJPlayerFuroman_CalculateWorkSize(const struct MJPlayerConfig *config);
/* インスタンス生成 */
static void *MJPlayerFuroman_Create(const struct MJPlayerConfig *config, void *work, int32_t work_size);
/* インスタンス破棄 */
static void MJPlayerFuroman_Destroy(void *player);
/* 誰かのアクション時の対応 */
static void MJPlayerFuroman_OnAction(void *player, const struct MJPlayerAction *trigger_action, struct MJPlayerAction *action);
/* 自摸時の対応 */
static void MJPlayerFuroman_OnDiscard(void *player, MJTile draw_tile, struct MJPlayerAction *player_action);
/* 局開始時の対応 */
static void MJPlayerFuroman_OnStartHand(void *player, int32_t hand_no, MJWind player_wind);
/* 局終了時の対応 */
static void MJPlayerFuroman_OnEndHand(void *player, MJHandEndReason reason, const int32_t *score_diff);
/* ゲーム開始時の対応 */
static void MJPlayerFuroman_OnStartGame(void *player);
/* ゲーム終了時の対応 */
static void MJPlayerFuroman_OnEndGame(void *player, int32_t player_rank, int32_t player_score);

/* インターフェース定義 */
static const struct MJPlayerInterface st_tsumogiri_player_interface = {
  MJPlayerFuroman_GetName,
  MJPlayerFuroman_CalculateWorkSize,
  MJPlayerFuroman_Create,
  MJPlayerFuroman_Destroy,
  MJPlayerFuroman_OnAction,
  MJPlayerFuroman_OnDiscard,
  MJPlayerFuroman_OnStartHand,
  MJPlayerFuroman_OnEndHand,
  MJPlayerFuroman_OnStartGame,
  MJPlayerFuroman_OnEndGame,
};

/* インターフェース取得 */
const struct MJPlayerInterface *MJPlayerFuroman_GetInterface(void)
{
  return &st_tsumogiri_player_interface;
}

/* インターフェース名の取得 */
static const char *MJPlayerFuroman_GetName(const void *player, const MJPlayerInterfaceVersion5Tag *version_tag)
{
  MJUTILITY_UNUSED_ARGUMENT(player);
  MJUTILITY_UNUSED_ARGUMENT(version_tag);
  return "Furo-Man";
}

/* ワークサイズ計算 */
static int32_t MJPlayerFuroman_CalculateWorkSize(const struct MJPlayerConfig *config)
{
  /* 引数チェック */
  if (config == NULL) {
    return -1;
  } else if (config->game_state_getter_if == NULL) {
    return -1;
  }

  return sizeof(struct MJPlayerFuroman) + MJPLAYER_FUROMAN_ALIGNMENT;
}

/* インスタンス生成 */
static void *MJPlayerFuroman_Create(const struct MJPlayerConfig *config, void *work, int32_t work_size)
{
  struct MJPlayerFuroman *player;
  uint8_t *work_ptr;

  /* 引数チェック */
  if ((work == NULL) || (work_size < MJPlayerFuroman_CalculateWorkSize(config))) {
    return NULL;
  }

  /* メモリアラインメント */
  work_ptr = work;
  work_ptr = (uint8_t *)MJUTILITY_ROUND_UP((uintptr_t)work_ptr, MJPLAYER_FUROMAN_ALIGNMENT);

  /* 構造体配置 */
  player = (struct MJPlayerFuroman *)work_ptr;
  work_ptr += sizeof(struct MJPlayerFuroman);

  /* トータル情報の初期化 */
  player->total_rank = 0;
  player->total_score = 0;

  /* 状態取得インターフェース取得 */
  player->game_state_getter = config->game_state_getter_if;

  return player;
}

/* インスタンス破棄 */
static void MJPlayerFuroman_Destroy(void *player)
{
  /* 特に何もしない */
  MJUTILITY_UNUSED_ARGUMENT(player);
}

/* 誰かのアクション時の対応 */
static void MJPlayerFuroman_OnAction(void *player, const struct MJPlayerAction *trigger_action, struct MJPlayerAction *action)
{
  struct MJPlayerFuroman *furoman = (struct MJPlayerFuroman *)player;

  /* デバッグ向けにアサート */
  assert(player != NULL);
  assert(trigger_action != NULL);
  assert(action != NULL);

  assert(furoman->wind == action->player);

  /* 自分自身のアクションには反応しない */
  if (trigger_action->player == furoman->wind) {
    action->type = MJPLAYER_ACTIONTYPE_NONE;
    return;
  }

  switch (trigger_action->type) {
    case MJPLAYER_ACTIONTYPE_DISCARD:
      {
        struct MJTileCount tile_count;
        MJTile trig_tile = trigger_action->tile;
        /* 手牌取得 */
        furoman->game_state_getter->GetHand(player, furoman->wind, &furoman->hand);
        MJShanten_ConvertHandToTileCount(&furoman->hand, &tile_count);
        tile_count.count[trig_tile]++;
        /* 和了っていて得点がつくなら倒す */
        if ((MJShanten_CalculateShanten(&tile_count) == -1)
            && (furoman->game_state_getter->GetAgariScore(player, &furoman->hand, trig_tile) > 0)) {
          action->type = MJPLAYER_ACTIONTYPE_RON;
          action->tile = trig_tile;
          return;
        }
        /* 副露判定 */
        /* カン/ポン */
        if (tile_count.count[trig_tile] == 4) {
          action->type = MJPLAYER_ACTIONTYPE_MINKAN;
          action->tile = trig_tile;
          return;
        } else if (tile_count.count[trig_tile] == 3) {
          action->type = MJPLAYER_ACTIONTYPE_PUNG;
          action->tile = trig_tile;
          return;
        }
        /* チー */
        /* TODO: 判定辛いっすね... */
        if (MJTILE_IS_SUHAI(trig_tile)) {
          if (((furoman->wind == MJWIND_TON) && (trigger_action->player == MJWIND_PEE))
              || ((furoman->wind == MJWIND_NAN) && (trigger_action->player == MJWIND_TON))
              || ((furoman->wind == MJWIND_SHA) && (trigger_action->player == MJWIND_NAN))
              || ((furoman->wind == MJWIND_PEE) && (trigger_action->player == MJWIND_SHA))) {
            if (MJTILE_NUMBER_IS(trig_tile, 1)) {
              if ((tile_count.count[trig_tile + 1] > 0) && (tile_count.count[trig_tile + 2] > 0)) {
                action->type = MJPLAYER_ACTIONTYPE_CHOW1;
                action->tile = trig_tile;
                return;
              }
            } else if (MJTILE_NUMBER_IS(trig_tile, 2)) {
              if ((tile_count.count[trig_tile + 1] > 0) && (tile_count.count[trig_tile + 2] > 0)) {
                action->type = MJPLAYER_ACTIONTYPE_CHOW1;
                action->tile = trig_tile;
                return;
              } else if ((tile_count.count[trig_tile - 1] > 0) && (tile_count.count[trig_tile + 1] > 0)) {
                action->type = MJPLAYER_ACTIONTYPE_CHOW2;
                action->tile = trig_tile;
                return;
              }
            } else if (MJTILE_NUMBER_IS(trig_tile, 8)) {
              if ((tile_count.count[trig_tile - 2] > 0) && (tile_count.count[trig_tile - 1] > 0)) {
                action->type = MJPLAYER_ACTIONTYPE_CHOW3;
                action->tile = trig_tile;
                return;
              } else if ((tile_count.count[trig_tile - 1] > 0) && (tile_count.count[trig_tile + 1] > 0)) {
                action->type = MJPLAYER_ACTIONTYPE_CHOW2;
                action->tile = trig_tile;
                return;
              }
            } else if (MJTILE_NUMBER_IS(trig_tile, 9)) {
              if ((tile_count.count[trig_tile - 2] > 0) && (tile_count.count[trig_tile - 1] > 0)) {
                action->type = MJPLAYER_ACTIONTYPE_CHOW3;
                action->tile = trig_tile;
                return;
              }
            } else {
              if ((tile_count.count[trig_tile + 1] > 0) && (tile_count.count[trig_tile + 2] > 0)) {
                action->type = MJPLAYER_ACTIONTYPE_CHOW1;
                action->tile = trig_tile;
                return;
              } else if ((tile_count.count[trig_tile - 1] > 0) && (tile_count.count[trig_tile + 1] > 0)) {
                action->type = MJPLAYER_ACTIONTYPE_CHOW2;
                action->tile = trig_tile;
                return;
              } else if ((tile_count.count[trig_tile - 2] > 0) && (tile_count.count[trig_tile - 1] > 0)) {
                action->type = MJPLAYER_ACTIONTYPE_CHOW3;
                action->tile = trig_tile;
                return;
              }
            }
          }
        }
      }
      break;
    default: break;
  }

  /* 何もしない */
  action->type = MJPLAYER_ACTIONTYPE_NONE;
}

/* 自摸時の対応 */
static void MJPlayerFuroman_OnDiscard(void *player, MJTile draw_tile, struct MJPlayerAction *player_action)
{
  int32_t t, min_shanten;
  MJTile min_tile;
  struct MJPlayerFuroman *furoman = (struct MJPlayerFuroman *)player;
  struct MJTileCount tile_count;

  assert(player != NULL);
  assert(player_action != NULL);

  assert(furoman->wind == player_action->player);

  /* 手牌取得 */
  furoman->game_state_getter->GetHand(player, furoman->wind, &furoman->hand);

  /* 手牌をカウントに直し、自摸牌を加える */
  MJShanten_ConvertHandToTileCount(&furoman->hand, &tile_count);
  if (draw_tile != MJTILE_INVALID) {
    tile_count.count[draw_tile]++;
  }

  /* 自摸和了（和了っていたら） */
  if ((MJShanten_CalculateShanten(&tile_count) == -1)
      && (furoman->game_state_getter->GetAgariScore(player, &furoman->hand, draw_tile) > 0)) {
    player_action->type = MJPLAYER_ACTIONTYPE_TSUMO;
    player_action->tile = draw_tile;
    return;
  }

  /* 捨て牌選択: 1枚ずつ牌を抜き向聴数が最も低い牌姿を選ぶ */
  min_shanten = 9;
  /* 字牌を優先して捨てにかかるため後ろから */
  for (t = MJTILE_MAX - 1; t > 0; t--) {
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

  /* 普通の捨て牌処理 */
  player_action->type = MJPLAYER_ACTIONTYPE_DISCARD;
  player_action->tile = min_tile;
}

/* 局開始時の対応 */
static void MJPlayerFuroman_OnStartHand(void *player, int32_t hand_no, MJWind player_wind)
{
  struct MJPlayerFuroman *furoman = (struct MJPlayerFuroman *)player;

  MJUTILITY_UNUSED_ARGUMENT(hand_no);

  assert(player != NULL);

  furoman->wind = player_wind;
}

/* 局終了時の対応 */
static void MJPlayerFuroman_OnEndHand(void *player, MJHandEndReason reason, const int32_t *score_diff)
{
  struct MJPlayerFuroman *furoman = (struct MJPlayerFuroman *)player;

  MJUTILITY_UNUSED_ARGUMENT(reason);

  assert(player != NULL);

  /* 自分の相対スコア変動を記録 */
  furoman->rel_score += score_diff[furoman->wind];
}

/* ゲーム開始時の対応 */
static void MJPlayerFuroman_OnStartGame(void *player)
{
  struct MJPlayerFuroman *furoman = (struct MJPlayerFuroman *)player;

  /* 相対スコアを0に初期化 */
  furoman->rel_score = 0;
}

/* ゲーム終了時の対応 */
static void MJPlayerFuroman_OnEndGame(void *player, int32_t player_rank, int32_t player_score)
{
  struct MJPlayerFuroman *furoman = (struct MJPlayerFuroman *)player;

  assert(player != NULL);

  furoman->total_rank += player_rank;
  furoman->total_score += player_score;
}
