#include "mj_shanten.h"

#include <assert.h>

/* 向聴数計算関数型 */
typedef int32_t (*ShantenCalculatorFunction)(const struct MJTileCount *count);

/* 有効牌をリストアップできる状態か？ */
static bool MJEffectiveTile_CheckTileCount(const struct MJTileCount *count)
{
  int32_t i, sum;

  /* 引数チェック */
  assert(count != NULL);

  /* 牌総数の数え上げ */
  sum = 0;
  for (i = 0; i < MJTILE_MAX; i++) {
    sum += count->count[i];
  }

  /* 自摸中でない場合は (sum % 3) == 1 がtrueになる */
  return (((sum % 3) == 1) && (sum >= 1) && (sum < 14));
}

/* 通常手の有効牌候補の列挙 */
static void MJEffectiveTile_SetNormalCandidateEffectiveTileFlag(const struct MJTileCount *count, bool *candidate_effective_tile_flag)
{
  int32_t t;

  /* 引数チェック */
  assert((count != NULL) && (candidate_effective_tile_flag != NULL));

  /* 結果を一旦クリア */
  for (t = 0; t < MJTILE_MAX; t++) {
    candidate_effective_tile_flag[t] = false;
  }

  for (t = 0; t < MJTILE_MAX; t++) {
    if (count->count[t] == 0) {
      continue;
    }
    /* 2枚目以上になるときは有効牌候補 */
    candidate_effective_tile_flag[t] = true;
    /* 面子を構成しうる牌を候補に入れる */
    if (MJTILE_IS_SUHAI(t)) {
      if (MJTILE_NUMBER_IS(t, 1)) {
          candidate_effective_tile_flag[t + 1]
          = candidate_effective_tile_flag[t + 2] = true;
      } else if (MJTILE_NUMBER_IS(t, 2)) {
        candidate_effective_tile_flag[t - 1]
          = candidate_effective_tile_flag[t + 1]
          = candidate_effective_tile_flag[t + 2] = true;
      } else if (MJTILE_NUMBER_IS(t, 8)) {
        candidate_effective_tile_flag[t - 2]
          = candidate_effective_tile_flag[t - 1]
          = candidate_effective_tile_flag[t + 1] = true;
      } else if (MJTILE_NUMBER_IS(t, 9)) {
        candidate_effective_tile_flag[t - 2]
          = candidate_effective_tile_flag[t - 1] = true;
      } else {
        candidate_effective_tile_flag[t - 2]
          = candidate_effective_tile_flag[t - 1]
          = candidate_effective_tile_flag[t + 1]
          = candidate_effective_tile_flag[t + 2] = true;
      }
    }
  }
}

/* 七対子手の有効牌候補の列挙 */
static void MJEffectiveTile_SetChitoitsuCandidateEffectiveTileFlag(const struct MJTileCount *count, bool *candidate_effective_tile_flag)
{
  int32_t t;

  /* 引数チェック */
  assert((count != NULL) && (candidate_effective_tile_flag != NULL));

  /* 結果を一旦クリア */
  for (t = 0; t < MJTILE_MAX; t++) {
    candidate_effective_tile_flag[t] = false;
  }

  for (t = 0; t < MJTILE_MAX; t++) {
    if (count->count[t] == 0) {
      continue;
    }
    /* 2枚目以上になるときは有効牌候補 */
    candidate_effective_tile_flag[t] = true;
  }
}

/* 国士無双手の有効牌候補の列挙 */
static void MJEffectiveTile_SetKokushimusouCandidateEffectiveTileFlag(const struct MJTileCount *count, bool *candidate_effective_tile_flag)
{
  int32_t t;

  /* 引数チェック */
  assert((count != NULL) && (candidate_effective_tile_flag != NULL));

  /* 結果を一旦クリア */
  for (t = 0; t < MJTILE_MAX; t++) {
    candidate_effective_tile_flag[t] = false;
  }

  /* 么九牌をマーク */
  candidate_effective_tile_flag[MJTILE_1MAN]
    = candidate_effective_tile_flag[MJTILE_9MAN]
    = candidate_effective_tile_flag[MJTILE_1PIN]
    = candidate_effective_tile_flag[MJTILE_9PIN]
    = candidate_effective_tile_flag[MJTILE_1SOU]
    = candidate_effective_tile_flag[MJTILE_9SOU]
    = candidate_effective_tile_flag[MJTILE_TON]
    = candidate_effective_tile_flag[MJTILE_NAN]
    = candidate_effective_tile_flag[MJTILE_SHA]
    = candidate_effective_tile_flag[MJTILE_PEE]
    = candidate_effective_tile_flag[MJTILE_HAKU]
    = candidate_effective_tile_flag[MJTILE_HATU]
    = candidate_effective_tile_flag[MJTILE_CHUN]
    = true;
}

/* 有効牌リストアップ処理コア */
static MJEffectiveTileApiResult MJEffectiveTile_GetEffectiveTilesCore(const struct MJTileCount *count, const bool *candidate_effective_tile_flag, ShantenCalculatorFunction shanten_calculator, struct MJEffectiveTiles *effective_tiles)
{
  int32_t t, current_shanten;
  struct MJTileCount count_work;
  struct MJEffectiveTiles tmp;

  /* 引数チェック: 呼び出し側で済ませているものとして、無効な場合は落とす */
  assert ((count != NULL) && (candidate_effective_tile_flag != NULL)
      && (shanten_calculator != NULL) && (effective_tiles != NULL));

  /* 有効牌がリストアップできる状態か確認 */
  if (!MJEffectiveTile_CheckTileCount(count)) {
    return MJEFFECTIVETILE_APIRESULT_INVALID_NUM_TILES;
  }

  /* 作業領域にカウントをコピー */
  memcpy(&count_work, count, sizeof(struct MJTileCount));
  
  /* 有効牌リストをクリア */
  for (t = 0; t < MJTILE_MAX; t++) {
    tmp.tiles[t] = MJTILE_INVALID;
  }
  tmp.num_effective_tiles = 0;

  /* 現在の向聴数を計算 */
  current_shanten = shanten_calculator(count);

  /* 有効牌候補を追加して向聴数が下がるか確認 */
  for (t = 0; t < MJTILE_MAX; t++) {
    if (candidate_effective_tile_flag[t]) {
      /* 有効牌候補を1つ増やす */
      count_work.count[t]++;
      /* 向聴数が下がっていれば有効牌 */
      if (current_shanten > shanten_calculator(&count_work)) {
        tmp.tiles[tmp.num_effective_tiles] = t;
        tmp.num_effective_tiles++;
      }
      /* 増やした牌を元に戻す */
      count_work.count[t]--;
    }
  }

  /* 成功終了 */
  memcpy(effective_tiles, &tmp, sizeof(struct MJEffectiveTiles));
  return MJEFFECTIVETILE_APIRESULT_OK;
}

/* 通常手の有効牌リストアップ */
MJEffectiveTileApiResult MJEffectiveTile_GetNormalEffectiveTiles(const struct MJTileCount *count, struct MJEffectiveTiles *effective_tiles)
{
  bool candidate_effective_tile_flag[MJTILE_MAX];

  /* 引数チェック */
  if ((count == NULL) || (effective_tiles == NULL)) {
    return MJEFFECTIVETILE_APIRESULT_INVALID_ARGUMENT;
  }

  /* 有効牌候補をリストアップ */
  MJEffectiveTile_SetNormalCandidateEffectiveTileFlag(count, candidate_effective_tile_flag);
  
  /* コア処理実行 */
  return MJEffectiveTile_GetEffectiveTilesCore(count,
      candidate_effective_tile_flag, MJShanten_CalculateNormalShanten, effective_tiles);
}

/* 七対子手の有効牌リストアップ */
MJEffectiveTileApiResult MJEffectiveTile_GetChitoitsuEffectiveTiles(const struct MJTileCount *count, struct MJEffectiveTiles *effective_tiles)
{
  bool candidate_effective_tile_flag[MJTILE_MAX];

  /* 引数チェック */
  if ((count == NULL) || (effective_tiles == NULL)) {
    return MJEFFECTIVETILE_APIRESULT_INVALID_ARGUMENT;
  }

  /* 有効牌候補をリストアップ */
  MJEffectiveTile_SetChitoitsuCandidateEffectiveTileFlag(count, candidate_effective_tile_flag);
  
  /* コア処理実行 */
  return MJEffectiveTile_GetEffectiveTilesCore(count,
      candidate_effective_tile_flag, MJShanten_CalculateChitoitsuShanten, effective_tiles);
}

/* 国士無双手の有効牌リストアップ */
MJEffectiveTileApiResult MJEffectiveTile_GetKokushimusouEffectiveTiles(const struct MJTileCount *count, struct MJEffectiveTiles *effective_tiles)
{
  bool candidate_effective_tile_flag[MJTILE_MAX];

  /* 引数チェック */
  if ((count == NULL) || (effective_tiles == NULL)) {
    return MJEFFECTIVETILE_APIRESULT_INVALID_ARGUMENT;
  }

  /* 有効牌候補をリストアップ */
  MJEffectiveTile_SetKokushimusouCandidateEffectiveTileFlag(count, candidate_effective_tile_flag);
  
  /* コア処理実行 */
  return MJEffectiveTile_GetEffectiveTilesCore(count,
      candidate_effective_tile_flag, MJShanten_CalculateKokushimusouShanten, effective_tiles);
}
