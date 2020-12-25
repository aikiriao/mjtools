#include "mj_shanten.h"
#include "mj_utility.h"

#include <string.h>
#include <assert.h>

/* 向聴数計算関数型 */
typedef int32_t (*ShantenCalculatorFunction)(const struct MJTileCount *count);

/* ナイーブな（バックトラック法を使用した）向聴数の計算コア処理 */
static int32_t MJShanten_CalculateNormalShantenNaive(struct MJTileCount *count);
/* 面子を含めた向聴数計算 */
static void MJShanten_CalculateNormalShantenMentsu(
    struct MJTileCount *count, int32_t pos, 
    int32_t num_mentsu, int32_t *min_shanten);
/* 搭子（面子候補）を含めた向聴数計算 */
static void MJShanten_CalculateNormalShantenTatsu(
    struct MJTileCount *count, int32_t pos,
    int32_t num_mentsu, int32_t num_tatsu, int32_t *min_shanten);
/* 有効牌をリストアップできる状態か？ */
static bool MJEffectiveTile_CheckTileCount(const struct MJTileCount *count);
/* 通常手の有効牌候補の列挙 */
static void MJEffectiveTile_SetNormalCandidateEffectiveTileFlag(const struct MJTileCount *count, bool *candidate_effective_tile_flag);
/* 七対子手の有効牌候補の列挙 */
static void MJEffectiveTile_SetChitoitsuCandidateEffectiveTileFlag(const struct MJTileCount *count, bool *candidate_effective_tile_flag);
/* 国士無双手の有効牌候補の列挙 */
static void MJEffectiveTile_SetKokushimusouCandidateEffectiveTileFlag(const struct MJTileCount *count, bool *candidate_effective_tile_flag);
/* 有効牌リストアップ処理コア */
static MJEffectiveTileApiResult MJEffectiveTile_GetEffectiveTilesCore(const struct MJTileCount *count,
    const bool *candidate_effective_tile_flag, ShantenCalculatorFunction shanten_calculator, struct MJEffectiveTiles *effective_tiles);

/* 手牌（副露含む）をカウントに変換 */
void MJShanten_ConvertHandToTileCount(const struct MJHand *hand, struct MJTileCount *tile_count)
{
  int32_t i;
  struct MJTileCount tmp;
  int32_t *count;
  const struct MJMeld *pmeld;

  assert((hand != NULL) && (tile_count != NULL));

  /* カウントを0クリア */
  memset(&tmp, 0, sizeof(struct MJTileCount));
  count = &(tmp.count[0]);

  /* 副露以外をカウント */
  for (i = 0; i < 13; i++) {
    if (MJTILE_IS_VALID(hand->hand[i])) {
      count[hand->hand[i]]++;
    }
  }
  
  /* 副露牌をカウント */
  for (i = 0; i < hand->num_meld; i++) {
    pmeld = &(hand->meld[i]);
    switch (pmeld->type) {
      case MJMELD_TYPE_CHOW:
        count[pmeld->min_tile]++; count[pmeld->min_tile + 1]++; count[pmeld->min_tile + 2]++;
        break;
      case MJMELD_TYPE_PUNG:
        count[pmeld->min_tile] += 3;
        break;
      case MJMELD_TYPE_ANKAN:
      case MJMELD_TYPE_MINKAN:
      case MJMELD_TYPE_KAKAN:
        count[pmeld->min_tile] += 4;
        break;
      default:
        assert(0);
    }
  }

  /* 成功終了 */
  (*tile_count) = tmp;
}

/* 七対子の向聴数を計算 */
int32_t MJShanten_CalculateChitoitsuShanten(const struct MJTileCount *count)
{
  int32_t i, shanten, num_toitu, num_types;
  const int32_t *cnt;

  /* 引数チェック */
  assert(count != NULL);

  cnt = &count->count[0];

  /* 対子を数える */
  num_toitu = num_types = 0;
  for (i = 0; i < MJTILE_MAX; i++) {
    if (cnt[i] == 0) {
      continue; 
    }
    num_types++;
    if (cnt[i] >= 2) {
      num_toitu++;
    }
  }
  
  /* 基本は(6-対子数) */
  shanten = 6 - num_toitu;

  /* 3枚持ち以上がある場合は7種より小さくなる */
  /* 3枚持ち以上の面子は無効のため、その分向聴数を増やす */
  if (num_types < 7) {
    shanten += (7 - num_types);
  }

  return shanten;
}

/* 国士無双の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJShanten_CalculateKokushimusouShanten(const struct MJTileCount *count)
{
  int32_t i, shanten, head;
  const int32_t *cnt;

  /* 引数チェック */
  assert(count != NULL);

  cnt = &count->count[0];

  /* 么九牌を数える */
  shanten = 13; head = 0;
  for (i = 0; i < MJTILE_MAX; i++) {
    if (MJTILE_IS_YAOCHU(i) && (cnt[i] > 0)) {
      shanten--;
      /* 頭をマーク */
      if ((cnt[i] >= 2) && (head == 0)) {
        head = 1;
      }
    }
  }

  /* 頭があるぶん減らす */
  shanten -= head;

  return shanten;
}

/* 通常手の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJShanten_CalculateNormalShanten(const struct MJTileCount *count)
{
  struct MJTileCount tmp;
  int32_t i, shanten, min_shanten;

  /* 引数チェック */
  assert(count != NULL);

  /* 作業用に手牌をコピー */
  tmp = (*count);

  min_shanten = 8;
  for (i = 0; i < MJTILE_MAX; i++) {
    /* 頭を抜いて調べる */
    if (tmp.count[i] >= 2) {            
      tmp.count[i] -= 2;
      /* 頭を抜いている分1減らす */
      shanten = MJShanten_CalculateNormalShantenNaive(&tmp) - 1;
      if (shanten < min_shanten) { min_shanten = shanten; }
      tmp.count[i] += 2;
    }
  }

  /* 雀頭がない場合を含めるため、頭を抜かずに調べる */
  shanten = MJShanten_CalculateNormalShantenNaive(&tmp);
  if (shanten < min_shanten) { min_shanten = shanten; }

  /* 最終結果 */
  return min_shanten;
}

/* ナイーブな向聴数の計算処理 */
static int32_t MJShanten_CalculateNormalShantenNaive(struct MJTileCount *count)
{
  int32_t min_shanten;

  /* 向聴数の最大値をセットして再帰計算開始 */
  min_shanten = 8;
  MJShanten_CalculateNormalShantenMentsu(count, 0, 0, &min_shanten);

  return min_shanten;
}

/* 面子を含めた向聴数計算 */
static void MJShanten_CalculateNormalShantenMentsu(
    struct MJTileCount *count, int32_t pos,
    int32_t num_mentsu, int32_t *min_shanten)
{
  int32_t *cnt;

  /* 引数チェック */
  assert((count != NULL) && (min_shanten != NULL));

  /* 面子を抜いて調べる */
  cnt = &count->count[0];
  for (; pos < MJTILE_MAX; pos++) {
    if (cnt[pos] == 0) {
      continue;
    }
    /* 刻子を抜いて調べる */
    if (cnt[pos] >= 3) {
      cnt[pos] -= 3;
      MJShanten_CalculateNormalShantenMentsu(count, pos, num_mentsu + 1, min_shanten);
      cnt[pos] += 3;
    }
    /* 順子を抜いて調べる */
    if (MJTILE_IS_SUHAI(pos) && (cnt[pos + 1] > 0) && (cnt[pos + 2] > 0)) {
      cnt[pos]--; cnt[pos + 1]--; cnt[pos + 2]--;
      MJShanten_CalculateNormalShantenMentsu(count, pos, num_mentsu + 1, min_shanten);
      cnt[pos]++; cnt[pos + 1]++; cnt[pos + 2]++;
    }
  }

  /* 面子を抜き終わった（これ以上抜けない）ので搭子を抜いて調べる */
  MJShanten_CalculateNormalShantenTatsu(count, 0, num_mentsu, 0, min_shanten);
}

/* 搭子（面子候補）を含めた向聴数計算 */
static void MJShanten_CalculateNormalShantenTatsu(
    struct MJTileCount *count, int32_t pos,
    int32_t num_mentsu, int32_t num_tatsu, int32_t *min_shanten)
{
  int32_t shanten;
  int32_t *cnt;

  /* 引数チェック */
  assert((count != NULL) && (min_shanten != NULL));

  /* 面子数+塔子数の上限に達していたら終わり */
  if ((num_mentsu + num_tatsu) >= 4) {
    goto EXIT;
  }

  /* 塔子を抜いて調べる */
  cnt = &count->count[0];
  for (; pos < MJTILE_MAX; pos++) {
    if (cnt[pos] == 0) {
      continue;
    }
    /* 対子を抜いて調べる */
    if (cnt[pos] == 2) {
      cnt[pos] -= 2;
      MJShanten_CalculateNormalShantenTatsu(
          count, pos, num_mentsu, num_tatsu + 1, min_shanten);
      cnt[pos] += 2;
    }
    /* 辺張or両面を抜いて調べる */
    if (MJTILE_IS_SUHAI(pos) && (cnt[pos + 1] > 0)) {
      cnt[pos]--; cnt[pos + 1]--;
      MJShanten_CalculateNormalShantenTatsu(
          count, pos, num_mentsu, num_tatsu + 1, min_shanten);
      cnt[pos]++; cnt[pos + 1]++;
    }
    /* 嵌張を抜いて調べる */
    if (MJTILE_IS_CHUNCHAN(pos + 1) && (cnt[pos + 2] > 0)) {
      cnt[pos]--; cnt[pos + 2]--;
      MJShanten_CalculateNormalShantenTatsu(
          count, pos, num_mentsu, num_tatsu + 1, min_shanten);
      cnt[pos]++; cnt[pos + 2]++;
    }
  }

EXIT:
  /* 向聴数計算 */
  shanten = 8 - 2 * num_mentsu - num_tatsu;
  /* 最小向聴数更新 */
  if (shanten < (*min_shanten)) {
    (*min_shanten) = shanten;
  }
}

/* 通常・七対子・国士無双手の中で最小の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJShanten_CalculateShanten(const struct MJTileCount *count)
{
  int32_t result, tmp;

  /* 引数チェック */
  assert(count != NULL);

  /* 通常手の向聴数を基準 */
  result = MJShanten_CalculateNormalShanten(count);

  /* 七対子・国士無双手の向聴数と比較し、小さい方を選択 */
  if ((tmp = MJShanten_CalculateChitoitsuShanten(count)) < result) {
    result = tmp;
  }
  if ((tmp = MJShanten_CalculateKokushimusouShanten(count)) < result) {
    result = tmp;
  }

  return result;
}

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
  /* 引数チェック */
  assert((count != NULL) && (candidate_effective_tile_flag != NULL));

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
static MJEffectiveTileApiResult MJEffectiveTile_GetEffectiveTilesCore(const struct MJTileCount *count,
    const bool *candidate_effective_tile_flag, ShantenCalculatorFunction shanten_calculator, struct MJEffectiveTiles *effective_tiles)
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
        tmp.tiles[tmp.num_effective_tiles] = (MJTile)t;
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
  bool candidate_effective_tile_flag[MJTILE_MAX] = { false, };

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
  bool candidate_effective_tile_flag[MJTILE_MAX] = { false, };

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
  bool candidate_effective_tile_flag[MJTILE_MAX] = { false, };

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

/* 有効牌（向聴数を下げる牌）リストアップ */
MJEffectiveTileApiResult MJEffectiveTile_GetEffectiveTiles(const struct MJTileCount *count, struct MJEffectiveTiles *effective_tiles)
{
  bool candidate_effective_tile_flag[MJTILE_MAX] = { false, };

  /* 引数チェック */
  if ((count == NULL) || (effective_tiles == NULL)) {
    return MJEFFECTIVETILE_APIRESULT_INVALID_ARGUMENT;
  }

  /* 有効牌候補をリストアップ */
  MJEffectiveTile_SetNormalCandidateEffectiveTileFlag(count, candidate_effective_tile_flag);
  MJEffectiveTile_SetChitoitsuCandidateEffectiveTileFlag(count, candidate_effective_tile_flag);
  MJEffectiveTile_SetKokushimusouCandidateEffectiveTileFlag(count, candidate_effective_tile_flag);

  /* コア処理実行 */
  return MJEffectiveTile_GetEffectiveTilesCore(count,
      candidate_effective_tile_flag, MJShanten_CalculateShanten, effective_tiles);
}
