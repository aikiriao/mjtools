#ifndef MJPLAYERWRAPPER_H_INCLUDED
#define MJPLAYERWRAPPER_H_INCLUDED

#include "mj_player_interface.h"
#include <stdint.h>

/* インスタンス生成コンフィグ */
struct MJPlayerWrapperConfig {
  const struct MJPlayerInterface *player_interface;   /* ユーザ定義のプレーヤーインターフェース */
  struct MJPlayerConfig player_config;                /* プレーヤー生成コンフィグ */
};

/* プレーヤー構造体 */
struct MJPlayerWrapper {
  const struct MJPlayerInterface *player_interface;
  void *player_instance;
  bool alloced_by_own;
  void *work;

  /* 公開メンバ */
  MJWind wind;          /* 家 */
  struct MJHand hand;   /* 手牌 */
  bool riichi;          /* リーチしたか？ */
  int32_t score;        /* 現在のスコア */
};

/* インターフェースラッパー */
/* プレーヤーハンドル作成に必要なワークサイズ計算 */
int32_t MJPlayerWrapper_CalculateWorkSize(const struct MJPlayerWrapperConfig *config);
/* プレーヤーハンドル作成 */
struct MJPlayerWrapper *MJPlayerWrapper_Create(const struct MJPlayerWrapperConfig *config, void *work, int32_t work_size);
/* プレーヤーハンドル破棄 */
void MJPlayerWrapper_Destroy(struct MJPlayerWrapper *player);
/* インターフェース名取得 */
const char *MJPlayerWrapper_GetName(const struct MJPlayerWrapper *player);
/* 誰かのアクション時 */
void MJPlayerWrapper_OnAction(struct MJPlayerWrapper *player, const struct MJPlayerAction *trigger_action, struct MJPlayerAction *action);
/* 自摸時 */
void MJPlayerWrapper_OnDiscard(struct MJPlayerWrapper *player, MJTile draw_tile, struct MJPlayerAction *player_action);
/* 局開始時 */
void MJPlayerWrapper_OnStartHand(struct MJPlayerWrapper *player, int32_t hand_no, MJWind player_wind);
/* 局終了時 */
void MJPlayerWrapper_OnEndHand(struct MJPlayerWrapper *player, MJHandEndReason reason, const int32_t *score_diff);
/* 対戦開始時 */
void MJPlayerWrapper_OnStartGame(struct MJPlayerWrapper *player);
/* 対戦終了時 */
void MJPlayerWrapper_OnEndGame(struct MJPlayerWrapper *player, int32_t player_rank, int32_t player_score);

#endif /* MJPLAYERWRAPPER_H_INCLUDED */
