#include "mj_player_cui.h"

#include "mj_utility.h"
#include "mj_shanten.h"

#include <stdlib.h>
#include <assert.h>
#include <curses.h>
#include <stdarg.h>
#include <string.h>

/* メモリアラインメント */
#define MJPLAYER_CUI_ALIGNMENT 16
/* メッセージバッファの縦幅 */
#define MJPLAYER_CUI_MESSAGE_WINDOW_HEIGHT      16
/* メッセージバッファの行サイズ */
#define MJPLAYER_CUI_MESSAGE_BUFFER_LINE_SIZE   256

/* CUIクライアント */
struct MJPlayerCUI {
  MJWind        wind;       /* 家 */
  struct MJHand hand;       /* 手牌 */
  bool          riichi;     /* リーチしたか？ */
  int32_t       rel_score;  /* 相対スコア */
  int32_t       total_rank;   /* 合計ランク */
  int64_t       total_score;  /* 合計スコア */
  const struct MJGameStateGetterInterface *game_state_getter; /* ゲーム状態取得インターフェース */
};

/* ウィンドウ情報 */
struct MJPlayerCUIWindow {
  WINDOW *window; /* ウィンドウハンドル */
  int width;      /* ウィンドウ幅 */
  int height;     /* ウィンドウ高さ */
  char message_buffer[MJPLAYER_CUI_MESSAGE_WINDOW_HEIGHT][MJPLAYER_CUI_MESSAGE_BUFFER_LINE_SIZE]; /* メッセージバッファ */
  int32_t message_buffer_line_pos; /* メッセージウインドウのバッファ先頭 */
  char white_message[MJPLAYER_CUI_MESSAGE_BUFFER_LINE_SIZE]; /* メッセージ消去用の何も書いてない文字列 */
};

/* ウィンドウ初期化処理 */
static void MJPlayerCUI_InitializeWindow(void);
/* ウィンドウ終了処理 */
static void MJPlayerCUI_FinalizeWindow(void);

/* yes or no のアクション取得 */
static bool MJPlayerCUI_GetBoolean(const char *message, bool default_key);
/* 状態表示 */
static void MJPlayerCUI_DrawGameStatus(const struct MJPlayerCUI *player);
/* メッセージウインドウ描画 */
static void MJPlayerCUI_DrawMessageWindow(void);
/* メッセージを追記 */
static void MJPlayerCUI_PutMessage(const char *format, ...);

/* アクションメッセージの追記 */
static void MJPlayerCUI_PutActionMessage(const struct MJPlayerAction *action);
/* アクション時の対応 */
static void MJPlayerCUI_OnActionCore(const struct MJPlayerCUI *player,
    const struct MJPlayerAction *trigger_action, struct MJPlayerAction *action);
/* 捨て牌選択 */
static void MJPlayerCUI_SelectDiscardTile(const struct MJPlayerCUI *player,
    MJTile draw_tile, struct MJPlayerAction *action);

/* インターフェース名の取得 */
static const char *MJPlayerCUI_GetName(const void *player, const MJPlayerInterfaceVersion5Tag *version_tag);
/* ワークサイズ計算 */
static int32_t MJPlayerCUI_CalculateWorkSize(const struct MJPlayerConfig *config);
/* インスタンス生成 */
static void *MJPlayerCUI_Create(const struct MJPlayerConfig *config, void *work, int32_t work_size);
/* インスタンス破棄 */
static void MJPlayerCUI_Destroy(void *player);
/* 誰かのアクション時の対応 */
static void MJPlayerCUI_OnAction(void *player, const struct MJPlayerAction *trigger_action, struct MJPlayerAction *action);
/* 自摸時の対応 */
static void MJPlayerCUI_OnDiscard(void *player, MJTile draw_tile, struct MJPlayerAction *player_action);
/* 局開始時の対応 */
static void MJPlayerCUI_OnStartHand(void *player, int32_t hand_no, MJWind player_wind);
/* 局終了時の対応 */
static void MJPlayerCUI_OnEndHand(void *player, MJHandEndReason reason, const int32_t *score_diff);
/* ゲーム開始時の対応 */
static void MJPlayerCUI_OnStartGame(void *player);
/* ゲーム終了時の対応 */
static void MJPlayerCUI_OnEndGame(void *player, int32_t player_rank, int32_t player_score);

/* インターフェース定義 */
static const struct MJPlayerInterface st_cui_player_interface = {
  MJPlayerCUI_GetName,
  MJPlayerCUI_CalculateWorkSize,
  MJPlayerCUI_Create,
  MJPlayerCUI_Destroy,
  MJPlayerCUI_OnAction,
  MJPlayerCUI_OnDiscard,
  MJPlayerCUI_OnStartHand,
  MJPlayerCUI_OnEndHand,
  MJPlayerCUI_OnStartGame,
  MJPlayerCUI_OnEndGame,
};

/* 初期化カウント */
static int32_t st_window_initialize_count = 0;

/* ウインドウ情報 */
static struct MJPlayerCUIWindow st_window = {
  .window = NULL,
  .width = 0,
  .height = 0,
  .message_buffer = { { 0, }, },
  .message_buffer_line_pos = 0,
  .white_message = { 0, },
};

/* 牌の文字列変換テーブル */
static const char *tile_string_table[MJTILE_MAX] = {
  "NA",
  "1M", "2M", "3M", "4M", "5M", "6M", "7M", "8M", "9M",
  "NA",
  "1P", "2P", "3P", "4P", "5P", "6P", "7P", "8P", "9P",
  "NA",
  "1S", "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S",
  "NA",
  "TO", "NA", "SH", "PE", "HK", "HT", "CN",
};

/* 家の文字列変換テーブル */
static const char *wind_sting_table[4] = { 
  "TON", "NAN", "SHA", "PEE"
};

/* アクションの文字列変換テーブル */
static const char *action_string_table[] = {
 "NONE", "TSUMO", "DISCARD", "PUNG",
 "CHOW1", "CHOW2", "CHOW3", "ANKAN",
 "MINKAN", "KAKAN", "RIICHI", "RON",
 "DRAWN",
};

/* 局終了理由の文字列変換テーブル */
static const char *endreason_string_table[] = {
  "RON", "DRAWN", "MISTAKE",
};

/* インターフェース取得 */
const struct MJPlayerInterface *MJPlayerCUI_GetInterface(void)
{
  return &st_cui_player_interface;
}

/* ウィンドウ初期化処理 */
static void MJPlayerCUI_InitializeWindow(void)
{
  /* 初期化済みなら何もしない */
  if (st_window_initialize_count > 0) {
    return;
  }

  /* cursesの初期化 */
  st_window.window = initscr();
  /* 入力の終了にCR(RET)が必要 */
  crmode();
  /* 入力された文字をエコーバックする */
  echo();
  /* 入力のタイムアウトを負（入力を永遠に待機する）に */
  timeout(-1);
  /* 画面サイズ取得 */
  getmaxyx(st_window.window, st_window.height, st_window.width);
  assert(st_window.width < MJPLAYER_CUI_MESSAGE_BUFFER_LINE_SIZE);
  /* 行クリア用文字列作成 */
  memset(st_window.white_message, ' ', sizeof(char) * (size_t)st_window.width);
  st_window.white_message[st_window.width] = '\0';

  /* 初期化カウントを増加 */
  st_window_initialize_count++;
}

/* ウィンドウ終了処理 */
static void MJPlayerCUI_FinalizeWindow(void)
{
  /* 初期化していない/既に終了している場合は何もしない */
  if (st_window_initialize_count <= 0) {
    return;
  }

  /* 初期化カウントを減らす */
  st_window_initialize_count--;

  /* 最後の1回ではcursesに関するリソースを開放 */
  if (st_window_initialize_count == 0) {
    /* cursesの終了 */
    endwin();
    /* メンバをクリアしとく */
    st_window.window = NULL;
    st_window.height = 0;
    st_window.width = 0;
  }
}

/* yes or no のアクション取得 */
static bool MJPlayerCUI_GetBoolean(const char *message, bool default_key)
{
  int32_t key;
  bool ret;

  assert(message != NULL);

  /* yes or no の問い */
  do {
    MJPlayerCUI_PutMessage("%s? [%s]", message, default_key ? "Y/n" : "y/N");
    MJPlayerCUI_DrawMessageWindow();
    /* キー取得 */
    key = getch();
    if ((key == 'n') || (key == 'N')) {
      ret = false;
      break;
    } else if ((key == 'y') || (key == 'Y')) {
      ret = true;
      break;
    } else if (key == '\n') {
      ret = default_key;
      break;
    }
    /* 無効なキーならやり直し */
    MJPlayerCUI_PutMessage("[Client] Please answer y or n(or enter key).");
  } while (1);

  return ret;
}

/* 状態表示 */
static void MJPlayerCUI_DrawGameStatus(const struct MJPlayerCUI *player)
{
  int32_t i, j, col;
  struct MJHand hand;

  assert(player != NULL);

  /* 描画列を最上段にセット */
  col = 0;

  /* 局/本場/リーチ棒 */
  mvprintw(col++, 0, "Hand: %d Honba: %d Riichi: %d", 
      player->game_state_getter->GetHandNumber(),
      player->game_state_getter->GetHonba(),
      player->game_state_getter->GetNumRiichibou());

  /* 現得点表示 */
  for (i = 0; i < 4; i++) {
    mvprintw(col, 17 * i, "%3s%5s: %5d",
        wind_sting_table[i], ((MJWind)i == player->wind) ? "(you)" : "",
        player->game_state_getter->GetScore((MJWind)i));
  }
  col++;

  /* ドラ表示牌 */
  {
    struct MJDoraTile dora;
    mvprintw(col, 0, "Dora: ");
    player->game_state_getter->GetDora(&dora);
    for (i = 0; i < dora.num_dora; i++) {
      mvprintw(col, 6 + 2 * i, "%2s", tile_string_table[dora.omote[i]]);
    }
    col++;
  }

  /* 残り山牌数表示 */
  mvprintw(col++, 0, "Remain Tiles: %2d", player->game_state_getter->GetNumRemainTilesInDeck());

  /* 河表示 */
  for (i = 0; i < 4; i++) {
    struct MJPlayerRiver river;
    player->game_state_getter->GetRiver((MJWind)i, &river);
    mvprintw(col, 0, "%3s", wind_sting_table[i]);
    for (j = 0; j < river.num_tiles; j++) {
      mvprintw(col, 4 + 2 * j, "%2s", tile_string_table[river.tiles[j].tile]);
    }
    col++;
  }

  /* 手牌表示 */
  player->game_state_getter->GetHand(player, player->wind, &hand);
  /* 純手牌 */
  mvprintw(col, 0, "Hand: ");
  for (i = 0; i < hand.num_hand_tiles; i++) {
    mvprintw(col, 6 + 2 * i, tile_string_table[hand.hand[i]]);
  }
  col++;
  /* 副露 */
  mvprintw(col, 0, "Meld: ");
  for (i = 0; i < hand.num_meld; i++) {
    int32_t num_tiles;
    switch (hand.meld[i].type) {
      case MJMELD_TYPE_MINKAN: case MJMELD_TYPE_ANKAN: case MJMELD_TYPE_KAKAN:
        num_tiles = 4;
        break;
      case MJMELD_TYPE_PUNG: case MJMELD_TYPE_CHOW:
        num_tiles = 3;
        break;
      default:
        assert(0);
    }
    for (j = 0; j < num_tiles; j++) {
      mvprintw(col, 6 + 8 * i + 2 * j, "%2s", tile_string_table[hand.meld[i].tiles[j]]);
    }
  }
  col++;
}

/* メッセージウインドウ描画 */
static void MJPlayerCUI_DrawMessageWindow(void)
{
  int32_t i;

  /* メッセージ欄 */
  mvprintw(st_window.height - MJPLAYER_CUI_MESSAGE_WINDOW_HEIGHT - 1, 0, "Messages -------------------------------- ");
  /* メッセージ一覧表示 */
  for (i = 0; i < MJPLAYER_CUI_MESSAGE_WINDOW_HEIGHT; i++) {
    int32_t col = st_window.height - i - 1;
    int32_t buffer_pos = (st_window.message_buffer_line_pos + i) % MJPLAYER_CUI_MESSAGE_WINDOW_HEIGHT;
    /* 末尾の文字列の残骸を消すため、一回行を消してから描画 */
    mvprintw(col, 0, st_window.white_message);
    mvprintw(col, 0, "%s", st_window.message_buffer[buffer_pos]);
  }
}

/* メッセージを追記 */
static void MJPlayerCUI_PutMessage(const char *format, ...)
{
  va_list ap;

  /* clang環境ではformatが文字列リテラルでないと警告を発するため、pragmaで回避 */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"

  /* メッセージバッファに文字列追記 */
  va_start(ap, format);
  vsnprintf(
      st_window.message_buffer[st_window.message_buffer_line_pos], MJPLAYER_CUI_MESSAGE_BUFFER_LINE_SIZE,
      format, ap);
  va_end(ap);

#pragma clang diagnostic pop

  /* バッファ参照位置更新 */
  st_window.message_buffer_line_pos
    = (st_window.message_buffer_line_pos + 1) % MJPLAYER_CUI_MESSAGE_WINDOW_HEIGHT;
}

/* アクションメッセージの追記 */
static void MJPlayerCUI_PutActionMessage(const struct MJPlayerAction *action)
{
  assert(action != NULL);

  MJPlayerCUI_PutMessage("[%3s] %s %2s", 
      wind_sting_table[action->player], action_string_table[action->type], tile_string_table[action->tile]);
}

/* アクション時の対応 */
static void MJPlayerCUI_OnActionCore(const struct MJPlayerCUI *player, const struct MJPlayerAction *trigger_action, struct MJPlayerAction *action)
{
  assert(player != NULL);
  assert(trigger_action != NULL);
  assert(action != NULL);

  /* ロン/鳴き */
  if ((trigger_action->type == MJPLAYER_ACTIONTYPE_DISCARD)
      || (trigger_action->type == MJPLAYER_ACTIONTYPE_RIICHI)) {
    /* 手牌をカウントに直す */
    struct MJTileCount tile_count;
    MJShanten_ConvertHandToTileCount(&player->hand, &tile_count);
    tile_count.count[trigger_action->tile]++;

    /* ロン */
    if ((MJShanten_CalculateShanten(&tile_count) == -1)
        && (player->game_state_getter->GetAgariScore(player, &player->hand, trigger_action->tile) > 0)) {
      if (MJPlayerCUI_GetBoolean("Ron", false)) {
        action->type = MJPLAYER_ACTIONTYPE_RON;
        action->tile = trigger_action->tile;
        return;
      }
    }

    /* カン */
    if (tile_count.count[trigger_action->tile] == 4) {
      if (MJPlayerCUI_GetBoolean("Kan", false)) {
        action->type = MJPLAYER_ACTIONTYPE_MINKAN;
        action->tile = trigger_action->tile;
        return;
      }
    }

    /* ポン */
    if (tile_count.count[trigger_action->tile] >= 3) {
      if (MJPlayerCUI_GetBoolean("Pung", false)) {
        action->type = MJPLAYER_ACTIONTYPE_PUNG;
        action->tile = trigger_action->tile;
        return;
      }
    }

    /* チー */
    if (MJTILE_IS_SUHAI(trigger_action->tile)
        && (((player->wind == MJWIND_TON) && (trigger_action->player == MJWIND_PEE))
        || ((player->wind == MJWIND_NAN) && (trigger_action->player == MJWIND_TON))
        || ((player->wind == MJWIND_SHA) && (trigger_action->player == MJWIND_NAN))
        || ((player->wind == MJWIND_PEE) && (trigger_action->player == MJWIND_SHA)))) {
      if (!MJTILE_NUMBER_IS(trigger_action->tile, 9)
          && (tile_count.count[trigger_action->tile + 1] > 0)
          && (tile_count.count[trigger_action->tile + 2] > 0)) {
        if (MJPlayerCUI_GetBoolean("Chow in low", false)) {
          action->type = MJPLAYER_ACTIONTYPE_CHOW1;
          action->tile = trigger_action->tile;
          return;
        }
      }
      if ((tile_count.count[trigger_action->tile - 1] > 0)
          && (tile_count.count[trigger_action->tile + 1] > 0)) {
        if (MJPlayerCUI_GetBoolean("Chow in middle", false)) {
          action->type = MJPLAYER_ACTIONTYPE_CHOW2;
          action->tile = trigger_action->tile;
          return;
        }
      }
      if (!MJTILE_NUMBER_IS(trigger_action->tile, 1)
          && (tile_count.count[trigger_action->tile - 2] > 0)
          && (tile_count.count[trigger_action->tile - 1] > 0)) {
        if (MJPlayerCUI_GetBoolean("Chow in high", false)) {
          action->type = MJPLAYER_ACTIONTYPE_CHOW3;
          action->tile = trigger_action->tile;
          return;
        }
      }
    }
  }
}

/* 捨て牌選択 */
static void MJPlayerCUI_SelectDiscardTile(const struct MJPlayerCUI *player, MJTile draw_tile, struct MJPlayerAction *action)
{
  int32_t i;
  MJTile discard_tile;
  static const char command_list[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'q', 'w', 'e' };
  struct MJTileCount tile_count;
  char line_string_buffer[MJPLAYER_CUI_MESSAGE_BUFFER_LINE_SIZE];

  assert(player != NULL);
  assert(action != NULL);

  /* カウントに直す */
  MJShanten_ConvertHandToTileCount(&player->hand, &tile_count);
  if (draw_tile != MJTILE_INVALID) {
    tile_count.count[draw_tile]++;
  }

  /* 自摸和了判定 */
  if (MJShanten_CalculateShanten(&tile_count) == -1) {
    if (MJPlayerCUI_GetBoolean("Tsumo", false)) {
      action->tile = draw_tile;
      action->type = MJPLAYER_ACTIONTYPE_TSUMO;
      return;
    }
  }

  /* リーチ状態ならツモ切り */
  if (player->riichi) {
    assert(MJTILE_IS_VALID(draw_tile));
    action->tile = draw_tile;
    action->type = MJPLAYER_ACTIONTYPE_DISCARD;
    return;
  }

  /* 捨て牌選択 */
  discard_tile = MJTILE_INVALID;
  do {
    int32_t key;
    char str_buf[256];

    /* 選択肢の表示 */
    line_string_buffer[0] = '\0';
    strncat(line_string_buffer, "Discard:", MJPLAYER_CUI_MESSAGE_BUFFER_LINE_SIZE);
    /* 捨てられる牌の文字列を作成 */
    for (i = 0; i < player->hand.num_hand_tiles; i++) {
      snprintf(str_buf, sizeof(str_buf), "%c)%2s,", command_list[i], tile_string_table[player->hand.hand[i]]);
      strncat(line_string_buffer, str_buf, MJPLAYER_CUI_MESSAGE_BUFFER_LINE_SIZE);
    }
    if (draw_tile != MJTILE_INVALID) {
      snprintf(str_buf, sizeof(str_buf), "ent)%2s ", tile_string_table[draw_tile]);
      strncat(line_string_buffer, str_buf, MJPLAYER_CUI_MESSAGE_BUFFER_LINE_SIZE);
    }
    MJPlayerCUI_PutMessage(line_string_buffer);
    MJPlayerCUI_DrawMessageWindow();

    /* 入力取得 */
    key = getch();
    if (key == '\n') {
      discard_tile = draw_tile;
    } else {
      for (i = 0; i < player->hand.num_hand_tiles; i++) {
        if (key == command_list[i]) {
          discard_tile = player->hand.hand[i];
          break;
        }
      }
    }
    if (discard_tile != MJTILE_INVALID) {
      break;
    }

    /* 無効なキー入力だったらメッセージ表示して再入力を促す */
    MJPlayerCUI_PutMessage("[Client] Invalid command; please hit key again.");
  } while (1);

  action->tile = discard_tile;
  action->type = MJPLAYER_ACTIONTYPE_DISCARD;

  /* 捨てた後の手牌が聴牌だったらリーチをかけるか尋ねる */
  if (player->hand.num_meld == 0) {
    MJShanten_ConvertHandToTileCount(&player->hand, &tile_count);
    tile_count.count[draw_tile]++;
    tile_count.count[discard_tile]--;
    if (MJShanten_CalculateShanten(&tile_count) == 0) {
      if (MJPlayerCUI_GetBoolean("Riichi", false)) {
        action->type = MJPLAYER_ACTIONTYPE_RIICHI;
      }
    }
  }
}

/* インターフェース名の取得 */
static const char *MJPlayerCUI_GetName(const void *player, const MJPlayerInterfaceVersion5Tag *version_tag)
{
  MJUTILITY_UNUSED_ARGUMENT(player);
  MJUTILITY_UNUSED_ARGUMENT(version_tag);
  return "CUI-Client";
}

/* ワークサイズ計算 */
static int32_t MJPlayerCUI_CalculateWorkSize(const struct MJPlayerConfig *config)
{
  /* 引数チェック */
  if (config == NULL) {
    return -1;
  } else if (config->game_state_getter_if == NULL) {
    return -1;
  }

  return sizeof(struct MJPlayerCUI) + MJPLAYER_CUI_ALIGNMENT;
}

/* インスタンス生成 */
static void *MJPlayerCUI_Create(const struct MJPlayerConfig *config, void *work, int32_t work_size)
{
  struct MJPlayerCUI *player;
  uint8_t *work_ptr;

  /* 引数チェック */
  if ((work == NULL) || (work_size < MJPlayerCUI_CalculateWorkSize(config))) {
    return NULL;
  }

  /* メモリアラインメント */
  work_ptr = work;
  work_ptr = (uint8_t *)MJUTILITY_ROUND_UP((uintptr_t)work_ptr, MJPLAYER_CUI_ALIGNMENT);

  /* 構造体配置 */
  player = (struct MJPlayerCUI *)work_ptr;
  work_ptr += sizeof(struct MJPlayerCUI);

  /* トータル情報の初期化 */
  player->total_rank = 0;
  player->total_score = 0;

  /* 状態取得インターフェース取得 */
  player->game_state_getter = config->game_state_getter_if;

  /* ウィンドウ初期化 */
  MJPlayerCUI_InitializeWindow();

  return player;
}

/* インスタンス破棄 */
static void MJPlayerCUI_Destroy(void *player)
{
  MJUTILITY_UNUSED_ARGUMENT(player);

  /* ウィンドウ終了 */
  MJPlayerCUI_FinalizeWindow();
}

/* 誰かのアクション時の対応 */
static void MJPlayerCUI_OnAction(void *player, const struct MJPlayerAction *trigger_action, struct MJPlayerAction *action)
{
  struct MJPlayerCUI *cuic = (struct MJPlayerCUI *)player;

  MJUTILITY_UNUSED_ARGUMENT(trigger_action);

  /* デバッグ向けにアサート */
  assert(player != NULL);
  assert(trigger_action != NULL);
  assert(action != NULL);
  assert(cuic->wind == action->player);

  /* トリガーアクションの内容をメッセージに追記 */
  MJPlayerCUI_PutActionMessage(trigger_action);

  /* 自分自身のアクションの場合は何もしない */
  if (trigger_action->player == cuic->wind) {
    return;
  }

  /* 画面再描画 */
  clear();
  MJPlayerCUI_DrawGameStatus(player);
  MJPlayerCUI_DrawMessageWindow();

  /* 手牌取得 */
  cuic->game_state_getter->GetHand(player, cuic->wind, &cuic->hand);

  /* 対応するアクションを決定 */
  MJPlayerCUI_OnActionCore(cuic, trigger_action, action);
}

/* 自摸時の対応 */
static void MJPlayerCUI_OnDiscard(void *player, MJTile draw_tile, struct MJPlayerAction *player_action)
{
  struct MJPlayerCUI *cuic = (struct MJPlayerCUI *)player;

  assert(player != NULL);
  assert(player_action != NULL);

  assert(cuic->wind == player_action->player);
  
  /* 手牌取得 */
  cuic->game_state_getter->GetHand(player, cuic->wind, &cuic->hand);

  assert(cuic->hand.num_hand_tiles > 0);

  /* 画面再描画 */
  clear();
  MJPlayerCUI_DrawGameStatus(player);
  MJPlayerCUI_DrawMessageWindow();

  /* 捨て牌選択 */
  MJPlayerCUI_SelectDiscardTile(cuic, draw_tile, player_action);

  /* リーチした？ */
  if (player_action->type == MJPLAYER_ACTIONTYPE_RIICHI) {
    cuic->riichi = true;
  }

  /* アクションの内容をメッセージに追記 */
  MJPlayerCUI_PutActionMessage(player_action);
}

/* 局開始時の対応 */
static void MJPlayerCUI_OnStartHand(void *player, int32_t hand_no, MJWind player_wind)
{
  struct MJPlayerCUI *cuic = (struct MJPlayerCUI *)player;

  MJUTILITY_UNUSED_ARGUMENT(hand_no);

  assert(player != NULL);

  cuic->wind = player_wind;

  /* リーチフラグをクリア */
  cuic->riichi = false;

  /* 局開始メッセージを追記 */
  MJPlayerCUI_PutMessage("Hand %d start. Player wind: %3s", hand_no, wind_sting_table[player_wind]);
}

/* 局終了時の対応 */
static void MJPlayerCUI_OnEndHand(void *player, MJHandEndReason reason, const int32_t *score_diff)
{
  struct MJPlayerCUI *cuic = (struct MJPlayerCUI *)player;

  MJUTILITY_UNUSED_ARGUMENT(reason);

  assert(player != NULL);

  /* 自分の相対スコア変動を記録 */
  cuic->rel_score += score_diff[cuic->wind];

  /* 局終了メッセージを追記 */
  MJPlayerCUI_PutMessage("Hand end. Reason:%s [TON]%7d [NAN]%7d [SHA]%7d [PEE]%7d",
      endreason_string_table[reason],
      score_diff[0], score_diff[1], score_diff[2], score_diff[3]);
}

/* ゲーム開始時の対応 */
static void MJPlayerCUI_OnStartGame(void *player)
{
  struct MJPlayerCUI *cuic = (struct MJPlayerCUI *)player;

  /* 相対スコアを0に初期化 */
  cuic->rel_score = 0;
}

/* ゲーム終了時の対応 */
static void MJPlayerCUI_OnEndGame(void *player, int32_t player_rank, int32_t player_score)
{
  struct MJPlayerCUI *cuic = (struct MJPlayerCUI *)player;

  assert(player != NULL);

  cuic->total_rank += player_rank;
  cuic->total_score += player_score;
}
