#ifndef MJPLAYER_H_INCLUDED
#define MJPLAYER_H_INCLUDED

#include "mj_player_interface.h"
#include <stdint.h>

/* ユーザ定義のインターフェースを薄くラップして
 * ゲーム管理者側が使いやすく（MJPlayerに対して操作するだけでよく）するためのモジュール */

/* インスタンス生成コンフィグ */
struct MJPlayerConfig {
  const struct MJPlayerInterface *player_interface;  /* ユーザ定義のプレーヤーインターフェース */
};

/* プレーヤーハンドル */
struct MJPlayer;

/* プレーヤーハンドル作成に必要なワークサイズ計算 */
int32_t MJPlayer_CalculateWorkSize(const struct MJPlayerConfig *config);
/* プレーヤーハンドル作成 */
struct MJPlayer *MJPlayer_Create(const struct MJPlayerConfig *config, void *work, int32_t work_size);
/* プレーヤーハンドル破棄 */
void MJPlayer_Destroy(struct MJPlayer *player);
/* インターフェース名取得 */
const char *MJPlayer_GetName(const struct MJPlayer *player);
/* 誰かのアクション時 */
void MJPlayer_OnAction(struct MJPlayer *player, MJWind trigger_player, const struct MJPlayerAction *trigger_action, MJWind action_player, struct MJPlayerAction *action);
/* 自摸時 */
void MJPlayer_OnDraw(struct MJPlayer *player, const struct MJHand *hand, MJTile draw_tile, struct MJPlayerAction *player_action);
/* 局開始時 */
void MJPlayer_OnStartHand(struct MJPlayer *player, int32_t hand_no, MJWind player_wind);
/* 局終了時 */
void MJPlayer_OnEndHand(struct MJPlayer *player, MJHandEndReason reason, const int32_t *score_diff);
/* 対戦開始時 */
void MJPlayer_OnStartGame(struct MJPlayer *player);
/* 対戦終了時 */
void MJPlayer_OnEndGame(struct MJPlayer *player, int32_t player_rank, int32_t player_score);

#endif /* MJPLAYER_H_INCLUDED */
