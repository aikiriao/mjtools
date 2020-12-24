#ifndef MJPLAYERINTERFACE_H_INCLUDED
#define MJPLAYERINTERFACE_H_INCLUDED

#include "mj_types.h"
#include <stdint.h>

/* プレーヤーインターフェースバージョン番号 */
#define MJPLAYER_INTERFACE_VERSION             3
/* ゲーム状態取得インターフェースバージョン番号 */
#define MJGAMESTATEGETTER_INTERFACE_VERSION    1

/* トークン3連結マクロ */
#define MJPLAYERINTERFACE_CAT3_SUB(x, y, z) x ## y ## z
#define MJPLAYERINTERFACE_CAT3(x, y, z)     MJPLAYERINTERFACE_CAT3_SUB(x, y, z)

/* バージョン番号を含んだ構造体を定義 */
typedef struct MJPLAYERINTERFACE_CAT3(MJPlayerInterfaceVersion, MJPLAYER_INTERFACE_VERSION, TagTag) MJPLAYERINTERFACE_CAT3(MJPlayerInterfaceVersion, MJPLAYER_INTERFACE_VERSION, Tag);
typedef struct MJPLAYERINTERFACE_CAT3(MJGameStateGetterInterfaceVersion, MJGAMESTATEGETTER_INTERFACE_VERSION, TagTag) MJPLAYERINTERFACE_CAT3(MJGameStateGetterInterfaceVersion, MJGAMESTATEGETTER_INTERFACE_VERSION, Tag);

#undef MJPLAYERINTERFACE_PP_CAT3_SUB
#undef MJPLAYERINTERFACE_PP_CAT3

/* 局終了の理由 */
typedef enum MJHandEndReasonTag {
  MJHAND_ENDREASON_RON = 0, /* 誰かが和了った */
  MJHAND_ENDREASON_DRAWN,   /* 流局 */
  MJHAND_ENDREASON_MISTAKE, /* チョンボ */
} MJHandEndReason;

/* プレーヤーのアクション識別 */
typedef enum MJPlayerActionTypeTag {
  MJPLAYER_ACTIONTYPE_NONE = 0, /* 何もしない（無視） */
  MJPLAYER_ACTIONTYPE_TSUMO,    /* 自摸和了 */
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

/* ゲームの状態取得インターフェース */
struct MJGameStateGetterInterface {
  /* インターフェースバージョン取得 */
  uint32_t (*GetVersion)(const MJGameStateGetterInterfaceVersion1Tag *version_tag);
  /* 手牌の取得 自分以外の場合は副露のみ見え、純手牌はINVALIDがセットされる */
  void (*GetHand)(MJWind player, struct MJHand *hand);
  /* 指定したプレーヤーの河の取得 */
  void (*GetRiver)(MJWind player, struct MJPlayerRiver *river);
  /* ドラ表示牌の取得(裏ドラはINVALIDがセットされる) */
  void (*GetDora)(struct MJDoraTile *dora);
  /* 指定したプレーヤーの現得点の取得 */
  int32_t (*GetScore)(MJWind player);
  /* 局番号を取得 */
  int32_t (*GetHandNumber)(void);
  /* 本場を取得 */
  int32_t (*GetHonba)(void);
  /* 供託リーチ棒数を取得 */
  int32_t (*GetNumRiichibou)(void);
  /* 山の残り自摸牌を取得 */
  int32_t (*GetNumRemainTilesInDeck)(void);
  /* 指定した牌が場に出ている数の取得 */
  int32_t (*GetNumVisibleTiles)(MJTile tile);
};

/* プレーヤー生成コンフィグ */
struct MJPlayerConfig {
  const struct MJGameStateGetterInterface *game_state_getter_if;  /* 状態取得インターフェース */
};

/* プレーヤーインターフェース */
struct MJPlayerInterface {
  /* プレーヤーインターフェース名の取得 引数は無視してください */
  const char *(*GetName)(const MJPlayerInterfaceVersion3Tag *version_tag);
  /* ワークサイズ計算 */
  int32_t (*CalculateWorkSize)(const struct MJPlayerConfig *config);
  /* プレーヤーインスタンス作成 */
  void *(*Create)(const struct MJPlayerConfig *config, void *work, int32_t work_size);
  /* プレーヤーインスタンス破棄 */
  void (*Destroy)(void *player);
  /* 自分含む誰かのアクション時 */
  void (*OnAction)(void *player, 
      MJWind trigger_player, const struct MJPlayerAction *trigger_action,
      MJWind action_player, struct MJPlayerAction *action);
  /* 自摸時 */
  void (*OnDraw)(void *player, MJTile draw_tile, struct MJPlayerAction *player_action);
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
