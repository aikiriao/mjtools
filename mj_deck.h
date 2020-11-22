#ifndef MJDECK_H_INCLUDED
#define MJDECK_H_INCLUDED

#include "mj_random.h"
#include <stdint.h>

/* 牌山ハンドル */
struct MJDeck;

/* インスタンス生成コンフィグ */
struct MJDeckConfig {
  const struct MJRandomGeneratorInterface *random_if;  /* 乱数生成インターフェース(NULLでデフォルトの乱数生成器を使用) */
};

/* ドラ牌 */
struct MJDoraHai {
  uint8_t   omote[5]; /* 表ドラ */
  uint8_t   ura[5];   /* 裏ドラ */
  uint32_t  num_dora; /* ドラ表示枚数 */
};

/* API結果型 */
typedef enum MJDeckApiResultTag {
  MJDECK_APIRESULT_OK = 0,            /* 成功 */
  MJDECK_APIRESULT_INVALID_ARGUMENT,  /* 不正な引数 */
  MJDECK_APIRESULT_EMPTY_DECK,        /* 牌山が空 */
  MJDECK_APIRESULT_NOT_SHUFFLED,      /* 牌山がシャッフルされてない */
  MJDECK_APIRESULT_NG,
} MJDeckApiResult;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 牌山ハンドル作成に必要なワークサイズの計算 */
int32_t MJDeck_CalculateWorkSize(const struct MJDeckConfig *config);

/* 牌山ハンドルの生成 */
struct MJDeck *MJDeck_Create(const struct MJDeckConfig *config, void *work, int32_t work_size);

/* 牌山ハンドルの破棄 */
void MJDeck_Destroy(struct MJDeck *deck);

/* ドラ表示牌の取得 */
MJDeckApiResult MJDeck_GetDoraHai(const struct MJDeck *deck, struct MJDoraHai *dora);

/* 残り自摸牌数の取得 */
MJDeckApiResult MJDeck_GetNumRemainHais(const struct MJDeck *deck, uint32_t *num_rest_hais);

/* 残り嶺上数の取得 */
MJDeckApiResult MJDeck_GetNumRemainRinshanHais(const struct MJDeck *deck, uint32_t *num_rest_hais);

/* 乱数シードのセット */
MJDeckApiResult MJDeck_SetRandomSeed(struct MJDeck *deck, const void *seed);

/* 洗牌 */
MJDeckApiResult MJDeck_Shuffle(struct MJDeck *deck);

/* 1枚ツモる */
MJDeckApiResult MJDeck_Tsumo(struct MJDeck *deck, uint8_t *hai);

/* 1枚嶺上牌からツモる（ドラ表示が増える） */
MJDeckApiResult MJDeck_RinshanTsumo(struct MJDeck *deck, uint8_t *hai);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MJDECK_H_INCLUDED */