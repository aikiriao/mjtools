#ifndef MJSHANTEN_H_INCLUDED
#define MJSHANTEN_H_INCLUDED

#include "mj_types.h"
#include <stdint.h>

/* 有効牌計算結果型 */
typedef enum MJEffectiveTileApiResultTag {
  MJEFFECTIVETILE_APIRESULT_OK = 0,             /* 成功 */
  MJEFFECTIVETILE_APIRESULT_INVALID_ARGUMENT,   /* 不正な引数 */
  MJEFFECTIVETILE_APIRESULT_INVALID_NUM_TILES,  /* 不正な牌数 */
  MJEFFECTIVETILE_APIRESULT_NG,                 /* 分類不能な失敗 */
} MJEffectiveTileApiResult;

/* 各牌の所持数が並んだ配列 */
struct MJTileCount {
  int32_t count[MJTILE_MAX];
};

/* 有効牌（向聴数を下げる牌）リスト */
struct MJEffectiveTiles {
  MJTile tiles[MJTILE_MAX];
  int32_t num_effective_tiles;  /* 有効牌個数 */
};

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

/* 通常手の有効牌（向聴数を下げる牌）リストアップ */
MJEffectiveTileApiResult MJEffectiveTile_GetNormalEffectiveTiles(const struct MJTileCount *count, struct MJEffectiveTiles *effective_tiles);

/* 七対子手の有効牌（向聴数を下げる牌）リストアップ */
MJEffectiveTileApiResult MJEffectiveTile_GetChitoitsuEffectiveTiles(const struct MJTileCount *count, struct MJEffectiveTiles *effective_tiles);

/* 国士無双手の有効牌（向聴数を下げる牌）リストアップ */
MJEffectiveTileApiResult MJEffectiveTile_GetKokushimusouEffectiveTiles(const struct MJTileCount *count, struct MJEffectiveTiles *effective_tiles);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MJSHANTEN_H_INCLUDED */
