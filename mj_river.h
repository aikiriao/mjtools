#ifndef MJRIVER_H_INCLUDED
#define MJRIVER_H_INCLUDED

#include "mj_types.h"

/* API結果型 */
typedef enum MJRiverApiResultTag {
  MJRIVER_APIRESULT_OK = 0,
  MJRIVER_APIRESULT_INVALID_ARGUMENT, /* 不正な引数 */
  MJRIVER_APIRESULT_ALREADY_RIICHI,   /* 既にリーチが掛けられている */
  MJRIVER_APIRESULT_CANNOT_MELD,      /* 鳴けない */
  MJRIVER_APIRESULT_NG,
} MJRiverApiResult;

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
