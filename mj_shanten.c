#include "mj_shanten.h"
#include "mj_utility.h"

/* テーブル引きを使用するか？ */
#define MJSHANTEN_USE_TABLE

#include <string.h>
#include <assert.h>

#if !defined(MJSHANTEN_USE_TABLE)

/* テーブルを使用しないナイーブな実装 */
static int32_t MJShanten_CalculateNormalSyantenNaive(const struct Tehai *tehai);
/* 面子を含めた向聴数計算 */
static void MJShanten_CalculateNormalSyantenMentsu(
    struct Tehai *tehai, int32_t pos, int32_t head, 
    int32_t num_mentsu, int32_t *min_shanten);
/* 搭子（面子候補）を含めた向聴数計算 */
static void MJShanten_CalculateNormalSyantenTatsu(
    struct Tehai *tehai, int32_t pos, int32_t head,
    int32_t num_mentsu, int32_t num_tatsu, int32_t *min_shanten);

#else /* MJSHANTEN_USE_TABLE */

/* 2**19 shanten.datのエントリ数を超える最小の2の冪数 */
#define MJSHANTEN_TABLE_SIZE        524288
/* Knuthの乗法ハッシュ */
#define MJSHANTEN_MAKE_HASH(val)    ((val) * 2654435769UL)

/* 向聴テーブルのエントリ */
struct ShantenTableEntry {
  uint32_t value;       /* 数牌の所持数の整数表記 */
  uint8_t  num_mentsu;  /* 面子数 */
  uint8_t  num_tatsu;   /* 塔子数 */
};

/* 向聴数計算 テーブル使用版 */
static int32_t MJShanten_CalculateNormalSyantenUseTable(const struct Tehai *tehai);
/* 数牌の並びを頼りにテーブル探索 */
static const struct ShantenTableEntry *MJShanten_SearchTableEntry(const uint8_t *suhai);
/* テーブル使用時のコア処理 */
static int32_t MJShanten_CalculateNormalSyantenUseTableCore(const struct Tehai *tehai);

/* 数牌の並びに対応した面子/塔子テーブル */
static const struct ShantenTableEntry st_shanten_table[] = {
#include "mj_shanten_table.c"
};

#endif /* MJSHANTEN_USE_TABLE */

/* 七対子の向聴数を計算 */
int32_t MJShanten_CalculateChitoitsuSyanten(const struct Tehai *tehai)
{
  int32_t i, shanten, num_toitu, num_types;
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
  shanten = 6 - num_toitu;

  /* 3枚持ち以上がある場合は7種より小さくなる */
  /* 3枚持ち以上の面子は無効のため、その分向聴数を増やす */
  if (num_types < 7) {
    shanten += (7 - num_types);
  }

  return shanten;
}

/* 国士無双の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJShanten_CalculateKokushimusouSyanten(const struct Tehai *tehai)
{
  int32_t i, shanten, head;
  const uint8_t *hai;

  /* 引数チェック */
  assert(tehai != NULL);

  hai = &tehai->tehai[0];

  /* 么九牌を数える */
  shanten = 13; head = 0;
  for (i = 0; i < MJTILE_MAX; i++) {
    if (MJTILE_IS_YAOCHU(i) && (hai[i] > 0)) {
      shanten--;
      /* 頭をマーク */
      if ((hai[i] >= 2) && (head == 0)) {
        head = 1;
      }
    }
  }

  /* 頭があるぶん減らす */
  shanten -= head;

  return shanten;
}

/* 通常手の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJShanten_CalculateNormalSyanten(const struct Tehai *tehai)
{
  /* 関数の呼び分け */
#if !defined(MJSHANTEN_USE_TABLE)
  return MJShanten_CalculateNormalSyantenNaive(tehai);
#else /* MJSHANTEN_USE_TABLE */
  return MJShanten_CalculateNormalSyantenUseTable(tehai);
#endif
}

#if !defined(MJSHANTEN_USE_TABLE)
/* 向聴数を計算 */
static int32_t MJShanten_CalculateNormalSyantenNaive(const struct Tehai *tehai)
{
  struct Tehai tmp;
  int32_t i, min_shanten;

  /* 引数チェック */
  assert(tehai != NULL);

  /* 作業用に手牌をコピー */
  memcpy(&tmp, tehai, sizeof(struct Tehai));

  min_shanten = 8;
	for (i = 0; i < MJTILE_MAX; i++) {
		/* 頭を抜いて調べる */
		if (tmp.tehai[i] >= 2) {            
			tmp.tehai[i] -= 2;
      MJShanten_CalculateNormalSyantenMentsu(&tmp, 0, 1, 0, &min_shanten);
			tmp.tehai[i] += 2;
		}
	}

  /* 副露なしなら頭なしとして計算 */
  if (tehai->num_fuuro == 0) {
    MJShanten_CalculateNormalSyantenMentsu(&tmp, 0, 0, 0, &min_shanten);
  }

  /* 最終結果 */
  return min_shanten - 2 * tehai->num_fuuro;
}

/* 面子を含めた向聴数計算 */
static void MJShanten_CalculateNormalSyantenMentsu(
    struct Tehai *tehai, int32_t pos, int32_t head, 
    int32_t num_mentsu, int32_t *min_shanten)
{
  uint8_t *hai;

  /* 引数チェック */
  assert((tehai != NULL) && (min_shanten != NULL));

  /* 面子を抜いて調べる */
  hai = &tehai->tehai[0];
  for (; pos < MJTILE_MAX; pos++) {
    if (hai[pos] == 0) {
      continue;
    }
    /* 刻子を抜いて調べる */
    if (hai[pos] >= 3) {
      hai[pos] -= 3;
      MJShanten_CalculateNormalSyantenMentsu(tehai, pos, head, num_mentsu + 1, min_shanten);
      hai[pos] += 3;
    }
    /* 順子を抜いて調べる */
    if (MJTILE_IS_SUHAI(pos) && (hai[pos + 1] > 0) && (hai[pos + 2] > 0)) {
      hai[pos]--; hai[pos + 1]--; hai[pos + 2]--;
      MJShanten_CalculateNormalSyantenMentsu(tehai, pos, head, num_mentsu + 1, min_shanten);
      hai[pos]++; hai[pos + 1]++; hai[pos + 2]++;
    }
  }

  /* 面子を抜き終わった（これ以上抜けない）ので搭子を抜いて調べる */
  MJShanten_CalculateNormalSyantenTatsu(tehai, 0, head, num_mentsu, 0, min_shanten);
}

/* 搭子（面子候補）を含めた向聴数計算 */
static void MJShanten_CalculateNormalSyantenTatsu(
    struct Tehai *tehai, int32_t pos, int32_t head,
    int32_t num_mentsu, int32_t num_tatsu, int32_t *min_shanten)
{
  int32_t shanten;
  uint8_t *hai;

  /* 引数チェック */
  assert((tehai != NULL) && (min_shanten != NULL));

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
      MJShanten_CalculateNormalSyantenTatsu(
          tehai, pos, head, num_mentsu, num_tatsu + 1, min_shanten);
      hai[pos] += 2;
    }
    /* 辺張or両面を抜いて調べる */
    if (MJTILE_IS_SUHAI(pos) && (hai[pos + 1] > 0)) {
      hai[pos]--; hai[pos + 1]--;
      MJShanten_CalculateNormalSyantenTatsu(
          tehai, pos, head, num_mentsu, num_tatsu + 1, min_shanten);
      hai[pos]++; hai[pos + 1]++;
    }
    /* 嵌張を抜いて調べる */
    if (MJTILE_IS_CHUNCHAN(pos + 1) && (hai[pos + 2] > 0)) {
      hai[pos]--; hai[pos + 2]--;
      MJShanten_CalculateNormalSyantenTatsu(
          tehai, pos, head, num_mentsu, num_tatsu + 1, min_shanten);
      hai[pos]++; hai[pos + 2]++;
    }
  }

EXIT:
  /* 向聴数計算 */
  shanten = 8 - 2 * num_mentsu - num_tatsu - head;
  /* 最小向聴数更新 */
  if (shanten < (*min_shanten)) {
    (*min_shanten) = shanten;
  }
}

#else /* MJSHANTEN_USE_TABLE */

/* 向聴数計算 テーブル使用版 */
static int32_t MJShanten_CalculateNormalSyantenUseTable(const struct Tehai *tehai)
{
  struct Tehai tmp;
  int32_t i, shanten, min_shanten;

  /* 引数チェック */
  assert(tehai != NULL);

  /* 作業用に手牌をコピー */
  memcpy(&tmp, tehai, sizeof(struct Tehai));

  min_shanten = 8;
	for (i = 0; i < MJTILE_MAX; i++) {
		/* 頭を抜いて調べる */
		if (tmp.tehai[i] >= 2) {            
			tmp.tehai[i] -= 2;
      /* 向聴数計算 頭を抜くので-1 */
      shanten = MJShanten_CalculateNormalSyantenUseTableCore(&tmp) - 1;
      if (shanten < min_shanten) { min_shanten = shanten; }
			tmp.tehai[i] += 2;
		}
	}

  /* 副露なしなら頭なしとして計算 */
  if (tehai->num_fuuro == 0) {
    shanten = MJShanten_CalculateNormalSyantenUseTableCore(&tmp);
    if (shanten < min_shanten) { min_shanten = shanten; }
  }

  /* 最終結果 */
  return min_shanten - 2 * tehai->num_fuuro;
}

/* 数牌の並びを頼りにテーブル探索 */
static const struct ShantenTableEntry *MJShanten_SearchTableEntry(const uint8_t *suhai)
{
  uint32_t key, value, number;
  /* 10の冪数テーブル（左側に一番若い数牌が置かれるため、降順） */
  static const uint32_t exp10_table[10] = {
    0, /* ダミー(インデックスアクセス側で-1するのを防ぐ) */
    1e8, 1e7, 1e6, 1e5, 1e4, 1e3, 1e2, 1e1, 1e0, 
  };

  assert(suhai != NULL);

  /* 牌を10進数に変換 */
  value = 0;
  for (number = 1; number <= 9; number++) {
    value += suhai[number] * exp10_table[number];
  }

  /* 開番地法: 空きエントリ探索 */
  key = MJSHANTEN_MAKE_HASH(value) & (MJSHANTEN_TABLE_SIZE - 1);
  while (st_shanten_table[key].value != value) {
    key = (key + 1) & (MJSHANTEN_TABLE_SIZE - 1);
  }

  return &st_shanten_table[key];
}

/* テーブル使用時のコア処理 */
static int32_t MJShanten_CalculateNormalSyantenUseTableCore(const struct Tehai *tehai)
{
  int32_t pos, type;
  int32_t num_mentsu, num_tatsu;
  const struct ShantenTableEntry *ptable;
  const uint8_t *hai;

  assert(tehai != NULL);

  /* 数牌の並びに関してチェック */
  MJUTILITY_STATIC_ASSERT(
         (MJTILE_1MAN ==  1) && (MJTILE_9MAN ==  9)
      && (MJTILE_1PIN == 11) && (MJTILE_9PIN == 19)
      && (MJTILE_1SOU == 21) && (MJTILE_9SOU == 29));

  /* 配列を頻繁に参照するので一旦オート変数に受ける */
  hai = &tehai->tehai[0];
  /* 面子/塔子数のリセット */
  num_mentsu = num_tatsu = 0;

  /* 萬子, 筒子, 索子の3種類について、数牌の並びを元に面子,塔子をカウント */
  for (type = 0; type < 3; type++) {
    /* テーブルエントリの探索・取得 */
    ptable = MJShanten_SearchTableEntry(&hai[10 * type]);
    /* 面子/塔子数の更新 */
    num_mentsu += ptable->num_mentsu;
    num_tatsu  += ptable->num_tatsu;
  }

  /* 字牌については刻子と対子だけ数えれば良い */
  for (pos = MJTILE_TON; pos < MJTILE_MAX; pos++) {
    if (hai[pos] == 2) {
      num_tatsu++;
    } else if (hai[pos] > 2) {
      num_mentsu++;
    }
  }

  /* 刻子+塔子が4を超えていたら4になるように塔子数を調整 */
  if ((num_mentsu + num_tatsu) > 4) {
    num_tatsu = 4 - num_mentsu;
  }

  /* 最終結果 */
  return 8 - 2 * num_mentsu - num_tatsu;
}

#endif /* MJSHANTEN_USE_TABLE */
