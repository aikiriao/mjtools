#include "mj_syanten.h"

#include <string.h>
#include <assert.h>

/* 面子を含めた向聴数計算 */
static void MJSyanten_CalculateNormalSyantenMentsu(
    struct Tehai *tehai, int32_t pos, int32_t head, 
    int32_t num_mentsu, int32_t *min_syanten);

/* 搭子（面子候補）を含めた向聴数計算 */
static void MJSyanten_CalculateNormalSyantenTatsu(
    struct Tehai *tehai, int32_t pos, int32_t head,
    int32_t num_mentsu, int32_t num_tatsu, int32_t *min_syanten);

/* 七対子の向聴数を計算 */
int32_t MJSyanten_CalculateChitoitsuSyanten(const struct Tehai *tehai)
{
  int32_t i, syanten, num_toitu, num_types;
  const uint8_t *hai;

  /* 引数チェック */
  assert(tehai != NULL);

  hai = &tehai->tehai[0];

  /* 対子を数える */
  num_toitu = num_types = 0;
  for (i = 0; i < MJTILE_MAX; i++) {
    if (hai[i] == 0) {
      continue; 
    }
    num_types++;
    if (hai[i] >= 2) {
      num_toitu++;
    }
  }
  
  /* 基本は(6-対子数) */
  syanten = 6 - num_toitu;

  /* 3枚持ち以上がある場合は7種より小さくなる */
  /* 3枚持ち以上の面子は無効のため、その分向聴数を増やす */
  if (num_types < 7) {
    syanten += (7 - num_types);
  }

  return syanten;
}

/* 国士無双の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJSyanten_CalculateKokushimusouSyanten(const struct Tehai *tehai)
{
  int32_t i, syanten, head;
  const uint8_t *hai;

  /* 引数チェック */
  assert(tehai != NULL);

  hai = &tehai->tehai[0];

  /* 么九牌を数える */
  syanten = 13; head = 0;
  for (i = 0; i < MJTILE_MAX; i++) {
    if (MJTILE_IS_YAOCHU(i) && (hai[i] > 0)) {
      syanten--;
      /* 頭をマーク */
      if ((hai[i] >= 2) && (head == 0)) {
        head = 1;
      }
    }
  }

  /* 頭があるぶん減らす */
  syanten -= head;

  return syanten;
}

/* 向聴数を計算 */
int32_t MJSyanten_CalculateNormalSyanten(const struct Tehai *tehai)
{
  struct Tehai tmp;
  int32_t i, min_syanten;

  /* 引数チェック */
  assert(tehai != NULL);

  /* 作業用に手牌をコピー */
  memcpy(&tmp, tehai, sizeof(struct Tehai));

  min_syanten = 8;
	for (i = 0; i < MJTILE_MAX; i++) {
		/* 頭を抜いて調べる */
		if (tmp.tehai[i] >= 2) {            
			tmp.tehai[i] -= 2;
      MJSyanten_CalculateNormalSyantenMentsu(&tmp, 0, 1, 0, &min_syanten);
			tmp.tehai[i] += 2;
		}
	}

  /* 副露なしなら頭なしとして計算 */
  if (tehai->num_fuuro == 0) {
    MJSyanten_CalculateNormalSyantenMentsu(&tmp, 0, 0, 0, &min_syanten);
  }

  /* 最終結果 */
  return min_syanten - 2 * tehai->num_fuuro;
}

/* 面子を含めた向聴数計算 */
static void MJSyanten_CalculateNormalSyantenMentsu(
    struct Tehai *tehai, int32_t pos, int32_t head, 
    int32_t num_mentsu, int32_t *min_syanten)
{
  uint8_t *hai;

  /* 引数チェック */
  assert((tehai != NULL) && (min_syanten != NULL));

  /* 面子を抜いて調べる */
  hai = &tehai->tehai[0];
  for (; pos < MJTILE_MAX; pos++) {
    if (hai[pos] == 0) {
      continue;
    }
    /* 刻子を抜いて調べる */
    if (hai[pos] >= 3) {
      hai[pos] -= 3;
      MJSyanten_CalculateNormalSyantenMentsu(tehai, pos, head, num_mentsu + 1, min_syanten);
      hai[pos] += 3;
    }
    /* 順子を抜いて調べる */
    if (MJTILE_IS_SUHAI(pos) && (hai[pos + 1] > 0) && (hai[pos + 2] > 0)) {
      hai[pos]--; hai[pos + 1]--; hai[pos + 2]--;
      MJSyanten_CalculateNormalSyantenMentsu(tehai, pos, head, num_mentsu + 1, min_syanten);
      hai[pos]++; hai[pos + 1]++; hai[pos + 2]++;
    }
  }

  /* 面子を抜き終わった（これ以上抜けない）ので搭子を抜いて調べる */
  MJSyanten_CalculateNormalSyantenTatsu(tehai, 0, head, num_mentsu, 0, min_syanten);
}

/* 搭子（面子候補）を含めた向聴数計算 */
static void MJSyanten_CalculateNormalSyantenTatsu(
    struct Tehai *tehai, int32_t pos, int32_t head,
    int32_t num_mentsu, int32_t num_tatsu, int32_t *min_syanten)
{
  int32_t syanten;
  uint8_t *hai;

  /* 引数チェック */
  assert((tehai != NULL) && (min_syanten != NULL));

  /* 面子数+塔子数の上限に達していたら終わり */
  if ((num_mentsu + num_tatsu) >= 4) {
    goto EXIT;
  }

  /* 塔子を抜いて調べる */
  hai = &tehai->tehai[0];
  for (; pos < MJTILE_MAX; pos++) {
    if (hai[pos] == 0) {
      continue;
    }
    /* 対子を抜いて調べる */
    if (hai[pos] == 2) {
      hai[pos] -= 2;
      MJSyanten_CalculateNormalSyantenTatsu(
          tehai, pos, head, num_mentsu, num_tatsu + 1, min_syanten);
      hai[pos] += 2;
    }
    /* 辺張or両面を抜いて調べる */
    if (MJTILE_IS_SUHAI(pos) && (hai[pos + 1] > 0)) {
      hai[pos]--; hai[pos + 1]--;
      MJSyanten_CalculateNormalSyantenTatsu(
          tehai, pos, head, num_mentsu, num_tatsu + 1, min_syanten);
      hai[pos]++; hai[pos + 1]++;
    }
    /* 嵌張を抜いて調べる */
    if (MJTILE_IS_CHUNCHAN(pos + 1) && (hai[pos + 2] > 0)) {
      hai[pos]--; hai[pos + 2]--;
      MJSyanten_CalculateNormalSyantenTatsu(
          tehai, pos, head, num_mentsu, num_tatsu + 1, min_syanten);
      hai[pos]++; hai[pos + 2]++;
    }
  }

EXIT:
  /* 向聴数計算 */
  syanten = 8 - 2 * num_mentsu - num_tatsu - head;
  /* 最小向聴数更新 */
  if (syanten < (*min_syanten)) {
    (*min_syanten) = syanten;
  }
}
