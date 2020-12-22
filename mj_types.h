#ifndef MJTYPES_H_INCLUDED
#define MJTYPES_H_INCLUDED

#include <stdbool.h>

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

/* 風 */
typedef enum MJWindTag {
  MJWIND_TON = 0,      /* 東 */
  MJWIND_NAN,          /* 南 */
  MJWIND_SHA,          /* 西 */
  MJWIND_PEE,          /* 北 */
  MJWIND_INVALID,      /* 無効値 */
} MJWind;

/* 副露の状態 */
struct MJMeld {
  MJMeldType  type;     /* 種類 */
  MJTile      min_tile; /* 副露を構成する牌で最小の識別整数を持つ牌 */
};

/* 手牌 */
struct MJHand {
  MJTile        hand[13]; /* 純手牌 */
  struct MJMeld meld[4];  /* 副露 */
  int32_t       num_meld; /* 副露数 */
};

/* ドラ牌 */
struct MJDoraTile {
  MJTile  omote[5];	/* 表ドラ */
  MJTile  ura[5];		/* 裏ドラ */
  int32_t num_dora;	/* ドラ表示枚数 */
};

#endif /* MJTYPES_H_INCLUDED */
