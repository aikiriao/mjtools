#ifndef MJSYANTEN_H_INCLUDED
#define MJSYANTEN_H_INCLUDED

#include "mj_tile.h"
#include <stdint.h>

/* 手牌の状態 */
struct Tehai {
  uint8_t tehai[MJTILE_MAX];  /* 各牌の所持数が並んだ配列 */
  uint8_t num_fuuro;          /* 副露数 */
};

/* 通常手の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJSyanten_CalculateNormalSyanten(const struct Tehai *tehai);

/* 七対子の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJSyanten_CalculateChitoitsuSyanten(const struct Tehai *tehai);

/* 国士無双の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJSyanten_CalculateKokushimusouSyanten(const struct Tehai *tehai);

#endif /* MJSYANTEN_H_INCLUDED */
