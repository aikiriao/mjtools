#ifndef MJRIVER_H_INCLUDED
#define MJRIVER_H_INCLUDED

#include "mj_types.h"

/* 河に捨てられる最大牌数 */
#define MJTYPES_MAX_NUM_DISCARDED_TILES  25

/* API結果型 */
typedef enum MJRiverApiResultTag {
  MJRIVER_APIRESULT_OK = 0,
  MJRIVER_APIRESULT_INVALID_ARGUMENT, /* 不正な引数 */
  MJRIVER_APIRESULT_ALREADY_RIICHI,   /* 既にリーチが掛けられている */
  MJRIVER_APIRESULT_CANNOT_MELD,      /* 鳴けない */
  MJRIVER_APIRESULT_NG,
} MJRiverApiResult;

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

/* 河 */
struct MJRiver {
  struct MJPlayerRiver player_river[4];
  MJWind last_discard_player;
  MJTile last_discard_tile;
};

/* 初期化 */
void MJRiver_Initialize(struct MJRiver *river);

/* 1枚捨てる */
MJRiverApiResult MJRiver_DiscardTile(struct MJRiver *river, MJWind player_wind, MJTile tile, bool riichi);

/* ポンできるか？ */
bool MJRiver_CanPung(const struct MJRiver *river, MJWind player_wind, const struct MJHand *player_hand); 

/* チーできるか？ */
bool MJRiver_CanChow(const struct MJRiver *river, MJWind player_wind, const struct MJHand *player_hand); 

/* カン（大明槓）できるか？ */
bool MJRiver_CanKan(const struct MJRiver *river, MJWind player_wind, const struct MJHand *player_hand); 

/* ポン */
MJRiverApiResult MJRiver_Pung(struct MJRiver *river, MJWind player_wind, const struct MJHand *player_hand);

/* チー */
MJRiverApiResult MJRiver_Chow(struct MJRiver *river, MJWind player_wind, const struct MJHand *player_hand);

/* カン（大明槓） */
MJRiverApiResult MJRiver_Kan(struct MJRiver *river, MJWind player_wind, const struct MJHand *player_hand);

#endif /* MJRIVER_H_INCLUDED */
