#include "mj_shanten.h"
#include "mj_utility.h"

#include <string.h>
#include <assert.h>

/* ナイーブな向聴数の計算コア処理 */
static int32_t MJShanten_CalculateNormalShantenNaive(struct MJTileCount *count);
/* 面子を含めた向聴数計算 */
static void MJShanten_CalculateNormalShantenMentsu(
    struct MJTileCount *count, int32_t pos, 
    int32_t num_mentsu, int32_t *min_shanten);
/* 搭子（面子候補）を含めた向聴数計算 */
static void MJShanten_CalculateNormalShantenTatsu(
    struct MJTileCount *count, int32_t pos,
    int32_t num_mentsu, int32_t num_tatsu, int32_t *min_shanten);

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
