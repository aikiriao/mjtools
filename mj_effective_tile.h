#ifndef MJEFFECTIVETILE_H_INCLUDED
#define MJEFFECTIVETILE_H_INCLUDED

#include "mj_types.h"
#include <stdint.h>

/* 有効牌計算結果型 */
typedef enum MJEffectiveTileApiResultTag {
  MJEFFECTIVETILE_APIRESULT_OK = 0,             /* 成功 */
  MJEFFECTIVETILE_APIRESULT_INVALID_ARGUMENT,   /* 不正な引数 */
  MJEFFECTIVETILE_APIRESULT_INVALID_NUM_TILES,  /* 不正な牌数 */
  MJEFFECTIVETILE_APIRESULT_NG,                 /* 分類不能な失敗 */
} MJEffectiveTileApiResult;

/* 有効牌リスト */
struct MJEffectiveTiles {
  MJTile tiles[MJTILE_MAX];
  int32_t num_effective_tiles;  /* 有効牌個数 */
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 通常手の有効牌（向聴数を下げる牌）リストアップ */
MJEffectiveTileApiResult MJEffectiveTile_GetNormalEffectiveTiles(const struct MJHand *hand, struct MJEffectiveTiles *effective_tiles);

/* 七対子手の有効牌（向聴数を下げる牌）リストアップ */
MJEffectiveTileApiResult MJEffectiveTile_GetChitoitsuEffectiveTiles(const struct MJHand *hand, struct MJEffectiveTiles *effective_tiles);

/* 国士無双手の有効牌（向聴数を下げる牌）リストアップ */
MJEffectiveTileApiResult MJEffectiveTile_GetKokushimusouEffectiveTiles(const struct MJHand *hand, struct MJEffectiveTiles *effective_tiles);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MJEFFECTIVETILE_H_INCLUDED */
