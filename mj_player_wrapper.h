#ifndef MJPLAYERWRAPPER_H_INCLUDED
#define MJPLAYERWRAPPER_H_INCLUDED

#include "mj_player_interface.h"
#include <stdint.h>

/* ユーザ定義のインターフェースを薄くラップして
 * ゲーム管理者側が使いやすく（MJPlayerに対して操作するだけでよく）するためのモジュール */

/* インスタンス生成コンフィグ */
struct MJPlayerWrapperConfig {
  const struct MJPlayerInterface *player_interface;   /* ユーザ定義のプレーヤーインターフェース */
  struct MJPlayerConfig player_config;                /* プレーヤー生成コンフィグ */
};

/* プレーヤーハンドル */
struct MJPlayerWrapper;

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
void MJPlayerWrapper_OnAction(struct MJPlayerWrapper *player, MJWind trigger_player, const struct MJPlayerAction *trigger_action, MJWind action_player, struct MJPlayerAction *action);
/* 自摸時 */
void MJPlayerWrapper_OnDraw(struct MJPlayerWrapper *player, MJTile draw_tile, struct MJPlayerAction *player_action);
/* 局開始時 */
void MJPlayerWrapper_OnStartHand(struct MJPlayerWrapper *player, int32_t hand_no, MJWind player_wind);
/* 局終了時 */
void MJPlayerWrapper_OnEndHand(struct MJPlayerWrapper *player, MJHandEndReason reason, const int32_t *score_diff);
/* 対戦開始時 */
void MJPlayerWrapper_OnStartGame(struct MJPlayerWrapper *player);
/* 対戦終了時 */
void MJPlayerWrapper_OnEndGame(struct MJPlayerWrapper *player, int32_t player_rank, int32_t player_score);

/* 固有関数 */
/* ハンドル取得 */
const void *MJPlayerWrapper_GetPlayerHandle(const struct MJPlayerWrapper *player);

#endif /* MJPLAYERWRAPPER_H_INCLUDED */
