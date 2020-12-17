#ifndef MJPLAYERINTERFACE_H_INCLUDED
#define MJPLAYERINTERFACE_H_INCLUDED

#include "mj_types.h"
#include <stdint.h>

/* インターフェースバージョン番号 */
#define MJPLAYERINTERFACE_VERSION    2

/* トークン3連結マクロ */
#define MJPLAYER_CAT3_SUB(x, y, z) x ## y ## z
#define MJPLAYER_CAT3(x, y, z)     MJPLAYER_CAT3_SUB(x, y, z)

/* バージョン番号を含んだ構造体を定義 */
typedef struct MJPLAYER_CAT3(MJPlayerInterfaceVersion, MJPLAYERINTERFACE_VERSION, TagTag) MJPLAYER_CAT3(MJPlayerInterfaceVersion, MJPLAYERINTERFACE_VERSION, Tag);

#undef MJPLAYER_PP_CAT3_SUB
#undef MJPLAYER_PP_CAT3

/* 局終了の理由 */
typedef enum MJHandEndReasonTag {
  MJHAND_ENDREASON_RON = 0, /* 誰かが和了った */
  MJHAND_ENDREASON_DRAWN,   /* 流局 */
  MJHAND_ENDREASON_MISTAKE, /* チョンボ */
} MJHandEndReason;

/* プレーヤーのアクション識別 */
typedef enum MJPlayerActionTypeTag {
  MJPLAYER_ACTIONTYPE_NONE = 0, /* 何もしない（流す） */
  MJPLAYER_ACTIONTYPE_DRAW,     /* 自摸 */
  MJPLAYER_ACTIONTYPE_DISCARD,  /* 牌を捨てる */
  MJPLAYER_ACTIONTYPE_PUNG,     /* ポン */
  MJPLAYER_ACTIONTYPE_CHOW1,    /* 下の牌でチー */
  MJPLAYER_ACTIONTYPE_CHOW2,    /* 中の牌でチー */
  MJPLAYER_ACTIONTYPE_CHOW3,    /* 上の牌でチー */
  MJPLAYER_ACTIONTYPE_ANKAN,    /* アンカン */
  MJPLAYER_ACTIONTYPE_MINKAN,   /* ミンカン */
  MJPLAYER_ACTIONTYPE_KAKAN,    /* カカン */
  MJPLAYER_ACTIONTYPE_RIICHI,   /* リーチ */
  MJPLAYER_ACTIONTYPE_RON,      /* ロン */
  MJPLAYER_ACTIONTYPE_DRAWN,    /* 流す */
} MJPlayerActionType;

/* プレーヤーのアクション */
struct MJPlayerAction {
  MJPlayerActionType  type; /* アクション種別 */
  MJTile              tile; /* 関連する牌 */
};

/* プレーヤーインターフェース */
struct MJPlayerInterface {
  /* プレーヤーインターフェース名の取得 引数は無視してください */
  const char *(*GetName)(const MJPlayerInterfaceVersion2Tag *version_tag);
  /* ワークサイズ計算 */
  int32_t (*CalculateWorkSize)(void);
  /* プレーヤーインスタンス作成 */
  void *(*Create)(void *work, int32_t work_size);
  /* プレーヤーインスタンス破棄 */
  void (*Destroy)(void *player);
  /* 誰かのアクション時 */
  void (*OnAction)(void *player, MJWind trigger_player, const struct MJPlayerAction *trigger_action, MJWind action_player, struct MJPlayerAction *action);
  /* 自摸時 */
  void (*OnDraw)(void *player, const struct MJHand *hand, MJTile draw_tile, struct MJPlayerAction *player_action);
  /* 局開始時 */
  void (*OnStartHand)(void *player, int32_t hand_no, MJWind player_wind);
  /* 局終了時 */
  void (*OnEndHand)(void *player, MJHandEndReason reason, const int32_t *score_diff);
  /* 対戦開始時 */
  void (*OnStartGame)(void *player);
  /* 対戦終了時 */
  void (*OnEndGame)(void *player, int32_t player_rank, int32_t player_score);
};

#endif /* MJPLAYERINTERFACE_H_INCLUDED */
