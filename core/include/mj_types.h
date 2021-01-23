#ifndef MJTYPES_H_INCLUDED
#define MJTYPES_H_INCLUDED

#include <stdbool.h>

/* 牌定義 */
#include "mj_tile.h"

/* 河に捨てられる最大牌数 */
#define MJTYPES_MAX_NUM_DISCARDED_TILES  25

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
  MJTile      tiles[4]; /* 副露の構成牌 チーの場合は昇順に並ぶ */
};

/* 手牌 */
struct MJHand {
  MJTile        hand[13];       /* 純手牌 */
  int32_t       num_hand_tiles; /* 純手牌数 */
  struct MJMeld meld[4];        /* 副露 */
  int32_t       num_meld;       /* 副露数 */
};

/* ドラ牌 */
struct MJDoraTile {
  MJTile  omote[5];	/* 表ドラ */
  MJTile  ura[5];		/* 裏ドラ */
  int32_t num_dora;	/* ドラ表示枚数 */
};

/* 河牌 */
struct MJDiscardedTile {
  MJTile  tile;         /* 牌 */
  bool    riichi;       /* リーチ？ */
  MJWind  meld_player;  /* 鳴いたプレーヤー 鳴いていない場合はMJWIND_INVALID */
};

/* 各プレーヤーの河牌 */
struct MJPlayerRiver {
  struct MJDiscardedTile tiles[MJTYPES_MAX_NUM_DISCARDED_TILES];  /* 河牌の配列 */
  int32_t num_tiles;                                              /* 捨てられた牌の数 */
};

#endif /* MJTYPES_H_INCLUDED */
