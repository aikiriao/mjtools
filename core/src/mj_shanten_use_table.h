#ifndef MJSHANTEN_USE_TABLE_H_INCLUDED
#define MJSHANTEN_USE_TABLE_H_INCLUDED

#include <stdint.h>

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

#endif /* MJSHANTEN_USE_TABLE_H_INCLUDED */
