#include "mj_deck.h"
#include "mj_tile.h"
#include "mj_random_xoshiro256pp.h"
#include "mj_utility.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

/* アラインメント */
#define MJDECK_ALIGNMENT  16

/* 配山の牌数 */
#define MJDECK_NUM_TILES  136

/* 牌山ハンドル */
struct MJDeck {
  MJTile            deck[122];      /* 牌山 */
  MJTile            rinshan[4];     /* 嶺上牌 */
  int32_t           tsumo_pos;      /* ツモ位置 */
  int32_t           rinshan_pos;    /* 嶺上ツモ位置 */
  struct MJDoraTile dora;           /* ドラ牌 */
  MJTile            unshuffled_deck[MJDECK_NUM_TILES];  /* シャッフル前の牌山 */
  void              *random;        /* 乱数生成インスタンス */
  const struct MJRandomGeneratorInterface *random_if; /* 乱数生成インターフェース */
  bool              shufffled;      /* シャッフル済み？ */
  bool              alloced_by_own; /* メモリは自前確保か？ */
  void              *work;          /* ワークメモリ先頭アドレス */
};

/* シャッフル/赤ドラ注入前の牌山 */
static const MJTile initial_deck[MJDECK_NUM_TILES] = {
  MJTILE_1MAN, MJTILE_1MAN, MJTILE_1MAN, MJTILE_1MAN,
  MJTILE_2MAN, MJTILE_2MAN, MJTILE_2MAN, MJTILE_2MAN,
  MJTILE_3MAN, MJTILE_3MAN, MJTILE_3MAN, MJTILE_3MAN,
  MJTILE_4MAN, MJTILE_4MAN, MJTILE_4MAN, MJTILE_4MAN,
  MJTILE_5MAN, MJTILE_5MAN, MJTILE_5MAN, MJTILE_5MAN,
  MJTILE_6MAN, MJTILE_6MAN, MJTILE_6MAN, MJTILE_6MAN,
  MJTILE_7MAN, MJTILE_7MAN, MJTILE_7MAN, MJTILE_7MAN,
  MJTILE_8MAN, MJTILE_8MAN, MJTILE_8MAN, MJTILE_8MAN,
  MJTILE_9MAN, MJTILE_9MAN, MJTILE_9MAN, MJTILE_9MAN,
  MJTILE_1PIN, MJTILE_1PIN, MJTILE_1PIN, MJTILE_1PIN,
  MJTILE_2PIN, MJTILE_2PIN, MJTILE_2PIN, MJTILE_2PIN,
  MJTILE_3PIN, MJTILE_3PIN, MJTILE_3PIN, MJTILE_3PIN,
  MJTILE_4PIN, MJTILE_4PIN, MJTILE_4PIN, MJTILE_4PIN,
  MJTILE_5PIN, MJTILE_5PIN, MJTILE_5PIN, MJTILE_5PIN,
  MJTILE_6PIN, MJTILE_6PIN, MJTILE_6PIN, MJTILE_6PIN,
  MJTILE_7PIN, MJTILE_7PIN, MJTILE_7PIN, MJTILE_7PIN,
  MJTILE_8PIN, MJTILE_8PIN, MJTILE_8PIN, MJTILE_8PIN,
  MJTILE_9PIN, MJTILE_9PIN, MJTILE_9PIN, MJTILE_9PIN,
  MJTILE_1SOU, MJTILE_1SOU, MJTILE_1SOU, MJTILE_1SOU,
  MJTILE_2SOU, MJTILE_2SOU, MJTILE_2SOU, MJTILE_2SOU,
  MJTILE_3SOU, MJTILE_3SOU, MJTILE_3SOU, MJTILE_3SOU,
  MJTILE_4SOU, MJTILE_4SOU, MJTILE_4SOU, MJTILE_4SOU,
  MJTILE_5SOU, MJTILE_5SOU, MJTILE_5SOU, MJTILE_5SOU,
  MJTILE_6SOU, MJTILE_6SOU, MJTILE_6SOU, MJTILE_6SOU,
  MJTILE_7SOU, MJTILE_7SOU, MJTILE_7SOU, MJTILE_7SOU,
  MJTILE_8SOU, MJTILE_8SOU, MJTILE_8SOU, MJTILE_8SOU,
  MJTILE_9SOU, MJTILE_9SOU, MJTILE_9SOU, MJTILE_9SOU,
  MJTILE_TON, MJTILE_TON, MJTILE_TON, MJTILE_TON, 
  MJTILE_NAN, MJTILE_NAN, MJTILE_NAN, MJTILE_NAN, 
  MJTILE_SHA, MJTILE_SHA, MJTILE_SHA, MJTILE_SHA, 
  MJTILE_PEE, MJTILE_PEE, MJTILE_PEE, MJTILE_PEE, 
  MJTILE_HAKU, MJTILE_HAKU, MJTILE_HAKU, MJTILE_HAKU, 
  MJTILE_HATU, MJTILE_HATU, MJTILE_HATU, MJTILE_HATU, 
  MJTILE_CHUN, MJTILE_CHUN, MJTILE_CHUN, MJTILE_CHUN, 
};

/* 赤ドラの設定をチェック true:OK, false:NG */
static bool MJDeck_CheckAkadoraCount(const int32_t *akadora_count)
{
  int32_t t;

  /* 引数チェック */
  if (akadora_count == NULL) {
    return false;
  }

  /* 赤ドラの設定をチェック */
  for (t = 0; t < MJTILE_MAX; t++) {
    if ((akadora_count[t] < 0) || (akadora_count[t] > 4)) {
      return false;
    }
  }

  return true;
}

/* 赤ドラ注入 */
static void MJDeck_PutAkadoraToDeck(const int32_t *akadora_count, MJTile *deck)
{
  int32_t t, i;
  int32_t count;

  assert((akadora_count != NULL) && (deck != NULL));

  /* 赤ドラ枚数分だけフラグを立てる */
  for (t = 0; t < MJTILE_MAX; t++) {
    count = akadora_count[t];
    for (i = 0; i < MJDECK_NUM_TILES; i++) {
      if (count == 0) {
        break;
      }
      if ((deck[i] == t) && !MJTILE_IS_AKADORA(deck[i])) {
        deck[i] |= MJTILE_FLAG_AKADORA;
        count--;
      }
    }
  }
}

/* デフォルトのコンフィグを設定 */
void MJDeck_SetDefaultConfig(struct MJDeckConfig *config)
{
  assert(config != NULL);

  /* デフォルトの乱数生成器のインターフェースをセット */
  config->random_if = MJRandomXoshiro256pp_GetInterface();

  /* 赤ドラ枚数設定 */
  memset(config->akadora_count, 0, sizeof(int32_t) * MJTILE_MAX);
}

/* 牌山ハンドル作成に必要なワークサイズの計算 */
int32_t MJDeck_CalculateWorkSize(const struct MJDeckConfig *config)
{
  int32_t work_size;
  struct MJDeckConfig apply_config;

  /* コンフィグが指定されなければデフォルトコンフィグに差し替え */
  if (config == NULL) {
    MJDeck_SetDefaultConfig(&apply_config);
  } else {
    apply_config = (*config);
  }

  /* 乱数生成インターフェースのチェック */
  if (apply_config.random_if == NULL) {
    return -1;
  }

  /* 赤ドラ枚数設定チェック */
  if (!MJDeck_CheckAkadoraCount(apply_config.akadora_count)) {
    return -1;
  }

  /* ハンドルサイズ */
  work_size = sizeof(struct MJDeck) + MJDECK_ALIGNMENT;

  /* 乱数生成器のワークサイズを加算 */
  work_size += apply_config.random_if->CalculateWorkSize();

  return work_size;
}

/* 牌山ハンドルの生成 */
struct MJDeck *MJDeck_Create(const struct MJDeckConfig *config, void *work, int32_t work_size)
{
  struct MJDeck *deck;
  uint8_t *work_ptr;
  bool tmp_alloced_by_own = false;
  struct MJDeckConfig apply_config;

  /* コンフィグが指定されなければデフォルトコンフィグに差し替え */
  if (config == NULL) {
    MJDeck_SetDefaultConfig(&apply_config);
  } else {
    apply_config = (*config);
  }

  /* 領域自前確保の場合 */
  if ((work == NULL) && (work_size == 0)) {
    if ((work_size = MJDeck_CalculateWorkSize(&apply_config)) < 0) {
      return NULL;
    }
    work = malloc((size_t)work_size);
    tmp_alloced_by_own = true;
  }

  /* 引数チェック */
  if ((work == NULL) || (work_size < MJDeck_CalculateWorkSize(&apply_config))) {
    return NULL;
  }

  /* 乱数生成インターフェースのチェック */
  if (apply_config.random_if == NULL) {
    return NULL;
  }

  /* 赤ドラ枚数設定チェック */
  if (!MJDeck_CheckAkadoraCount(apply_config.akadora_count)) {
    if (tmp_alloced_by_own) {
      free(work);
    }
    return NULL;
  }

  /* アラインメント調整 */
  work_ptr = (uint8_t *)work;
  work_ptr = (uint8_t *)MJUTILITY_ROUND_UP((uintptr_t)work_ptr, MJDECK_ALIGNMENT);

  /* インスタンス領域の確保 */
  deck = (struct MJDeck *)work_ptr;
  work_ptr += sizeof(struct MJDeck);
  /* メモリ先頭アドレスを記録 */
  deck->work = work;
  /* 自前確保フラグを記録 */
  deck->alloced_by_own = tmp_alloced_by_own;
  /* 乱数生成インターフェースの記録 */
  deck->random_if = apply_config.random_if;
  /* シャッフル済フラグは落とす */
  deck->shufffled = false;

  /* 乱数生成器の作成 */
  deck->random = apply_config.random_if->Create(work_ptr, apply_config.random_if->CalculateWorkSize());
  if (deck->random == NULL) {
    return NULL;
  }
  work_ptr += apply_config.random_if->CalculateWorkSize();

  /* 初期の配山をコピー */
  memcpy(deck->unshuffled_deck, initial_deck, sizeof(deck->unshuffled_deck));

  /* 赤ドラ注入 */
  MJDeck_PutAkadoraToDeck(apply_config.akadora_count, deck->unshuffled_deck);

  return deck;
}

/* 牌山ハンドルの破棄 */
void MJDeck_Destroy(struct MJDeck *deck)
{
  if (deck != NULL) {
    /* 乱数生成器の破棄 */
    deck->random_if->Destroy(deck->random);
    /* 自分で領域確保していたら領域解放 */
    if (deck->alloced_by_own) {
      free(deck->work);
    }
  }
}

/* ドラ表示牌の取得 */
MJDeckApiResult MJDeck_GetDoraTile(const struct MJDeck *deck, struct MJDoraTile *dora)
{
  /* 引数チェック */
  if ((deck == NULL) || (dora == NULL)) {
    return MJDECK_APIRESULT_INVALID_ARGUMENT;
  }

  /* 洗牌前は呼び出し不可 */
  if (!deck->shufffled) {
    return MJDECK_APIRESULT_NOT_SHUFFLED;
  }

  /* 成功終了 */
  (*dora) = deck->dora;
  return MJDECK_APIRESULT_OK;
}

/* 残り自摸牌数の取得 */
MJDeckApiResult MJDeck_GetNumRemainTiles(const struct MJDeck *deck, int32_t *num_remain_tiles)
{
  /* 引数チェック */
  if ((deck == NULL) || (num_remain_tiles == NULL)) {
    return MJDECK_APIRESULT_INVALID_ARGUMENT;
  }

  /* 洗牌前は呼び出し不可 */
  if (!deck->shufffled) {
    return MJDECK_APIRESULT_NOT_SHUFFLED;
  }

  assert((deck->tsumo_pos + deck->rinshan_pos) <= 122);

  /* 成功終了 */
  (*num_remain_tiles) = 122 - deck->tsumo_pos - deck->rinshan_pos;
  return MJDECK_APIRESULT_OK;
}

/* 残り嶺上数の取得 */
MJDeckApiResult MJDeck_GetNumRemainRinshanTiles(const struct MJDeck *deck, int32_t *num_remain_tiles)
{
  /* 引数チェック */
  if ((deck == NULL) || (num_remain_tiles == NULL)) {
    return MJDECK_APIRESULT_INVALID_ARGUMENT;
  }

  /* 洗牌前は呼び出し不可 */
  if (!deck->shufffled) {
    return MJDECK_APIRESULT_NOT_SHUFFLED;
  }

  assert(deck->rinshan_pos <= 4);

  /* 成功終了 */
  (*num_remain_tiles) = 4 - deck->rinshan_pos;
  return MJDECK_APIRESULT_OK;
}

/* 乱数シードのセット */
MJDeckApiResult MJDeck_SetRandomSeed(struct MJDeck *deck, const void *seed)
{
  /* 引数チェック */
  if ((deck == NULL) || (seed == NULL)) {
    return MJDECK_APIRESULT_INVALID_ARGUMENT;
  }

  /* シード値をセット */
  deck->random_if->SetSeed(deck->random, seed);

  return MJDECK_APIRESULT_OK;
}

/* 洗牌 */
MJDeckApiResult MJDeck_Shuffle(struct MJDeck *deck)
{
  int32_t i;
  MJTile deck_work[MJDECK_NUM_TILES]; /* シャッフル用一時領域 */

  /* 引数チェック */
  if (deck == NULL) {
    return MJDECK_APIRESULT_INVALID_ARGUMENT;
  }

  /* シャッフル前の山をコピー */
  memcpy(deck_work, deck->unshuffled_deck, sizeof(deck_work));

  /* シャッフル（Fisher-Yatesのシャッフル） */
  for (i = MJDECK_NUM_TILES; i > 1; i--) {
    int32_t j;
    MJTile tmp;
    j = deck->random_if->GetRandom(deck->random, 0, i - 1);
    tmp = deck_work[i - 1];
    deck_work[i - 1] = deck_work[j];
    deck_work[j] = tmp;
  }

  /* 牌山/嶺上牌/ドラ牌のセット */
  memcpy(deck->deck,        &deck_work[  0],  sizeof(MJTile) * 122);
  memcpy(deck->rinshan,     &deck_work[122],  sizeof(MJTile) *   4);
  memcpy(deck->dora.omote,  &deck_work[126],  sizeof(MJTile) *   5);
  memcpy(deck->dora.ura,    &deck_work[131],  sizeof(MJTile) *   5);

  /* ツモ位置の初期化 */
  deck->tsumo_pos = 0;
  deck->rinshan_pos = 0;
  
  /* ドラ表示枚数の初期化 */
  deck->dora.num_dora = 1;

  /* シャッフル済みにセット */
  deck->shufffled = true;

  return MJDECK_APIRESULT_OK;
}

/* 1枚ツモる */
MJDeckApiResult MJDeck_Draw(struct MJDeck *deck, MJTile *tile)
{
  MJDeckApiResult ret;
  int32_t num_remain_tiles;

  /* 引数チェック */
  if ((deck == NULL) || (tile == NULL)) {
    return MJDECK_APIRESULT_INVALID_ARGUMENT;
  }

  /* 洗牌前は呼び出し不可 */
  if (!deck->shufffled) {
    return MJDECK_APIRESULT_NOT_SHUFFLED;
  }

  /* 残り牌数の取得 */
  if ((ret = MJDeck_GetNumRemainTiles(deck, &num_remain_tiles)) != MJDECK_APIRESULT_OK) {
    return ret;
  }
  /* 牌山が空 */
  if (num_remain_tiles == 0) {
    return MJDECK_APIRESULT_EMPTY_DECK;
  }

  /* 山から1枚取得 */
  (*tile) = deck->deck[deck->tsumo_pos];
  deck->tsumo_pos++;

  return MJDECK_APIRESULT_OK;
}

/* 1枚嶺上牌からツモる（ドラ表示が増える） */
MJDeckApiResult MJDeck_RinshanDraw(struct MJDeck *deck, MJTile *tile)
{
  MJDeckApiResult ret;
  int32_t num_remain_tiles;

  /* 引数チェック */
  if ((deck == NULL) || (tile == NULL)) {
    return MJDECK_APIRESULT_INVALID_ARGUMENT;
  }

  /* 洗牌前は呼び出し不可 */
  if (!deck->shufffled) {
    return MJDECK_APIRESULT_NOT_SHUFFLED;
  }

  /* 残り嶺上牌数の取得 */
  if ((ret = MJDeck_GetNumRemainRinshanTiles(deck, &num_remain_tiles)) != MJDECK_APIRESULT_OK) {
    return ret;
  }
  /* 牌山が空 */
  if (num_remain_tiles == 0) {
    return MJDECK_APIRESULT_EMPTY_DECK;
  }

  /* 最後の自摸の後に嶺上自摸はできない */
  if ((ret = MJDeck_GetNumRemainTiles(deck, &num_remain_tiles)) != MJDECK_APIRESULT_OK) {
    return ret;
  }
  if (num_remain_tiles == 0) {
    return MJDECK_APIRESULT_EMPTY_DECK;
  }

  /* 山から1枚取得 */
  (*tile) = deck->rinshan[deck->rinshan_pos];
  deck->rinshan_pos++;

  /* ドラ表示牌数増加 */
  deck->dora.num_dora++;

  return MJDECK_APIRESULT_OK;
}
