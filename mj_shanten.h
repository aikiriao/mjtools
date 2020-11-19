#ifndef MJSHANTEN_H_INCLUDED
#define MJSHANTEN_H_INCLUDED

#include "mj_tile.h"
#include <stdint.h>

/* 手牌の状態 */
struct MJHand {
  uint8_t hand[MJTILE_MAX];  /* 各牌の所持数が並んだ配列 */
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 通常手の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJShanten_CalculateNormalShanten(const struct MJHand *hand);

/* 通常手の向聴数を計算(テーブルを使用した高速実装) 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJShanten_CalculateNormalShantenUseTable(const struct MJHand *hand);

/* 七対子の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJShanten_CalculateChitoitsuShanten(const struct MJHand *hand);

/* 国士無双の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJShanten_CalculateKokushimusouShanten(const struct MJHand *hand);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MJSHANTEN_H_INCLUDED */
