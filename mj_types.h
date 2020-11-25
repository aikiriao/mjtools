#ifndef MJTYPES_H_INCLUDED
#define MJTYPES_H_INCLUDED

/* 牌定義 */
#include "mj_tile.h"

/* 副露の識別型 */
typedef enum MJMeldTypeTag {
  MJMELD_TYPE_PUNG = 0,    /* ポン   */
  MJMELD_TYPE_CHOW,        /* チー   */
  MJMELD_TYPE_ANKAN,       /* 暗槓   */
  MJMELD_TYPE_MINKAN,      /* 明槓   */
  MJMELD_TYPE_KAKAN,       /* 加槓   */
  MJMELD_TYPE_INVALID,     /* 無効値 */
} MJMeldType;

/* 副露の状態 */
struct MJMeld {
  MJMeldType  type;     /* 種類 */
  MJTile      minhai;   /* 副露を構成する牌で最小の識別整数を持つ牌 */
};

/* 風 */
typedef enum MJWindTag {
  MJWIND_TON = 0,      /* 東 */
  MJWIND_NAN,          /* 南 */
  MJWIND_SHA,          /* 西 */
  MJWIND_PEE,          /* 北 */
  MJWIND_INVALID,      /* 無効値 */
} MJWind;

/* 手牌 */
struct MJHand {
  MJTile        hand[13]; /* 純手牌 */
  struct MJMeld meld[4];  /* 副露 */
  int32_t       num_meld; /* 副露数 */
};

#endif /* MJTYPES_H_INCLUDED */