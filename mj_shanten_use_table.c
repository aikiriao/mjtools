#include "mj_shanten.h"
#include "mj_utility.h"

#include <string.h>
#include <assert.h>

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

/* 数牌の並びを頼りにテーブル探索 */
static const struct ShantenTableEntry *MJShanten_SearchTableEntry(const uint8_t *suhai);
/* テーブル使用時のコア処理 */
static int32_t MJShanten_CalculateNormalShantenUseTableCore(const struct MJTehai *tehai);

/* 数牌の並びに対応した面子/塔子テーブル */
static const struct ShantenTableEntry st_shanten_table[] = {
#include "mj_shanten_table.c"
};

/* 向聴数計算 テーブル使用版 */
int32_t MJShanten_CalculateNormalShantenUseTable(const struct MJTehai *tehai)
{
  struct MJTehai tmp;
  int32_t i, shanten, min_shanten;

  /* 引数チェック */
  assert(tehai != NULL);

  /* 作業用に手牌をコピー */
  memcpy(&tmp, tehai, sizeof(struct MJTehai));

  min_shanten = 8;
	for (i = 0; i < MJTILE_MAX; i++) {
		/* 頭を抜いて調べる */
		if (tmp.tehai[i] >= 2) {            
			tmp.tehai[i] -= 2;
      /* 向聴数計算 頭を抜くので-1 */
      shanten = MJShanten_CalculateNormalShantenUseTableCore(&tmp) - 1;
      if (shanten < min_shanten) { min_shanten = shanten; }
			tmp.tehai[i] += 2;
		}
	}

  /* 雀頭がない場合を含めるため、頭を抜かずに調べる */
  shanten = MJShanten_CalculateNormalShantenUseTableCore(&tmp);
  if (shanten < min_shanten) { min_shanten = shanten; }

  return min_shanten;
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
static int32_t MJShanten_CalculateNormalShantenUseTableCore(const struct MJTehai *tehai)
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
