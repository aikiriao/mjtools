#include "mj_river.h"

#include <assert.h>
#include <string.h>

/* 初期化 */
void MJRiver_Initialize(struct MJRiver *river)
{
  int32_t i, j;
  /* 無効な捨て牌エントリ */
  static const struct MJDiscardedTile invalid_entry = {
    .tile = MJTILE_INVALID,
    .riichi = false,
    .meld_player = MJWIND_INVALID
  };

  assert(river != NULL);

  /* 河を無効値で埋める/捨てた牌数を0に */
  for (i = 0; i < 4; i++) {
    struct MJPlayerRiver *priver = &(river->player_river[i]);
    for (j = 0; j < MJTYPES_MAX_NUM_DISCARDED_TILES; j++) {
      priver->tiles[j] = invalid_entry;
    }
    priver->num_tiles = 0;
  }

  /* 最後に捨てられた牌/プレーヤー */
  river->last_discard_tile    = MJTILE_INVALID;
  river->last_discard_player  = MJWIND_INVALID;
}

/* 1枚捨てる */
MJRiverApiResult MJRiver_DiscardTile(struct MJRiver *river, MJWind player_wind, MJTile tile, bool riichi)
{
  struct MJPlayerRiver *priver;

  /* 引数チェック */
  if ((river == NULL)
      || (player_wind == MJWIND_INVALID) || !MJTILE_IS_VALID(tile)) {
    return MJRIVER_APIRESULT_INVALID_ARGUMENT;
  }

  /* プレーヤーの河取得 */
  priver = &(river->player_river[player_wind]);
  assert(priver->num_tiles <= MJTYPES_MAX_NUM_DISCARDED_TILES);

  /* 既に立直していないか？ */
  if (riichi) {
    int32_t i;
    for (i = 0; i < priver->num_tiles; i++) {
      if (priver->tiles[i].riichi) {
        return MJRIVER_APIRESULT_ALREADY_RIICHI;
      }
    }
  }

  /* 新しく捨て牌を追加 */
  priver->tiles[priver->num_tiles].tile = tile;
  priver->tiles[priver->num_tiles].riichi = riichi;
  priver->num_tiles++;

  /* 最後に捨てられた牌を更新 */
  river->last_discard_tile = tile;
  river->last_discard_player = player_wind;

  return MJRIVER_APIRESULT_OK;
}

/* ポンできるか？ */
bool MJRiver_CanPung(const struct MJRiver *river, MJWind player_wind, const struct MJHand *player_hand)
{
  /* 引数が不正 */
  if ((river == NULL) ||
      (player_wind == MJWIND_INVALID) || (player_hand == NULL)) {
    return false;
  }

  /* 捨て牌がない */
  if (!MJTILE_IS_VALID(river->last_discard_tile)) {
    return false;
  }

  /* 捨てた本人が鳴くことはできない */
  if (player_wind == river->last_discard_player) {
    return false;
  }

  /* 手牌を見てポン可能か判定 */
  {
    int32_t i;
    int32_t num_tiles = 0;

    /* 同一牌数をカウント */
    for (i = 0; i < 13; i++) {
      if (river->last_discard_tile == player_hand->hand[i]) {
        num_tiles++;
      }
    }

    /* 枚数が足らない */
    if (num_tiles < 2) {
      return false;
    }
  }

  return true;
}

/* チーできるか？ */
bool MJRiver_CanChow(const struct MJRiver *river, MJWind player_wind, const struct MJHand *player_hand)
{
  /* 引数が不正 */
  if ((river == NULL) ||
      (player_wind == MJWIND_INVALID) || (player_hand == NULL)) {
    return false;
  }

  /* 捨て牌がない */
  if (!MJTILE_IS_VALID(river->last_discard_tile)) {
    return false;
  }

  /* 捨て牌が数牌でない */
  if (!MJTILE_IS_SUHAI(river->last_discard_tile)) {
    return false;
  }

  /* 捨てた本人が鳴くことはできない */
  if (player_wind == river->last_discard_player) {
    return false;
  }

  /* チー可能な家か？（上家から鳴こうとしているか？） */
  if (!((player_wind == MJWIND_TON) && (river->last_discard_player == MJWIND_PEE))
      && !((player_wind == MJWIND_NAN) && (river->last_discard_player == MJWIND_TON))
      && !((player_wind == MJWIND_SHA) && (river->last_discard_player == MJWIND_NAN))
      && !((player_wind == MJWIND_PEE) && (river->last_discard_player == MJWIND_SHA))) {
    return false;
  }

  /* 手牌を見てチー可能か判定 */
  /* 喰い替えの考慮もいるかもしれない... */
  {
    int32_t i;
    int32_t syuntu_count[4] = { 0, 0, 0, 0 };

    /* 近隣牌の出現回数をカウント */
    for (i = 0; i < 13; i++) {
      if (!MJTILE_NUMBER_IS(river->last_discard_tile, 1)
          && (player_hand->hand[i] == (river->last_discard_tile - 2))) {
        syuntu_count[0]++;
      } else if (player_hand->hand[i] == (river->last_discard_tile - 1)) {
        syuntu_count[1]++;
      } else if (player_hand->hand[i] == (river->last_discard_tile + 1)) {
        syuntu_count[2]++;
      } else if (!MJTILE_NUMBER_IS(river->last_discard_tile, 9)
          && (player_hand->hand[i] == (river->last_discard_tile + 2))) {
        syuntu_count[3]++;
      }
    }

    /* チーの成立条件を満たしているか？ */
    if (((syuntu_count[0] > 0) && (syuntu_count[1] > 0))
        || ((syuntu_count[1] > 0) && (syuntu_count[2] > 0))
        || ((syuntu_count[2] > 0) && (syuntu_count[3] > 0))) {
      return true;
    }

  }

  return false;
}

/* カン（大明槓）できるか？ */
bool MJRiver_CanKan(const struct MJRiver *river, MJWind player_wind, const struct MJHand *player_hand)
{
  /* 引数が不正 */
  if ((river == NULL) ||
      (player_wind == MJWIND_INVALID) || (player_hand == NULL)) {
    return false;
  }

  /* 捨て牌がない */
  if (!MJTILE_IS_VALID(river->last_discard_tile)) {
    return false;
  }

  /* 捨てた本人が鳴くことはできない */
  if (player_wind == river->last_discard_player) {
    return false;
  }

  /* 手牌を見てカン可能か判定 */
  {
    int32_t i;
    int32_t num_tiles = 0;

    /* 同一牌数をカウント */
    for (i = 0; i < 13; i++) {
      if (river->last_discard_tile == player_hand->hand[i]) {
        num_tiles++;
      }
    }

    /* 枚数が足らない */
    if (num_tiles < 3) {
      return false;
    }
  }

  return true;
}

/* ポン */
MJRiverApiResult MJRiver_Pung(struct MJRiver *river, MJWind player_wind, const struct MJHand *player_hand)
{
  struct MJPlayerRiver *priver;

  /* 引数チェック */
  if ((river == NULL) || (player_hand == NULL) || (player_wind == MJWIND_INVALID)) {
    return MJRIVER_APIRESULT_INVALID_ARGUMENT;
  }

  /* 鳴けない */
  if (!MJRiver_CanPung(river, player_wind, player_hand)) {
    return MJRIVER_APIRESULT_CANNOT_MELD;
  }

  /* 鳴いたプレーヤーをセット */
  priver = &(river->player_river[river->last_discard_player]);
  assert(priver->num_tiles > 0);
  assert(river->last_discard_tile == priver->tiles[priver->num_tiles - 1].tile);
  priver->tiles[priver->num_tiles - 1].meld_player = player_wind;

  return MJRIVER_APIRESULT_OK;
}

/* チー */
MJRiverApiResult MJRiver_Chow(struct MJRiver *river, MJWind player_wind, const struct MJHand *player_hand)
{
  struct MJPlayerRiver *priver;

  /* 引数チェック */
  if ((river == NULL) || (player_hand == NULL) || (player_wind == MJWIND_INVALID)) {
    return MJRIVER_APIRESULT_INVALID_ARGUMENT;
  }

  /* 鳴けない */
  if (!MJRiver_CanChow(river, player_wind, player_hand)) {
    return MJRIVER_APIRESULT_CANNOT_MELD;
  }

  /* 鳴いたプレーヤーをセット */
  priver = &(river->player_river[river->last_discard_player]);
  assert(priver->num_tiles > 0);
  assert(river->last_discard_tile == priver->tiles[priver->num_tiles - 1].tile);
  priver->tiles[priver->num_tiles - 1].meld_player = player_wind;

  return MJRIVER_APIRESULT_OK;
}

/* カン（大明槓） */
MJRiverApiResult MJRiver_Kan(struct MJRiver *river, MJWind player_wind, const struct MJHand *player_hand)
{
  struct MJPlayerRiver *priver;

  /* 引数チェック */
  if ((river == NULL) || (player_hand == NULL) || (player_wind == MJWIND_INVALID)) {
    return MJRIVER_APIRESULT_INVALID_ARGUMENT;
  }

  /* 鳴けない */
  if (!MJRiver_CanKan(river, player_wind, player_hand)) {
    return MJRIVER_APIRESULT_CANNOT_MELD;
  }

  /* 鳴いたプレーヤーをセット */
  priver = &(river->player_river[river->last_discard_player]);
  assert(priver->num_tiles > 0);
  assert(river->last_discard_tile == priver->tiles[priver->num_tiles - 1].tile);
  priver->tiles[priver->num_tiles - 1].meld_player = player_wind;

  return MJRIVER_APIRESULT_OK;
}
