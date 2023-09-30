#include "../../simulator/src/mj_deck.h"
#include "../../simulator/src/mj_river.h"
#include "../../simulator/src/mj_player_wrapper.h"
#include "mj_shanten.h"
#include "mj_score.h"
#include "mj_utility.h"
#include "mj_player_tsumogiri.h"
#include "mj_player_shantenman.h"
#include "mj_player_furoman.h"
#include "mj_player_cui.h"

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

/* デバッグ表示を有効にするか？ */
#define DEBUG_PRINT 0
/* 対局数 */
#define MAX_NUM_HANDS 10000

/* ゲーム状態変数 TODO:スコア, 手牌, リーチフラグはプレーヤーラッパーに含めてしまう */
static struct MJPlayerWrapper *players[4] = { NULL, NULL, NULL, NULL };
static struct MJDeck *deck = NULL;
static struct MJRiver river;
static int32_t hand_no = 0;

static MJWind MJGameStateGetterStub_SearchPlayerFromHandle(const void *player)
{
  int32_t i;

  /* ハンドル探索 */
  for (i = 0; i < 4; i++) {
    if (player == players[i]->player_instance) {
      break;
    }
  }
  assert((i >= 0) && (i < 4));

  return players[i]->wind;
}

/* ゲーム状態取得関数のスタブ */
static uint32_t MJGameStateGetterStub_GetVersion(const MJGameStateGetterInterfaceVersion3Tag *version_tag)
{
  MJUTILITY_UNUSED_ARGUMENT(version_tag);
  return MJGAMESTATEGETTER_INTERFACE_VERSION;
}

static void MJGameStateGetterStub_GetHand(const void *player, MJWind player_wind, struct MJHand *hand)
{
  MJWind caller_wind;

  /* 呼び出し側の家を特定 */
  caller_wind = MJGameStateGetterStub_SearchPlayerFromHandle(player);

  /* 手牌取得 */
  (*hand) = players[player_wind]->hand;

  /* 自分以外の場合は純手牌をINVALIDに書き換える */
  if (caller_wind != player_wind) {
    int32_t j;
    for (j = 0; j < 13; j++) {
      hand->hand[j] = MJTILE_INVALID;
    }
  }
}
static void MJGameStateGetterStub_GetRiver(MJWind player_wind, struct MJPlayerRiver *playe_river)
{
  assert(playe_river != NULL);
  (*playe_river) = river.player_river[player_wind];
}
static void MJGameStateGetterStub_GetDora(struct MJDoraTile *dora)
{
  int32_t i;
  struct MJDoraTile tmp;

  assert(dora != NULL);

  /* 裏ドラは隠す */
  MJDeck_GetDoraTile(deck, &tmp);
  for (i = 0; i < tmp.num_dora; i++) {
    tmp.ura[i] = MJTILE_INVALID;
  }

  (*dora) = tmp;
}
static int32_t MJGameStateGetterStub_GetScore(MJWind player)
{
  return players[player]->score;
}
static int32_t MJGameStateGetterStub_GetHandNumber(void)
{
  return hand_no;
}
static int32_t MJGameStateGetterStub_GetHonba(void)
{
  return 0;
}
static int32_t MJGameStateGetterStub_GetNumRiichibou(void)
{
  return 0;
}
static int32_t MJGameStateGetterStub_GetNumRemainTilesInDeck(void)
{
  int32_t ret;
  MJDeck_GetNumRemainTiles(deck, &ret);
  return ret;
}
static int32_t MJGameStateGetterStub_GetNumVisibleTiles(MJTile tile)
{
  MJUTILITY_UNUSED_ARGUMENT(tile);
  return 0;
}
static int32_t MJGameStateGetterStub_GetAgariScore(const void *player, const struct MJHand *hand, MJTile winning_tile)
{
  struct MJAgariInformation agari_info;
  struct MJScore score;
  MJScoreCalculationResult ret;

  /* 和了状況を作成 */
  agari_info.winning_tile = winning_tile;
  agari_info.hand = (*hand);
  agari_info.player_wind = MJGameStateGetterStub_SearchPlayerFromHandle(player);
  /* TODO: ここらへんは判断できる */
  agari_info.num_dora = 0;
  agari_info.num_honba = 0;
  agari_info.num_riichibo = 0;
  agari_info.round_wind = MJWIND_TON;
  /* 状況役は排す */
  agari_info.tsumo
    = agari_info.riichi
    = agari_info.ippatsu
    = agari_info.double_riichi
    = agari_info.haitei
    = agari_info.rinshan
    = agari_info.chankan
    = agari_info.nagashimangan
    = agari_info.tenho
    = agari_info.chiho = false;

  /* 得点計算 */
  if ((ret = MJScore_CalculateScore(&agari_info, &score)) == MJSCORE_CALCRESULT_OK) {
    return score.point.point;
  }

  /* 役がなかったりチョンボだったり */
  return 0;
}

/* ゲーム状態取得のインターフェースのスタブ */
static const struct MJGameStateGetterInterface st_stub_game_state_if = {
  MJGameStateGetterStub_GetVersion,
  MJGameStateGetterStub_GetHand,
  MJGameStateGetterStub_GetRiver,
  MJGameStateGetterStub_GetDora,
  MJGameStateGetterStub_GetScore,
  MJGameStateGetterStub_GetHandNumber,
  MJGameStateGetterStub_GetHonba,
  MJGameStateGetterStub_GetNumRiichibou,
  MJGameStateGetterStub_GetNumRemainTilesInDeck,
  MJGameStateGetterStub_GetNumVisibleTiles,
  MJGameStateGetterStub_GetAgariScore,
};

/* インデックスを家に変換 */
static const MJWind index_mjwind_table[4] = {
  MJWIND_TON, MJWIND_NAN, MJWIND_SHA, MJWIND_PEE
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

static const char *wind_sting_table[4] = { 
  "TON", "NAN", "SHA", "PEE"
};

/* 役名テーブル（IDに対応する文字列が並ぶ） */
static const char *yaku_name_table[] = {
  "RIICHI",           "DOUBLERIICHI",   "IPPATSU",      "TSUMO",
  "TANYAO",           "PINFU",          "IPEKO",        "BAKAZE",
  "JIKAZE",           "HAKU",           "HATU",         "CHUN",
  "RINSHAN",          "CHANKAN",        "HAITEITSUMO",  "HOUTEIRON",
  "SANSYOKUDOUJYUN",  "IKKITSUKAN",     "CHANTA",       "CHITOITSU",
  "TOITOIHO",         "SANANKO",        "HONROUTO",     "SANSYOKUDOUKOKU",
  "SANKANTSU",        "SYOSANGEN",      "HONITSU",      "JYUNCHANTA",
  "RYANPEKO",         "CHINITSU",       "TENHO",        "CHIHO",
  "KOKUSHIMUSOU",     "KOKUSHIMUSOU13", "CHURENPOUTON", "CHURENPOUTON9",
  "SUANKO",           "SUANKOTANKI",    "DAISUSHI",     "SYOSUSHI",
  "DAISANGEN",        "TSUISO",         "CHINROTO",     "RYUISO",
  "SUKANTSU",         "DORA",           "NAGASHIMANGAN",
};

/* 牌の比較関数 */
static int compare_tile(const void *a, const void *b)
{
  const int32_t ia = (int32_t)(*(MJTile *)a);
  const int32_t ib = (int32_t)(*(MJTile *)b);
  return ia - ib;
}

/* スコアの比較関数（降順） */
static int compare_score_dec(const void *a, const void *b)
{
  const struct MJPlayerWrapper **pa = (const struct MJPlayerWrapper **)a;
  const struct MJPlayerWrapper **pb = (const struct MJPlayerWrapper **)b;
  return (*pb)->score - (*pa)->score;
}

/* 手牌をソート */
static void sort_hand(struct MJHand *hand)
{
  assert(hand != NULL);
  qsort(hand->hand, (size_t)hand->num_hand_tiles, sizeof(MJTile), compare_tile);
}

/* 河の表示 */
static void print_river(const struct MJRiver *river)
{
#if DEBUG_PRINT
  int32_t i;

  assert(river != NULL);

  for (i = 0; i < 4; i++) {
    int32_t j;
    const struct MJPlayerRiver *priver = &(river->player_river[i]);
    printf("%s river: ", wind_sting_table[i]);
    for (j = 0; j < priver->num_tiles; j++) {
      printf("%s,", tile_string_table[priver->tiles[j].tile]);
    }
    printf("\n");
  }
#endif
}

/* 和了役・得点の表示 */
static void print_score(const struct MJScore *score)
{
#if DEBUG_PRINT
  int32_t i;

  printf("%d han %d fu %d points ", score->han, score->fu, score->point.point);

  /* 成立した役リストを表示 */
  printf("Yaku:");
  for (i = 0; i < (int32_t)(sizeof(yaku_name_table) / sizeof(yaku_name_table[0])); i++) {
    if (MJSCORE_IS_YAKU_ESTABLISHED(score->yaku_flags, i)) {
      printf("%s,", yaku_name_table[i]);
    }
  }

  printf("\n");
#endif
}

/* 捨て牌に対してアクション */
static bool execute_action_for_discard_tile(struct MJPlayerAction *last_action, int32_t *score_diff)
{
  int32_t i;
  struct MJPlayerAction actions[4];

  assert(score_diff != NULL);

  /* 捨て牌アクションを全員に通知 */
  assert((last_action->type == MJPLAYER_ACTIONTYPE_DISCARD)
      || (last_action->type == MJPLAYER_ACTIONTYPE_RIICHI));
  for (i = 0; i < 4; i++) {
    actions[i].player = players[i]->wind;
    MJPlayerWrapper_OnAction(players[i], last_action, &actions[i]);
  }

  /* アクションの優先度順に処理 */

  /* ロン */
  for (i = 0; i < 4; i++) {
    if (actions[i].type == MJPLAYER_ACTIONTYPE_RON) {
      /* 得点計算 */
      const MJWind action_player = players[i]->wind;
      struct MJAgariInformation agari_info;
      struct MJScore score;
      MJScoreCalculationResult ret;
      agari_info.winning_tile = last_action->tile;
      agari_info.hand = players[i]->hand;
      agari_info.num_dora = 0;  /* TODO: 仮 */
      agari_info.num_honba = 0;
      agari_info.num_riichibo = 0;
      agari_info.round_wind = MJWIND_TON;
      agari_info.player_wind = action_player;
      agari_info.tsumo = false;
      agari_info.riichi = players[i]->riichi;
      agari_info.ippatsu = false;
      agari_info.double_riichi = false;
      agari_info.haitei = false;  /* TODO: 仮 */
      agari_info.rinshan = false;
      agari_info.chankan = false;
      agari_info.nagashimangan = false;
      agari_info.tenho = false;
      agari_info.chiho = false;
      ret = MJScore_CalculateScore(&agari_info, &score);
      /* チョンボ */
      if (ret == MJSCORE_CALCRESULT_NOT_YAKU) {
#if DEBUG_PRINT
        printf("[%15s (%s)] mistake \n", MJPlayerWrapper_GetName(players[i]), wind_sting_table[i]);
#endif
        if (action_player == MJWIND_TON) {
          score_diff[MJWIND_TON] = -12000;
          score_diff[MJWIND_NAN] = 4000;
          score_diff[MJWIND_SHA] = 4000;
          score_diff[MJWIND_PEE] = 4000;
        } else {
          int32_t k;
          score_diff[action_player] = -8000;
          score_diff[MJWIND_TON] = 4000;
          for (k = 1; k < 4; k++) {
            if (index_mjwind_table[k] != action_player) {
              score_diff[k] = 2000;
            }
          }
        } 
        return true;
      }
      assert(ret == MJSCORE_CALCRESULT_OK);
      /* スコア差分を計算 */
      score_diff[0] = score_diff[1] = score_diff[2] = score_diff[3] = 0;
      score_diff[action_player] = score.point.point;
      score_diff[last_action->player] = -score.point.feed.point;
      print_score(&score);
      return true;
    }
  }

  /* カン/ポン */
  for (i = 0; i < 4; i++) {
    const MJWind action_player = players[i]->wind;
    switch (actions[i].type) {
      case MJPLAYER_ACTIONTYPE_MINKAN:
        /* 大明槓処理 -> 嶺上自摸 */
        if (MJRiver_Kan(&river, action_player, &players[i]->hand) == MJRIVER_APIRESULT_OK) {
          int32_t t;
          MJTile rinshan;
          struct MJHand *phand = &(players[i]->hand);
          const MJTile discard_tile = actions[i].tile;
          /* 純手牌を更新 */
          assert(phand->num_hand_tiles >= 4);
          for (t = 0; t < phand->num_hand_tiles; t++) {
            if (phand->hand[t] == discard_tile) {
              assert(phand->hand[t] == phand->hand[t + 1]);
              assert(phand->hand[t + 1] == phand->hand[t + 2]);
              phand->hand[t] = MJTILE_INVALID;
              phand->hand[t + 1] = MJTILE_INVALID;
              phand->hand[t + 2] = MJTILE_INVALID;
              sort_hand(phand);
              phand->num_hand_tiles -= 3;
              break;
            }
          }
          /* 副露を追加 */
          for (t = 0; t < 4; t++) {
            phand->meld[phand->num_meld].tiles[t] = discard_tile;
          }
          phand->meld[phand->num_meld].type = MJMELD_TYPE_MINKAN;
          phand->num_meld++;
          assert(phand->num_meld <= 4);
          /* 嶺上自摸 */
          MJDeck_RinshanDraw(deck, &rinshan);
          last_action->player = action_player;
          MJPlayerWrapper_OnDiscard(players[i], rinshan, last_action);
          /* TODO: ここでロンだったら嶺上開花 */
          if (last_action->type == MJPLAYER_ACTIONTYPE_TSUMO) {
            struct MJAgariInformation agari_info;
            struct MJScore score;
            MJScoreCalculationResult ret;
            agari_info.winning_tile = rinshan;
            agari_info.hand = players[i]->hand;
            agari_info.num_dora = 0;  /* TODO: 仮 */
            agari_info.num_honba = 0;
            agari_info.num_riichibo = 0;
            agari_info.round_wind = MJWIND_TON;
            agari_info.player_wind = players[i]->wind;
            agari_info.tsumo = true;
            agari_info.riichi = false;
            agari_info.ippatsu = false;
            agari_info.double_riichi = false;
            agari_info.haitei = false;
            agari_info.rinshan = true;
            agari_info.chankan = false;
            agari_info.nagashimangan = false;
            agari_info.tenho = false;
            agari_info.chiho = false;
            ret = MJScore_CalculateScore(&agari_info, &score);
            assert(ret == MJSCORE_CALCRESULT_OK);
            /* スコア差分を計算 */
            {
              int32_t k;
              score_diff[action_player] = score.point.point;
              if (action_player == MJWIND_TON) {
                for (k = 1; k < 4; k++) {
                  score_diff[k] = -score.point.feed.tsumo.ko;
                }
              } else {
                score_diff[MJWIND_TON] = -score.point.feed.tsumo.oya;
                for (k = 1; k < 4; k++) {
                  if (k != (int32_t)action_player) {
                    score_diff[k] = -score.point.feed.tsumo.ko;
                  }
                }
              }
            }
            return true;
          }
          assert(last_action->type == MJPLAYER_ACTIONTYPE_DISCARD);
          /* 手牌更新(ツモ切りでなければ) */
          if (last_action->tile != rinshan) {
            for (t = 0; t < players[i]->hand.num_hand_tiles; t++) {
              if (phand->hand[t] == last_action->tile) {
                players[i]->hand.hand[t] = rinshan;
                sort_hand(phand);
                break;
              }
            }
          }
          /* 河に捨てる */
          MJRiver_DiscardTile(&river, action_player, last_action->tile, false);
          /* 再度捨て牌処理へ */
#if DEBUG_PRINT
          printf("[%15s (%s)] mkan %s -> discard %s \n",
              MJPlayerWrapper_GetName(players[i]), wind_sting_table[i],
              tile_string_table[discard_tile], tile_string_table[last_action->tile]);
#endif
          /* 全プレーヤーに通知 */
          {
            int32_t j;
            struct MJPlayerAction tmp;
            for (j = 0; j < 4; j++) {
              tmp.player = (MJWind)j;
              MJPlayerWrapper_OnAction(players[j], &actions[i], &tmp);
            }
          }
          return execute_action_for_discard_tile(last_action, score_diff);
        } 
        /* カンできないのにカンしたetc TODO: 無視するのもあり */
        assert(0);
        break;
      case MJPLAYER_ACTIONTYPE_PUNG:
        /* ポン */
        if (MJRiver_Pung(&river, action_player, &(players[i]->hand)) == MJRIVER_APIRESULT_OK) {
          int32_t t;
          const MJTile discard_tile = actions[i].tile;
          struct MJHand *phand = &(players[i]->hand);
          /* 純手牌を更新 */
          assert(phand->num_hand_tiles >= 4);
          for (t = 0; t < phand->num_hand_tiles; t++) {
            if (phand->hand[t] == discard_tile) {
              assert(phand->hand[t] == phand->hand[t + 1]);
              phand->hand[t] = MJTILE_INVALID;
              phand->hand[t + 1] = MJTILE_INVALID;
              sort_hand(phand);
              phand->num_hand_tiles -= 2;
              break;
            }
          }
          /* 副露を追加 */
          for (t = 0; t < 3; t++) {
            phand->meld[phand->num_meld].tiles[t] = discard_tile;
          }
          phand->meld[phand->num_meld].type = MJMELD_TYPE_PUNG;
          phand->num_meld++;
          assert(phand->num_meld <= 4);
          /* 捨て牌を要求 */
          last_action->player = action_player;
          MJPlayerWrapper_OnDiscard(players[i], MJTILE_INVALID, last_action);
          assert(last_action->type == MJPLAYER_ACTIONTYPE_DISCARD);
          /* 手牌更新 */
          {
            int32_t t;
            assert(phand->num_hand_tiles > 1);
            for (t = 0; t < players[i]->hand.num_hand_tiles; t++) {
              if (phand->hand[t] == last_action->tile) {
                phand->hand[t] = MJTILE_INVALID;
                sort_hand(phand);
                phand->num_hand_tiles--;
                break;
              }
            }
          }
          /* 河に捨てる */
          MJRiver_DiscardTile(&river, action_player, last_action->tile, false);
          /* 再度捨て牌処理へ */
#if DEBUG_PRINT
          printf("[%15s (%s)] pung %s -> discard %s \n",
              MJPlayerWrapper_GetName(players[i]), wind_sting_table[i],
              tile_string_table[discard_tile], tile_string_table[last_action->tile]);
#endif
          /* 全プレーヤーに通知 */
          {
            int32_t j;
            struct MJPlayerAction tmp;
            for (j = 0; j < 4; j++) {
              tmp.player = (MJWind)j;
              MJPlayerWrapper_OnAction(players[j], &actions[i], &tmp);
            }
          }
          return execute_action_for_discard_tile(last_action, score_diff);
        }
        /* ポンできないのにポンしたetc TODO: 無視するのもあり */
        assert(0);
        break;
      default:
        break;
    }
  }

  /* チー */
  for (i = 0; i < 4; i++) {
    const MJWind action_player = players[i]->wind;
    if ((actions[i].type == MJPLAYER_ACTIONTYPE_CHOW1)
        || (actions[i].type == MJPLAYER_ACTIONTYPE_CHOW2)
        || (actions[i].type == MJPLAYER_ACTIONTYPE_CHOW3)) {
      if (MJRiver_Chow(&river, action_player, &(players[i]->hand)) == MJRIVER_APIRESULT_OK) {
        int32_t t;
        bool tile_mark[3] = { false, false, false };
        struct MJHand *phand = &(players[i]->hand);
        MJTile min_tile = MJTILE_INVALID;
        /* 純手牌を更新 */
        assert(phand->num_hand_tiles >= 4);
        assert(MJTILE_IS_SUHAI(actions[i].tile));
        switch (actions[i].type) {
          case MJPLAYER_ACTIONTYPE_CHOW1: 
            min_tile = actions[i].tile; 
            tile_mark[0] = true;
            break;
          case MJPLAYER_ACTIONTYPE_CHOW2:
            min_tile = actions[i].tile - 1;
            tile_mark[1] = true;
            break;
          case MJPLAYER_ACTIONTYPE_CHOW3: 
            tile_mark[2] = true;
            min_tile = actions[i].tile - 2; 
            break;
          default: assert(0);
        }
        assert(min_tile != MJTILE_INVALID);
        for (t = 0; t < phand->num_hand_tiles; t++) {
          if (phand->hand[t] == actions[i].tile) {
            continue;
          }
          if (!tile_mark[0] && (phand->hand[t] == min_tile)) {
            phand->hand[t] = MJTILE_INVALID;
            tile_mark[0] = true;
          } else if (!tile_mark[1] && (phand->hand[t] == (min_tile + 1))) {
            phand->hand[t] = MJTILE_INVALID;
            tile_mark[1] = true;
          } else if (!tile_mark[2] && (phand->hand[t] == (min_tile + 2))) {
            phand->hand[t] = MJTILE_INVALID;
            tile_mark[2] = true;
          }
        }
        assert(tile_mark[0] && tile_mark[1] && tile_mark[2]);
        sort_hand(phand);
        phand->num_hand_tiles -= 2;
        /* 副露を追加 */
        for (t = 0; t < 3; t++) {
          phand->meld[phand->num_meld].tiles[t] = (MJTile)(min_tile + t);
        }
        phand->meld[phand->num_meld].type = MJMELD_TYPE_CHOW;
        phand->num_meld++;
        assert(phand->num_meld <= 4);
        /* 捨て牌を要求 */
        {
          int32_t t;
          last_action->player = action_player;
          MJPlayerWrapper_OnDiscard(players[i], MJTILE_INVALID, last_action);
          assert(last_action->type == MJPLAYER_ACTIONTYPE_DISCARD);
          assert(players[i]->hand.num_hand_tiles > 1);
          for (t = 0; t < players[i]->hand.num_hand_tiles; t++) {
            if (phand->hand[t] == last_action->tile) {
              phand->hand[t] = MJTILE_INVALID;
              sort_hand(phand);
              phand->num_hand_tiles--;
              break;
            }
          }
        }
        /* 河に捨てる */
        MJRiver_DiscardTile(&river, action_player, last_action->tile, false);
        /* 再度捨て牌処理へ */
#if DEBUG_PRINT
        printf("[%15s (%s)] chow %s -> discard %s \n",
            MJPlayerWrapper_GetName(players[i]), wind_sting_table[i],
            tile_string_table[actions[i].tile], tile_string_table[last_action->tile]);
#endif
        /* 全プレーヤーに通知 */
        {
          int32_t j;
          struct MJPlayerAction tmp;
          for (j = 0; j < 4; j++) {
            tmp.player = (MJWind)j;
            MJPlayerWrapper_OnAction(players[j], &actions[i], &tmp);
          }
        }
        return execute_action_for_discard_tile(last_action, score_diff);
      } else {
        /* チーできないのにチーしたetc TODO: 無視するのもあり */
        assert(0);
      }
    }
  }

  /* 誰も何もしてなかった */
  return false;
}

int main(void)
{
  int32_t i, j;
  struct MJPlayerWrapperConfig config[4];

  /* 山、乱数ハンドル生成 */
  deck = MJDeck_Create(NULL, NULL, 0);

  /* プレーヤーコンフィグ設定 */
  config[0].player_interface = MJPlayerTsumogiri_GetInterface();
  config[1].player_interface = MJPlayerShantenman_GetInterface();
  // config[1].player_interface = MJPlayerCUI_GetInterface();
  config[2].player_interface = MJPlayerShantenman_GetInterface();
  config[3].player_interface = MJPlayerFuroman_GetInterface();

  /* プレーヤーハンドル作成 */
  for (i = 0; i < 4; i++) {
    config[i].player_config.game_state_getter_if = &st_stub_game_state_if;
    players[i] = MJPlayerWrapper_Create(&config[i], NULL, 0);
  }

  /* ゲーム開始を通知 */
  for (i = 0; i < 4; i++) {
    MJPlayerWrapper_OnStartGame(players[i]);
  }

  /* 初期得点 */
  players[0]->score = players[1]->score = players[2]->score = players[3]->score = 25000;
  for (hand_no = 0; hand_no < MAX_NUM_HANDS; hand_no++) {
    int32_t turn, num_remain_tiles;
    int32_t score_diff[4];
    bool someone_ron;

    /* 洗牌 */
    MJDeck_Shuffle(deck);
    /* 河の初期化 */
    MJRiver_Initialize(&river);

    /* 配牌 */
    for (i = 0; i < 4; i++) {
      struct MJHand *phand = &(players[i]->hand);
      for (j = 0; j < 13; j++) {
        MJDeck_Draw(deck, &(phand->hand[j]));
      }
      phand->num_hand_tiles = 13;
      /* 副露はない状態に */
      for (j = 0; j < 4; j++) {
        int32_t k;
        phand->meld[j].type = MJMELD_TYPE_INVALID;
        for (k = 0; k < 4; k++) {
          phand->meld[j].tiles[k] = MJTILE_INVALID;
        }
      }
      phand->num_meld = 0;
      /* 手牌をソート */
      sort_hand(phand);
    }

    /* 局開始を通知 */
#if DEBUG_PRINT
    printf("hand: %d\n", hand_no);
#endif 
    /* 家と局番号を通知 */
    for (i = 0; i < 4; i++) {
      players[i]->wind = index_mjwind_table[i];
      MJPlayerWrapper_OnStartHand(players[i], hand_no, players[i]->wind);
    }

    turn = 1;
    someone_ron = false;
    for (i = 0; i < 4; i++) {
      score_diff[i] = 0;
      players[i]->riichi = false;
    }
    do {
#if DEBUG_PRINT
      printf("turn %d -------------------------------- \n", turn);
#endif

      /* 順繰りに自摸 -> 捨て牌処理 */
      for (i = 0; i < 4; i++) {
        MJTile draw;
        struct MJPlayerAction action;
        const MJWind action_player = players[i]->wind;

        /* 自摸 */
        if (MJDeck_Draw(deck, &draw) == MJDECK_APIRESULT_EMPTY_DECK) {
          break;
        }
        action.player = action_player;
        MJPlayerWrapper_OnDiscard(players[i], draw, &action);

        /* 自摸に対するアクション処理 */
        switch (action.type) {
          case MJPLAYER_ACTIONTYPE_RIICHI:
            players[i]->riichi = true;
          case MJPLAYER_ACTIONTYPE_DISCARD:
            /* ダブリー判定とかできるとよい */
            MJRiver_DiscardTile(&river, action_player, action.tile, action.type == MJPLAYER_ACTIONTYPE_RIICHI);
            /* ツモ切りでなければ手牌の更新 */
            if (action.tile != draw) {
              int32_t t;
              /* 純手牌の更新 */
              for (t = 0; t < players[i]->hand.num_hand_tiles; t++) {
                if (players[i]->hand.hand[t] == action.tile) {
                  players[i]->hand.hand[t] = draw;
                  sort_hand(&players[i]->hand);
                  break;
                }
              }
              /* 手牌に存在しない牌を捨ててしまった */
              assert((t >= 0) && (t < players[i]->hand.num_hand_tiles));
            }
            break;
          case MJPLAYER_ACTIONTYPE_ANKAN:
          case MJPLAYER_ACTIONTYPE_KAKAN:
            /* 暗槓/加槓 */
            assert(0);
            break;
          case MJPLAYER_ACTIONTYPE_TSUMO:
            /* 自摸和了 */
            {
              struct MJAgariInformation agari_info;
              struct MJScore score;
              MJScoreCalculationResult ret;
              agari_info.winning_tile = draw;
              agari_info.hand = players[i]->hand;
              agari_info.num_dora = 0;  /* TODO: 仮 */
              agari_info.num_honba = 0;
              agari_info.num_riichibo = 0;
              agari_info.round_wind = MJWIND_TON;
              agari_info.player_wind = players[i]->wind;
              agari_info.tsumo = true;
              agari_info.riichi = players[i]->riichi;
              agari_info.ippatsu = false;
              agari_info.double_riichi = false;
              agari_info.haitei = (num_remain_tiles == 0);
              agari_info.rinshan = false;
              agari_info.chankan = false;
              agari_info.nagashimangan = false;
              agari_info.tenho = false;
              agari_info.chiho = false;
              ret = MJScore_CalculateScore(&agari_info, &score);
              assert(ret == MJSCORE_CALCRESULT_OK);
              /* スコア差分を計算 */
              {
                int32_t k;
                score_diff[action_player] = score.point.point;
                if (action_player == MJWIND_TON) {
                  for (k = 1; k < 4; k++) {
                    score_diff[k] = -score.point.feed.tsumo.ko;
                  }
                } else {
                  score_diff[MJWIND_TON] = -score.point.feed.tsumo.oya;
                  for (k = 1; k < 4; k++) {
                    if (k != (int32_t)action_player) {
                      score_diff[k] = -score.point.feed.tsumo.ko;
                    }
                  }
                }
              }
              someone_ron = true;
              print_score(&score);
              goto END_HAND;
            }
            break;
          case MJPLAYER_ACTIONTYPE_DRAWN:
            /* 流局 */
            assert(0);
            break;
          default: assert(0);
        }

#if DEBUG_PRINT
        printf("[%15s (%s)] draw %s -> discard %s hand: ",
            MJPlayerWrapper_GetName(players[i]), wind_sting_table[action_player],
            tile_string_table[draw], tile_string_table[action.tile]);
        for (j = 0; j < players[i]->hand.num_hand_tiles; j++) {
          printf("%s,", tile_string_table[players[i]->hand.hand[j]]);
        }
        printf(" meld: ");
        for (j = 0; j < players[i]->hand.num_meld; j++) {
          static const char *furo_strings[] = { "P", "C", "A", "M", "K" };
          printf("%s(%s),", 
              tile_string_table[players[i]->hand.meld[j].tiles[0]], 
              furo_strings[players[i]->hand.meld[j].type]);
        }
        printf("\n");
#endif

        /* 捨てた牌に対してアクション */
        /* TODO: チョンボと区別がつかん */
        someone_ron = execute_action_for_discard_tile(&action, score_diff);
        /* 最後に捨てたプレーヤーの次にスキップ */
        i = (int32_t)action.player;
        /* 誰かが和了った */
        if (someone_ron) {
          goto END_HAND;
        }
      }

      print_river(&river);
      turn++;
      MJDeck_GetNumRemainTiles(deck, &num_remain_tiles);
    } while (num_remain_tiles > 0);

END_HAND:
    /* 局終了 */
    {
      MJHandEndReason reason;
      if (someone_ron) {
        reason = MJHAND_ENDREASON_RON;
      } else if (num_remain_tiles == 0) {
        /* 荒牌流局 */
        reason = MJHAND_ENDREASON_DRAWN;
        /* TODO: 得点割り振り */
      } else {
        assert(0);
      }
      /* 局終了通知/得点更新 */
      for (i = 0; i < 4; i++) {
        MJPlayerWrapper_OnEndHand(players[i], reason, score_diff);
        players[i]->score += score_diff[i];
      }
#if DEBUG_PRINT
      /* 得点表示 */
      for (i = 0; i < 4; i++) {
        printf("[%15s (%s)] %d \n", 
            MJPlayerWrapper_GetName(players[i]), wind_sting_table[players[i]->wind], players[i]->score);
      }
      int32_t sum = 0;
      for (i = 0; i < 4; i++) {
        sum += players[i]->score;
      }
      printf("%d %d \n", hand_no, sum);
      assert(sum == 100000);
#endif
    }

    /* 次の局へ */
    {
      /* 家の入れ替え */
      struct MJPlayerWrapper *tmp = players[MJWIND_TON];
      players[MJWIND_TON] = players[MJWIND_NAN];
      players[MJWIND_NAN] = players[MJWIND_SHA];
      players[MJWIND_SHA] = players[MJWIND_PEE];
      players[MJWIND_PEE] = tmp;
    }
  }

  /* 対戦終了を通知 */
  { 
    /* スコアの並び替え */
    struct MJPlayerWrapper *sorted_players[4];
    for (i = 0; i < 4; i++) {
      sorted_players[i] = players[i];
    }
    qsort(sorted_players, 4, sizeof(struct MJPlayerWrapper *), compare_score_dec);
    for (i = 0; i < 4; i++) {
      MJPlayerWrapper_OnEndGame(sorted_players[i], i + 1, sorted_players[i]->score);
    }
    /* 得点表示 */
    for (i = 0; i < 4; i++) {
      printf("rank:%d [%15s] %8d \n",
          i + 1, MJPlayerWrapper_GetName(sorted_players[i]), sorted_players[i]->score);
    }
  }

  /* プレイヤーを破棄 */
  for (i = 0; i < 4; i++) {
    MJPlayerWrapper_Destroy(players[i]);
  }

  /* 牌山、乱数ハンドルを破棄 */
  MJDeck_Destroy(deck);

  return 0;
}
