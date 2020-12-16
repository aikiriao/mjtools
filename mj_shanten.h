#ifndef MJSHANTEN_H_INCLUDED
#define MJSHANTEN_H_INCLUDED

#include "mj_types.h"
#include <stdint.h>

/* 各牌の所持数が並んだ配列 */
struct MJTileCount {
  int32_t count[MJTILE_MAX];
};

/* TODO: 有効牌リストアップ関数追加 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 手牌（副露含む）を牌出現カウントに変換 */
void MJShanten_ConvertHandToTileCount(const struct MJHand *hand, struct MJTileCount *count);

/* 通常手の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJShanten_CalculateNormalShanten(const struct MJTileCount *count);

/* 通常手の向聴数を計算(テーブルを使用した高速実装) 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJShanten_CalculateNormalShantenUseTable(const struct MJTileCount *count);

/* 七対子の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJShanten_CalculateChitoitsuShanten(const struct MJTileCount *count);

/* 国士無双の向聴数を計算 1で一向聴, 0で聴牌, -1で和了 */
int32_t MJShanten_CalculateKokushimusouShanten(const struct MJTileCount *count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MJSHANTEN_H_INCLUDED */
