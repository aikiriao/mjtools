#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

/* テスト対象のモジュール */
#include "../mj_score.c"

/* 役をフラグ化したもの */
#define MJSCORE_FLAG_RIICHI           MJSCORE_YAKU_FLAG(MJSCORE_YAKU_RIICHI)
#define MJSCORE_FLAG_DOUBLERIICHI     MJSCORE_YAKU_FLAG(MJSCORE_YAKU_DOUBLERIICHI)
#define MJSCORE_FLAG_IPPATSU          MJSCORE_YAKU_FLAG(MJSCORE_YAKU_IPPATSU)
#define MJSCORE_FLAG_TSUMO            MJSCORE_YAKU_FLAG(MJSCORE_YAKU_TSUMO)
#define MJSCORE_FLAG_TANYAO           MJSCORE_YAKU_FLAG(MJSCORE_YAKU_TANYAO)
#define MJSCORE_FLAG_PINFU            MJSCORE_YAKU_FLAG(MJSCORE_YAKU_PINFU)
#define MJSCORE_FLAG_IPEKO            MJSCORE_YAKU_FLAG(MJSCORE_YAKU_IPEKO)
#define MJSCORE_FLAG_BAKAZE           MJSCORE_YAKU_FLAG(MJSCORE_YAKU_BAKAZE)
#define MJSCORE_FLAG_JIKAZE           MJSCORE_YAKU_FLAG(MJSCORE_YAKU_JIKAZE)
#define MJSCORE_FLAG_HAKU             MJSCORE_YAKU_FLAG(MJSCORE_YAKU_HAKU)
#define MJSCORE_FLAG_HATU             MJSCORE_YAKU_FLAG(MJSCORE_YAKU_HATU)
#define MJSCORE_FLAG_CHUN             MJSCORE_YAKU_FLAG(MJSCORE_YAKU_CHUN)
#define MJSCORE_FLAG_RINSHAN          MJSCORE_YAKU_FLAG(MJSCORE_YAKU_RINSHAN)
#define MJSCORE_FLAG_CHANKAN          MJSCORE_YAKU_FLAG(MJSCORE_YAKU_CHANKAN)
#define MJSCORE_FLAG_HAITEITSUMO      MJSCORE_YAKU_FLAG(MJSCORE_YAKU_HAITEITSUMO)
#define MJSCORE_FLAG_HOUTEIRON        MJSCORE_YAKU_FLAG(MJSCORE_YAKU_HOUTEIRON)
#define MJSCORE_FLAG_SANSYOKUDOUJYUN  MJSCORE_YAKU_FLAG(MJSCORE_YAKU_SANSYOKUDOUJYUN)
#define MJSCORE_FLAG_IKKITSUKAN       MJSCORE_YAKU_FLAG(MJSCORE_YAKU_IKKITSUKAN)
#define MJSCORE_FLAG_CHANTA           MJSCORE_YAKU_FLAG(MJSCORE_YAKU_CHANTA)
#define MJSCORE_FLAG_CHITOITSU        MJSCORE_YAKU_FLAG(MJSCORE_YAKU_CHITOITSU)
#define MJSCORE_FLAG_TOITOIHO         MJSCORE_YAKU_FLAG(MJSCORE_YAKU_TOITOIHO)
#define MJSCORE_FLAG_SANANKO          MJSCORE_YAKU_FLAG(MJSCORE_YAKU_SANANKO)
#define MJSCORE_FLAG_HONROUTO         MJSCORE_YAKU_FLAG(MJSCORE_YAKU_HONROUTO)
#define MJSCORE_FLAG_SANSYOKUDOUKOKU  MJSCORE_YAKU_FLAG(MJSCORE_YAKU_SANSYOKUDOUKOKU)
#define MJSCORE_FLAG_SANKANTSU        MJSCORE_YAKU_FLAG(MJSCORE_YAKU_SANKANTSU)
#define MJSCORE_FLAG_SYOSANGEN        MJSCORE_YAKU_FLAG(MJSCORE_YAKU_SYOSANGEN)
#define MJSCORE_FLAG_HONITSU          MJSCORE_YAKU_FLAG(MJSCORE_YAKU_HONITSU)
#define MJSCORE_FLAG_JYUNCHANTA       MJSCORE_YAKU_FLAG(MJSCORE_YAKU_JYUNCHANTA)
#define MJSCORE_FLAG_RYANPEKO         MJSCORE_YAKU_FLAG(MJSCORE_YAKU_RYANPEKO)
#define MJSCORE_FLAG_CHINITSU         MJSCORE_YAKU_FLAG(MJSCORE_YAKU_CHINITSU)
#define MJSCORE_FLAG_TENHO            MJSCORE_YAKU_FLAG(MJSCORE_YAKU_TENHO)
#define MJSCORE_FLAG_CHIHO            MJSCORE_YAKU_FLAG(MJSCORE_YAKU_CHIHO)
#define MJSCORE_FLAG_KOKUSHIMUSOU     MJSCORE_YAKU_FLAG(MJSCORE_YAKU_KOKUSHIMUSOU)
#define MJSCORE_FLAG_KOKUSHIMUSOU13   MJSCORE_YAKU_FLAG(MJSCORE_YAKU_KOKUSHIMUSOU13)
#define MJSCORE_FLAG_CHURENPOUTON     MJSCORE_YAKU_FLAG(MJSCORE_YAKU_CHURENPOUTON)
#define MJSCORE_FLAG_CHURENPOUTON9    MJSCORE_YAKU_FLAG(MJSCORE_YAKU_CHURENPOUTON9)
#define MJSCORE_FLAG_SUANKO           MJSCORE_YAKU_FLAG(MJSCORE_YAKU_SUANKO)
#define MJSCORE_FLAG_SUANKOTANKI      MJSCORE_YAKU_FLAG(MJSCORE_YAKU_SUANKOTANKI)
#define MJSCORE_FLAG_DAISUSHI         MJSCORE_YAKU_FLAG(MJSCORE_YAKU_DAISUSHI)
#define MJSCORE_FLAG_SYOSUSHI         MJSCORE_YAKU_FLAG(MJSCORE_YAKU_SYOSUSHI)
#define MJSCORE_FLAG_DAISANGEN        MJSCORE_YAKU_FLAG(MJSCORE_YAKU_DAISANGEN)
#define MJSCORE_FLAG_TSUISO           MJSCORE_YAKU_FLAG(MJSCORE_YAKU_TSUISO)
#define MJSCORE_FLAG_CHINROTO         MJSCORE_YAKU_FLAG(MJSCORE_YAKU_CHINROTO)
#define MJSCORE_FLAG_RYUISO           MJSCORE_YAKU_FLAG(MJSCORE_YAKU_RYUISO)
#define MJSCORE_FLAG_SUKANTSU         MJSCORE_YAKU_FLAG(MJSCORE_YAKU_SUKANTSU)
#define MJSCORE_FLAG_DORA             MJSCORE_YAKU_FLAG(MJSCORE_YAKU_DORA)
#define MJSCORE_FLAG_NAGASHIMANGAN    MJSCORE_YAKU_FLAG(MJSCORE_YAKU_NAGASHIMANGAN)

/* 1枚の牌情報 */
struct TileInfo {
  int8_t  type;   /* 牌の種類(-1で終わり) */
  uint8_t maisu;  /* 所持数               */
};

/* テストケース */
struct MJScoreTestCase {
  uint8_t agarihai;             /* 和了牌 */
  struct TileInfo tiles[14];    /* 手牌情報（最大14エントリ） */
  struct MJMeld   meld[4];      /* 副露情報（最大4エントリ, typeをINVALIDにして終わり） */
  uint8_t         num_dora;     /* ドラ牌数 */
  uint8_t         num_honba;    /* 本場数 */
  uint8_t         num_riichibo; /* 供託立直棒 */
  MJKaze          bakaze;       /* 場風 */
  MJKaze          jikaze;       /* 自風 */
  bool            tsumo;        /* 自摸 */
  bool            riichi;       /* 立直 */
  bool            ippatsu;      /* 一発 */
  struct MJScore  answer;       /* 正解 */
};

/* テストのセットアップ関数 */
void MJScoreTest_Setup(void);

/* 牌の文字列変換テーブル */
static const char *tile_string_table[MJTILE_MAX] = {
  "invalid",
  "1M", "2M", "3M", "4M", "5M", "6M", "7M", "8M", "9M",
  "invalid",
  "1P", "2P", "3P", "4P", "5P", "6P", "7P", "8P", "9P",
  "invalid",
  "1S", "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S",
  "invalid",
  "TON", "NAN", "SHA", "PEE", "HAKU", "HATU", "CHUN",
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

/* 通常手のテストケース */
static const struct MJScoreTestCase normal_test_cases[] = {
  {
    .agarihai = MJTILE_4SOU,
    .tiles = {
      { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 },
      { MJTILE_9PIN, 2 }, { MJTILE_3SOU, 3 }, { MJTILE_4SOU, 3 },
      { MJTILE_8SOU, 3 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_NAN,
    .tsumo = false, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 1, .fu = 40,
      .point = {
        .point = 1300,
        .feed.point = 1300,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI,
    }
  },
  {
    .agarihai = MJTILE_4SOU,
    .tiles = {
      { MJTILE_2PIN, 1 }, { MJTILE_3PIN, 1 }, { MJTILE_4PIN, 1 },
      { MJTILE_3SOU, 1 }, { MJTILE_4SOU, 1 }, { MJTILE_5SOU, 1 },
      { MJTILE_8PIN, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_CHOW, MJTILE_7MAN, MJTILE_9MAN, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_ANKAN, MJTILE_CHUN, 0, 0 },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 2,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_TON,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 60,
      .point = {
        .point = 11600,
        .feed.point = 11600,
      },
      .yaku_flags = MJSCORE_FLAG_DORA | MJSCORE_FLAG_CHUN,
    }
  },
  {
    .agarihai = MJTILE_4SOU,
    .tiles = {
      { MJTILE_6MAN, 1 }, { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 },
      { MJTILE_4SOU, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_ANKAN, MJTILE_9PIN, 0, 0 },
      { MJFURO_TYPE_PUNG, MJTILE_CHUN, MJTILE_CHUN, MJTACHA_TYPE_KAMICHA },
      { MJFURO_TYPE_PUNG, MJTILE_HATU, MJTILE_HATU, MJTACHA_TYPE_KAMICHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 1, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_NAN,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 70,
      .point = {
        .point = 8300,
        .feed.point = 8300,
      },
      .yaku_flags = MJSCORE_FLAG_HATU | MJSCORE_FLAG_CHUN | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_1SOU,
    .tiles = {
      { MJTILE_3MAN, 3 }, { MJTILE_1SOU, 2 }, { MJTILE_3SOU, 3 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_ANKAN, MJTILE_3PIN, 0, 0 },
      { MJFURO_TYPE_CHOW, MJTILE_5PIN, MJTILE_5PIN, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 3,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_NAN,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 7, .fu = 50,
      .point = {
        .point = 12000,
        .feed.point = 12000,
      },
      .yaku_flags = MJSCORE_FLAG_DORA | MJSCORE_FLAG_SANANKO | MJSCORE_FLAG_SANSYOKUDOUKOKU,
    }
  },
  {
    .agarihai = MJTILE_HATU,
    .tiles = {
      { MJTILE_9MAN, 2 }, { MJTILE_2PIN, 1 }, { MJTILE_3PIN, 1 },
      { MJTILE_4PIN, 1 }, { MJTILE_HATU, 3 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_CHOW, MJTILE_5SOU, MJTILE_5SOU, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_CHOW, MJTILE_1SOU, MJTILE_1SOU, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 3,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 4, .fu = 30,
      .point = {
        .point = 7700,
        .feed.point = 7700,
      },
      .yaku_flags = MJSCORE_FLAG_HATU | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_4MAN,
    .tiles = {
      { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 },
      { MJTILE_8PIN, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_NAN,  MJTILE_NAN,  MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_PUNG, MJTILE_4SOU, MJTILE_4SOU, MJTACHA_TYPE_KAMICHA  },
      { MJFURO_TYPE_CHOW, MJTILE_1MAN, MJTILE_1MAN, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_SHA,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 1, .fu = 30,
      .point = {
        .point = 1100,
        .feed.tsumo.oya = 500,
        .feed.tsumo.ko  = 300,
      },
      .yaku_flags = MJSCORE_FLAG_BAKAZE,
    }
  },
  {
    .agarihai = MJTILE_3SOU,
    .tiles = {
      { MJTILE_4MAN, 2 }, { MJTILE_6MAN, 1 }, { MJTILE_7MAN, 1 },
      { MJTILE_8MAN, 1 }, { MJTILE_7PIN, 1 }, { MJTILE_8PIN, 1 },
      { MJTILE_9PIN, 1 }, { MJTILE_2SOU, 1 }, { MJTILE_3SOU, 2 },
      { MJTILE_4SOU, 2 }, { MJTILE_5SOU, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_TON,
    .tsumo = true, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 20,
      .point = {
        .point = 3900,
        .feed.tsumo.oya = 0,
        .feed.tsumo.ko  = 1300,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_TSUMO | MJSCORE_FLAG_PINFU,
    }
  },
  {
    .agarihai = MJTILE_1SOU,
    .tiles = {
      { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 }, { MJTILE_7MAN, 1 },
      { MJTILE_1PIN, 1 }, { MJTILE_2PIN, 1 }, { MJTILE_3PIN, 1 },
      { MJTILE_6PIN, 1 }, { MJTILE_7PIN, 1 }, { MJTILE_8PIN, 1 },
      { MJTILE_1SOU, 1 }, { MJTILE_2SOU, 1 }, { MJTILE_3SOU, 1 },
      { MJTILE_5SOU, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 3,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_TON,
    .tsumo = false, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 5, .fu = 30,
      .point = {
        .point = 12000,
        .feed.point = 12000,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_PINFU | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_2MAN,
    .tiles = {
      { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 }, { MJTILE_4MAN, 1 },
      { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 }, { MJTILE_7MAN, 1 },
      { MJTILE_6PIN, 1 }, { MJTILE_7PIN, 1 }, { MJTILE_8PIN, 1 },
      { MJTILE_6SOU, 1 }, { MJTILE_7SOU, 1 }, { MJTILE_8SOU, 1 },
      { MJTILE_TON,  2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_NAN,
    .tsumo = false, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 30,
      .point = {
        .point = 3900,
        .feed.point = 3900,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_PINFU | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_8SOU,
    .tiles = {
      { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 }, { MJTILE_7MAN, 1 },
      { MJTILE_3PIN, 2 }, { MJTILE_5PIN, 1 }, { MJTILE_6PIN, 1 },
      { MJTILE_7PIN, 1 }, { MJTILE_2SOU, 1 }, { MJTILE_3SOU, 1 },
      { MJTILE_4SOU, 1 }, { MJTILE_6SOU, 1 }, { MJTILE_7SOU, 1 },
      { MJTILE_8SOU, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 3,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_SHA,
    .tsumo = false, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 6, .fu = 30,
      .point = {
        .point = 12000,
        .feed.point = 12000,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_PINFU | MJSCORE_FLAG_DORA | MJSCORE_FLAG_TANYAO,
    }
  },
  {
    .agarihai = MJTILE_6SOU,
    .tiles = {
      { MJTILE_3MAN, 1 }, { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 },
      { MJTILE_6MAN, 2 }, { MJTILE_5PIN, 1 }, { MJTILE_6PIN, 1 },
      { MJTILE_7PIN, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_CHOW, MJTILE_5SOU, MJTILE_7SOU, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_PUNG, MJTILE_8MAN, MJTILE_8MAN, MJTACHA_TYPE_KAMICHA  },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 3,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_TON,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 4, .fu = 30,
      .point = {
        .point = 11600,
        .feed.point = 11600,
      },
      .yaku_flags = MJSCORE_FLAG_TANYAO | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_7MAN,
    .tiles = {
      { MJTILE_3MAN, 1 }, { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 },
      { MJTILE_7MAN, 2 }, { MJTILE_8PIN, 3 }, { MJTILE_5SOU, 3 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_CHOW, MJTILE_2PIN, MJTILE_2PIN, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 2, .fu = 40,
      .point = {
        .point = 2600,
        .feed.tsumo.oya = 1300,
        .feed.tsumo.ko = 700,
      },
      .yaku_flags = MJSCORE_FLAG_TANYAO | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_9PIN,
    .tiles = {
      { MJTILE_2PIN, 1 }, { MJTILE_3PIN, 1 }, { MJTILE_4PIN, 2 },
      { MJTILE_5PIN, 1 }, { MJTILE_6PIN, 1 }, { MJTILE_7PIN, 1 },
      { MJTILE_8PIN, 1 }, { MJTILE_9PIN, 1 }, { MJTILE_HAKU, 2 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_HATU, MJTILE_HATU, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 30,
      .point = {
        .point = 3900,
        .feed.tsumo.oya = 2000,
        .feed.tsumo.ko = 1000,
      },
      .yaku_flags = MJSCORE_FLAG_HATU | MJSCORE_FLAG_HONITSU,
    }
  },
  {
    .agarihai = MJTILE_2PIN,
    .tiles = {
      { MJTILE_2PIN, 2 }, { MJTILE_3PIN, 1 }, { MJTILE_4PIN, 1 },
      { MJTILE_5PIN, 1 }, { MJTILE_6PIN, 2 }, { MJTILE_7PIN, 2 },
      { MJTILE_8PIN, 2 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_1PIN, MJTILE_1PIN, MJTACHA_TYPE_KAMICHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 5, .fu = 30,
      .point = {
        .point = 8000,
        .feed.tsumo.oya = 4000,
        .feed.tsumo.ko = 2000,
      },
      .yaku_flags = MJSCORE_FLAG_CHINITSU,
    }
  },
  {
    .agarihai = MJTILE_8PIN,
    .tiles = {
      { MJTILE_3MAN, 1 }, { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 },
      { MJTILE_8MAN, 2 }, { MJTILE_6PIN, 1 }, { MJTILE_7PIN, 1 },
      { MJTILE_8PIN, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_CHOW, MJTILE_1PIN, MJTILE_1PIN, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_PUNG, MJTILE_CHUN, MJTILE_CHUN, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_SHA,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 1, .fu = 30,
      .point = {
        .point = 1100,
        .feed.tsumo.oya = 500,
        .feed.tsumo.ko = 300,
      },
      .yaku_flags = MJSCORE_FLAG_CHUN,
    }
  },
  {
    .agarihai = MJTILE_7MAN,
    .tiles = {
      { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { MJTILE_9MAN, 1 },
      { MJTILE_7PIN, 1 }, { MJTILE_8PIN, 1 }, { MJTILE_9PIN, 1 },
      { MJTILE_1SOU, 1 }, { MJTILE_2SOU, 1 }, { MJTILE_3SOU, 1 },
      { MJTILE_7SOU, 1 }, { MJTILE_8SOU, 1 }, { MJTILE_9SOU, 3 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_TON,
    .tsumo = true, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 8, .fu = 30,
      .point = {
        .point = 24000,
        .feed.tsumo.oya = 0,
        .feed.tsumo.ko = 8000,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_TSUMO | MJSCORE_FLAG_SANSYOKUDOUJYUN | MJSCORE_FLAG_JYUNCHANTA | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_6PIN,
    .tiles = {
      { MJTILE_4PIN, 3 }, { MJTILE_5PIN, 1 }, { MJTILE_6PIN, 1 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_HAKU, MJTILE_HAKU, MJTACHA_TYPE_TOIMEN   },
      { MJFURO_TYPE_PUNG, MJTILE_5SOU, MJTILE_5SOU, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_CHOW, MJTILE_7SOU, MJTILE_7SOU, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 2,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_SHA,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 30,
      .point = {
        .point = 3900,
        .feed.tsumo.oya = 2000,
        .feed.tsumo.ko = 1000,
      },
      .yaku_flags = MJSCORE_FLAG_HAKU | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_4MAN,
    .tiles = {
      { MJTILE_3MAN, 1 }, { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 },
      { MJTILE_3PIN, 1 }, { MJTILE_4PIN, 1 }, { MJTILE_5PIN, 1 },
      { MJTILE_4SOU, 2 }, { MJTILE_6SOU, 3 }, { MJTILE_7SOU, 1 },
      { MJTILE_8SOU, 1 }, { MJTILE_9SOU, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_TON,
    .tsumo = false, .riichi = true, .ippatsu = true,
    .answer = {
      .han = 3, .fu = 40,
      .point = {
        .point = 7700,
        .feed.point = 7700,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_IPPATSU | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_8MAN,
    .tiles = {
      { MJTILE_2MAN, 3 }, { MJTILE_8MAN, 3 }, { MJTILE_4SOU, 2 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_CHOW, MJTILE_2SOU, MJTILE_4SOU, MJTACHA_TYPE_SHIMOCHA   },
      { MJFURO_TYPE_CHOW, MJTILE_5PIN, MJTILE_5PIN, MJTACHA_TYPE_SHIMOCHA   },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_NAN,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 2, .fu = 30,
      .point = {
        .point = 2000,
        .feed.point = 2000,
      },
      .yaku_flags = MJSCORE_FLAG_TANYAO | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_4SOU,
    .tiles = {
      { MJTILE_4MAN, 2 }, { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 2 },
      { MJTILE_7MAN, 2 }, { MJTILE_8MAN, 1 }, { MJTILE_4SOU, 1 },
      { MJTILE_5SOU, 1 }, { MJTILE_6SOU, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_CHOW, MJTILE_2PIN, MJTILE_3PIN, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 2,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_NAN,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 30,
      .point = {
        .point = 3900,
        .feed.point = 3900,
      },
      .yaku_flags = MJSCORE_FLAG_TANYAO | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_5SOU,
    .tiles = {
      { MJTILE_4MAN, 3 }, { MJTILE_6PIN, 2 }, { MJTILE_3SOU, 1 },
      { MJTILE_4SOU, 1 }, { MJTILE_5SOU, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_CHOW, MJTILE_5MAN, MJTILE_7MAN, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_CHOW, MJTILE_6PIN, MJTILE_6PIN, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 1, .fu = 30,
      .point = {
        .point = 1000,
        .feed.point = 1000,
      },
      .yaku_flags = MJSCORE_FLAG_TANYAO,
    }
  },
  {
    .agarihai = MJTILE_6MAN,
    .tiles = {
      { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 2 },
      { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { MJTILE_6PIN, 2 },
      { MJTILE_1SOU, 2 }, { MJTILE_2SOU, 2 }, { MJTILE_3SOU, 2 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = true, .ippatsu = true,
    .answer = {
      .han = 5, .fu = 30,
      .point = {
        .point = 8000,
        .feed.point = 8000,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_IPPATSU | MJSCORE_FLAG_PINFU | MJSCORE_FLAG_IPEKO | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_4SOU,
    .tiles = {
      { MJTILE_9MAN, 2 }, { MJTILE_4PIN, 1 }, { MJTILE_5PIN, 1 },
      { MJTILE_6PIN, 1 }, { MJTILE_4SOU, 1 }, { MJTILE_5SOU, 1 },
      { MJTILE_6SOU, 1 }, { MJTILE_SHA,  3 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_CHUN, MJTILE_CHUN, MJTACHA_TYPE_KAMICHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_SHA,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 2, .fu = 40,
      .point = {
        .point = 2600,
        .feed.point = 2600,
      },
      .yaku_flags = MJSCORE_FLAG_JIKAZE | MJSCORE_FLAG_CHUN,
    }
  },
  {
    .agarihai = MJTILE_4PIN,
    .tiles = {
      { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 }, { MJTILE_7MAN, 1 },
      { MJTILE_1PIN, 1 }, { MJTILE_2PIN, 1 }, { MJTILE_3PIN, 1 },
      { MJTILE_4PIN, 1 }, { MJTILE_5PIN, 1 }, { MJTILE_6PIN, 1 },
      { MJTILE_7PIN, 1 }, { MJTILE_8PIN, 1 }, { MJTILE_9PIN, 1 },
      { MJTILE_7SOU, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_TON,
    .tsumo = true, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 5, .fu = 20,
      .point = {
        .point = 12000,
        .feed.tsumo.oya = 0,
        .feed.tsumo.ko = 4000,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_TSUMO | MJSCORE_FLAG_PINFU | MJSCORE_FLAG_IKKITSUKAN,
    }
  },
  {
    .agarihai = MJTILE_HAKU,
    .tiles = {
      { MJTILE_1MAN, 1 }, { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 },
      { MJTILE_NAN,  3 }, { MJTILE_HAKU, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG,  MJTILE_TON,  MJTILE_TON, MJTACHA_TYPE_TOIMEN },
      { MJFURO_TYPE_PUNG, MJTILE_HATU, MJTILE_HATU, MJTACHA_TYPE_TOIMEN },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_SHA,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 5, .fu = 50,
      .point = {
        .point = 8000,
        .feed.tsumo.oya = 4000,
        .feed.tsumo.ko = 2000,
      },
      .yaku_flags = MJSCORE_FLAG_BAKAZE | MJSCORE_FLAG_HATU | MJSCORE_FLAG_CHANTA | MJSCORE_FLAG_HONITSU,
    }
  },
  {
    .agarihai = MJTILE_2MAN,
    .tiles = {
      { MJTILE_2MAN, 2 }, { MJTILE_3MAN, 2 }, { MJTILE_4PIN, 2 },
      { MJTILE_8PIN, 2 }, { MJTILE_6SOU, 2 }, { MJTILE_7SOU, 2 },
      { MJTILE_8SOU, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_PEE,
    .tsumo = true, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 5, .fu = 25,
      .point = {
        .point = 8000,
        .feed.tsumo.oya = 4000,
        .feed.tsumo.ko = 2000,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_TSUMO | MJSCORE_FLAG_TANYAO | MJSCORE_FLAG_CHITOITSU,
    }
  },
  {
    .agarihai = MJTILE_2SOU,
    .tiles = {
      { MJTILE_2SOU, 2 }, { MJTILE_4SOU, 3 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG,  MJTILE_SHA,  MJTILE_SHA, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_PUNG, MJTILE_1PIN, MJTILE_1PIN, MJTACHA_TYPE_KAMICHA  },
      { MJFURO_TYPE_PUNG, MJTILE_8SOU, MJTILE_8SOU, MJTACHA_TYPE_TOIMEN   },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_SHA,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 40,
      .point = {
        .point = 5200,
        .feed.point = 5200,
      },
      .yaku_flags = MJSCORE_FLAG_TOITOIHO | MJSCORE_FLAG_JIKAZE,
    }
  },
  {
    .agarihai = MJTILE_NAN,
    .tiles = {
      { MJTILE_1SOU, 2 }, { MJTILE_1PIN, 2 }, { MJTILE_9PIN, 2 },
      { MJTILE_9SOU, 2 }, { MJTILE_HAKU, 2 }, { MJTILE_PEE,  2 },
      { MJTILE_NAN,  2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_SHA,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 4, .fu = 25,
      .point = {
        .point = 6400,
        .feed.point = 6400,
      },
      .yaku_flags = MJSCORE_FLAG_CHITOITSU | MJSCORE_FLAG_HONROUTO,
    }
  },
  {
    .agarihai = MJTILE_HAKU,
    .tiles = {
      { MJTILE_2PIN, 3 }, { MJTILE_HAKU, 2 }, { MJTILE_HATU, 3 },
      { MJTILE_CHUN, 3 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_CHOW, MJTILE_5SOU, MJTILE_5SOU, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_SHA,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 6, .fu = 50,
      .point = {
        .point = 12000,
        .feed.point = 12000,
      },
      .yaku_flags = MJSCORE_FLAG_HATU | MJSCORE_FLAG_CHUN | MJSCORE_FLAG_SYOSANGEN | MJSCORE_FLAG_SANANKO,
    }
  },
  {
    .agarihai = MJTILE_3MAN,
    .tiles = {
      { MJTILE_3MAN, 1 }, { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 },
      { MJTILE_1PIN, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_ANKAN,  MJTILE_HATU, MJTILE_HATU, MJTACHA_TYPE_INVALID  },
      { MJFURO_TYPE_KAKAN,  MJTILE_8MAN, MJTILE_8MAN, MJTACHA_TYPE_TOIMEN   },
      { MJFURO_TYPE_MINKAN, MJTILE_2SOU, MJTILE_2SOU, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_TON,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 70,
      .point = {
        .point = 12000,
        .feed.point = 12000,
      },
      .yaku_flags = MJSCORE_FLAG_HATU | MJSCORE_FLAG_SANKANTSU,
    }
  },
  {
    .agarihai = MJTILE_5MAN,
    .tiles = {
      { MJTILE_3MAN, 4 }, { MJTILE_4MAN, 4 }, { MJTILE_5MAN, 4 },
      { MJTILE_2PIN, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_SHA,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 6, .fu = 20,
      .point = {
        .point = 12000,
        .feed.tsumo.oya = 6000,
        .feed.tsumo.ko = 3000,
      },
      .yaku_flags = MJSCORE_FLAG_TSUMO | MJSCORE_FLAG_TANYAO | MJSCORE_FLAG_PINFU | MJSCORE_FLAG_RYANPEKO,
    }
  },
  {
    .agarihai = MJTILE_9SOU,
    .tiles = {
      { MJTILE_1SOU, 2 }, { MJTILE_2SOU, 2 }, { MJTILE_3SOU, 2 },
      { MJTILE_7SOU, 2 }, { MJTILE_8SOU, 2 }, { MJTILE_9SOU, 4 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_SHA,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 13, .fu = 30,
      .point = {
        .point = 32000,
        .feed.point = 32000,
      },
      .yaku_flags = MJSCORE_FLAG_CHINITSU | MJSCORE_FLAG_JYUNCHANTA | MJSCORE_FLAG_PINFU | MJSCORE_FLAG_RYANPEKO,
    }
  },
  {
    .agarihai = MJTILE_9PIN,
    .tiles = {
      { MJTILE_1PIN, 3 }, { MJTILE_9PIN, 3 }, { MJTILE_HAKU, 3 },
      { MJTILE_HATU, 2 }, { MJTILE_CHUN, 3 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_SHA,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 13, .fu = 60,
      .point = {
        .point = 32000,
        .feed.point = 32000,
      },
      .yaku_flags = MJSCORE_FLAG_HONROUTO | MJSCORE_FLAG_TOITOIHO | MJSCORE_FLAG_SANANKO | MJSCORE_FLAG_HONITSU | MJSCORE_FLAG_SYOSANGEN | MJSCORE_FLAG_CHUN | MJSCORE_FLAG_HAKU,
    }
  },
  {
    .agarihai = MJTILE_5MAN,
    .tiles = {
      { MJTILE_5MAN, 3 }, { MJTILE_3PIN, 1 }, { MJTILE_4PIN, 1 },
      { MJTILE_5PIN, 1 }, { MJTILE_7PIN, 1 }, { MJTILE_8PIN, 1 },
      { MJTILE_9PIN, 1 }, { MJTILE_4SOU, 1 }, { MJTILE_5SOU, 1 },
      { MJTILE_6SOU, 1 }, { MJTILE_8SOU, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 7,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_NAN,
    .tsumo = false, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 8, .fu = 40,
      .point = {
        .point = 16000,
        .feed.point = 16000,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_7MAN,
    .tiles = {
      { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 },
      { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { MJTILE_9MAN, 1 },
      { MJTILE_3PIN, 2 }, { MJTILE_5PIN, 1 }, { MJTILE_6PIN, 1 },
      { MJTILE_7PIN, 1 }, { MJTILE_7SOU, 1 }, { MJTILE_8SOU, 1 },
      { MJTILE_9SOU, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 2,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 40,
      .point = {
        .point = 5200,
        .feed.point = 5200,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_5MAN,
    .tiles = {
      { MJTILE_1MAN, 1 }, { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 },
      { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 },
      { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { MJTILE_9MAN, 1 },
      { MJTILE_6PIN, 2 }, { MJTILE_7SOU, 3 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_NAN,
    .tsumo = true, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 4, .fu = 30,
      .point = {
        .point = 7700,
        .feed.tsumo.oya = 3900,
        .feed.tsumo.ko = 2000,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_TSUMO | MJSCORE_FLAG_IKKITSUKAN,
    }
  },
  {
    .agarihai = MJTILE_9MAN,
    .tiles = {
      { MJTILE_1MAN, 1 }, { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 },
      { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { MJTILE_9MAN, 1 },
      { MJTILE_3PIN, 2 }, { MJTILE_5PIN, 1 }, { MJTILE_6PIN, 1 },
      { MJTILE_7PIN, 1 }, { MJTILE_6SOU, 1 }, { MJTILE_7SOU, 1 },
      { MJTILE_8SOU, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = true, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 4, .fu = 20,
      .point = {
        .point = 5200,
        .feed.tsumo.oya = 2600,
        .feed.tsumo.ko = 1300,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_TSUMO | MJSCORE_FLAG_PINFU | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_4MAN,
    .tiles = {
      { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 }, { MJTILE_4MAN, 3 },
      { MJTILE_5MAN, 2 }, { MJTILE_6MAN, 2 }, { MJTILE_4PIN, 2 },
      { MJTILE_3SOU, 3 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_NAN,
    .tsumo = true, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 5, .fu = 30,
      .point = {
        .point = 8000,
        .feed.tsumo.oya = 4000,
        .feed.tsumo.ko = 2000,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_TSUMO | MJSCORE_FLAG_TANYAO | MJSCORE_FLAG_IPEKO | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_7MAN,
    .tiles = {
      { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 }, { MJTILE_4MAN, 3 },
      { MJTILE_5MAN, 2 }, { MJTILE_6MAN, 2 }, { MJTILE_4PIN, 2 },
      { MJTILE_3SOU, 3 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_NAN,
    .tsumo = true, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 5, .fu = 30,
      .point = {
        .point = 8000,
        .feed.tsumo.oya = 4000,
        .feed.tsumo.ko = 2000,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_TSUMO | MJSCORE_FLAG_TANYAO | MJSCORE_FLAG_IPEKO | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_7MAN,
    .tiles = {
      { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { MJTILE_9MAN, 1 }, 
      { MJTILE_2PIN, 2 }, { MJTILE_6SOU, 1 }, { MJTILE_7SOU, 1 }, 
      { MJTILE_8SOU, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_ANKAN,  MJTILE_5MAN, MJTILE_5MAN, MJTACHA_TYPE_INVALID  },
      { MJFURO_TYPE_PUNG,   MJTILE_HATU, MJTILE_HATU, MJTACHA_TYPE_KAMICHA  },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 3,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_TON,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 4, .fu = 50,
      .point = {
        .point = 12000,
        .feed.point = 12000,
      },
      .yaku_flags = MJSCORE_FLAG_HATU | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_5MAN,
    .tiles = {
      { MJTILE_3MAN, 3 }, { MJTILE_5MAN, 2 }, { MJTILE_4PIN, 1 }, 
      { MJTILE_5PIN, 1 }, { MJTILE_6PIN, 1 }, { MJTILE_2SOU, 3 }, 
      { MJTILE_4SOU, 1 }, { MJTILE_5SOU, 1 }, { MJTILE_6SOU, 1 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 3,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_TON,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 4, .fu = 40,
      .point = {
        .point = 12000,
        .feed.point = 12000,
      },
      .yaku_flags = MJSCORE_FLAG_TANYAO | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_6MAN,
    .tiles = {
      { MJTILE_2MAN, 3 }, { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 }, 
      { MJTILE_6MAN, 1 }, { MJTILE_5PIN, 1 }, { MJTILE_6PIN, 1 }, 
      { MJTILE_7PIN, 1 }, { MJTILE_5SOU, 1 }, { MJTILE_6SOU, 1 },
      { MJTILE_7SOU, 1 }, { MJTILE_8SOU, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_NAN,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 30,
      .point = {
        .point = 3900,
        .feed.tsumo.oya = 2000,
        .feed.tsumo.ko = 1000,
      },
      .yaku_flags = MJSCORE_FLAG_TSUMO | MJSCORE_FLAG_TANYAO | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_9SOU,
    .tiles = {
      { MJTILE_2SOU, 1 }, { MJTILE_3SOU, 1 }, { MJTILE_4SOU, 2 }, 
      { MJTILE_5SOU, 1 }, { MJTILE_6SOU, 1 }, { MJTILE_7SOU, 1 }, 
      { MJTILE_8SOU, 1 }, { MJTILE_9SOU, 1 }, 
      { MJTILE_CHUN, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG,   MJTILE_9SOU, MJTILE_9SOU, MJTACHA_TYPE_TOIMEN  },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_NAN,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 2, .fu = 30,
      .point = {
        .point = 2000,
        .feed.point = 2000,
      },
      .yaku_flags = MJSCORE_FLAG_HONITSU,
    }
  },
  {
    .agarihai = MJTILE_TON,
    .tiles = {
      { MJTILE_3MAN, 1 }, { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 2 }, 
      { MJTILE_6MAN, 1 }, { MJTILE_7MAN, 1 }, { MJTILE_2PIN, 2 }, 
      { MJTILE_6SOU, 1 }, { MJTILE_7SOU, 1 }, { MJTILE_8SOU, 1 },
      { MJTILE_TON,  3 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 2,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_TON,
    .tsumo = false, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 4, .fu = 40,
      .point = {
        .point = 12000,
        .feed.point = 12000,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_JIKAZE | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_6SOU,
    .tiles = {
      { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 }, { MJTILE_7MAN, 1 }, 
      { MJTILE_6PIN, 2 }, { MJTILE_4SOU, 1 }, { MJTILE_5SOU, 1 }, 
      { MJTILE_6SOU, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_8MAN, MJTILE_8MAN, MJTACHA_TYPE_TOIMEN   },
      { MJFURO_TYPE_PUNG, MJTILE_NAN,  MJTILE_NAN,  MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_TON,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 1, .fu = 30,
      .point = {
        .point = 1500,
        .feed.point = 1500,
      },
      .yaku_flags = MJSCORE_FLAG_BAKAZE,
    }
  },
  {
    .agarihai = MJTILE_6MAN,
    .tiles = {
      { MJTILE_3MAN, 2 }, { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 }, 
      { MJTILE_7MAN, 1 }, { MJTILE_5PIN, 1 }, { MJTILE_6PIN, 1 }, 
      { MJTILE_7PIN, 2 }, { MJTILE_8PIN, 1 }, { MJTILE_9PIN, 1 },
      { MJTILE_5SOU, 1 }, { MJTILE_6SOU, 1 }, { MJTILE_7SOU, 1 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_SHA,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 40,
      .point = {
        .point = 5200,
        .feed.point = 5200,
      },
      .yaku_flags = MJSCORE_FLAG_SANSYOKUDOUJYUN | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_8SOU,
    .tiles = {
      { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 }, { MJTILE_4MAN, 1 }, 
      { MJTILE_5PIN, 2 }, { MJTILE_6PIN, 1 }, { MJTILE_7PIN, 1 }, 
      { MJTILE_8PIN, 1 }, { MJTILE_6SOU, 1 }, { MJTILE_7SOU, 1 },
      { MJTILE_8SOU, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_CHOW, MJTILE_5MAN, MJTILE_5MAN, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 2,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_PEE,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 30,
      .point = {
        .point = 3900,
        .feed.tsumo.oya = 2000,
        .feed.tsumo.ko = 1000,
      },
      .yaku_flags = MJSCORE_FLAG_TANYAO | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_2SOU,
    .tiles = {
      { MJTILE_1MAN, 1 }, { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 }, 
      { MJTILE_6MAN, 2 }, { MJTILE_3PIN, 3 }, { MJTILE_1SOU, 2 }, 
      { MJTILE_2SOU, 2 }, { MJTILE_3SOU, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 2,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 3, .fu = 40,
      .point = {
        .point = 5200,
        .feed.point = 5200,
      },
      .yaku_flags = MJSCORE_FLAG_IPEKO | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_9PIN,
    .tiles = {
      { MJTILE_9MAN, 2 }, { MJTILE_7PIN, 1 }, { MJTILE_8PIN, 1 }, 
      { MJTILE_9PIN, 1 }, { MJTILE_2SOU, 1 }, { MJTILE_3SOU, 1 },
      { MJTILE_4SOU, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_CHOW, MJTILE_5SOU, MJTILE_5SOU, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_PUNG, MJTILE_PEE,  MJTILE_PEE,  MJTACHA_TYPE_TOIMEN   },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_PEE,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 1, .fu = 30,
      .point = {
        .point = 1000,
        .feed.tsumo.oya = 500,
        .feed.tsumo.ko = 300,
      },
      .yaku_flags = MJSCORE_FLAG_JIKAZE,
    }
  },
  {
    .agarihai = MJTILE_3PIN,
    .tiles = {
      { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 }, { MJTILE_4MAN, 1 }, 
      { MJTILE_2PIN, 1 }, { MJTILE_3PIN, 1 }, { MJTILE_4PIN, 1 },
      { MJTILE_4SOU, 3 }, { MJTILE_5SOU, 1 }, { MJTILE_6SOU, 1 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_CHUN, MJTILE_CHUN, MJTACHA_TYPE_KAMICHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 2, .fu = 30,
      .point = {
        .point = 2000,
        .feed.point = 2000,
      },
      .yaku_flags = MJSCORE_FLAG_CHUN | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_4MAN,
    .tiles = {
      { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 }, 
      { MJTILE_1PIN, 1 }, { MJTILE_2PIN, 1 }, { MJTILE_3PIN, 1 },
      { MJTILE_6PIN, 1 }, { MJTILE_7PIN, 1 }, { MJTILE_8PIN, 1 },
      { MJTILE_3SOU, 1 }, { MJTILE_4SOU, 1 }, { MJTILE_5SOU, 1 },
      { MJTILE_7SOU, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_SHA,
    .tsumo = true, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 4, .fu = 20,
      .point = {
        .point = 5200,
        .feed.tsumo.oya = 2600,
        .feed.tsumo.ko = 1300,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_TSUMO | MJSCORE_FLAG_PINFU | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_4MAN,
    .tiles = {
      { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 }, { MJTILE_4MAN, 2 }, 
      { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 }, { MJTILE_3PIN, 1 },
      { MJTILE_4PIN, 1 }, { MJTILE_5PIN, 1 }, { MJTILE_4SOU, 2 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_CHOW, MJTILE_5SOU, MJTILE_6SOU, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_SHA,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 1, .fu = 30,
      .point = {
        .point = 1000,
        .feed.tsumo.oya = 500,
        .feed.tsumo.ko = 300,
      },
      .yaku_flags = MJSCORE_FLAG_TANYAO,
    }
  },
  {
    .agarihai = MJTILE_2PIN,
    .tiles = {
      { MJTILE_4MAN, 2 }, { MJTILE_2PIN, 3 }, { MJTILE_4SOU, 3 }, 
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_4PIN, MJTILE_4PIN, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_PUNG,  MJTILE_SHA,  MJTILE_SHA, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 2, .fu = 40,
      .point = {
        .point = 2700,
        .feed.tsumo.oya = 1300,
        .feed.tsumo.ko = 700,
      },
      .yaku_flags = MJSCORE_FLAG_TOITOIHO,
    }
  },
  {
    .agarihai = MJTILE_2PIN,
    .tiles = {
      { MJTILE_6MAN, 1 }, { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 },
      { MJTILE_1PIN, 2 }, { MJTILE_2PIN, 1 }, { MJTILE_3PIN, 1 }, 
      { MJTILE_4PIN, 1 }, { MJTILE_6PIN, 1 }, { MJTILE_7PIN, 1 }, 
      { MJTILE_8PIN, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_CHOW, MJTILE_6SOU, MJTILE_8SOU, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_TON,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 1, .fu = 30,
      .point = {
        .point = 1500,
        .feed.tsumo.oya = 0,
        .feed.tsumo.ko = 500,
      },
      .yaku_flags = MJSCORE_FLAG_SANSYOKUDOUJYUN,
    }
  },
  {
    .agarihai = MJTILE_8MAN,
    .tiles = {
      { MJTILE_3MAN, 2 }, { MJTILE_4MAN, 3 }, { MJTILE_7MAN, 1 },
      { MJTILE_8MAN, 1 }, { MJTILE_9MAN, 1 }, { MJTILE_1PIN, 1 }, 
      { MJTILE_2PIN, 1 }, { MJTILE_3PIN, 1 }, { MJTILE_2SOU, 1 }, 
      { MJTILE_3SOU, 1 }, { MJTILE_4SOU, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 1,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = true, .ippatsu = false,
    .answer = {
      .han = 2, .fu = 40,
      .point = {
        .point = 2600,
        .feed.point = 2600,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_DORA,
    }
  },
  {
    .agarihai = MJTILE_2SOU,
    .tiles = {
      { MJTILE_1MAN, 1 }, { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 },
      { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { MJTILE_9MAN, 1 }, 
      { MJTILE_1SOU, 3 }, { MJTILE_2SOU, 2 }, { MJTILE_3SOU, 2 }, 
      { MJTILE_4SOU, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_NAN, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = true, .ippatsu = true,
    .answer = {
      .han = 3, .fu = 30,
      .point = {
        .point = 3900,
        .feed.point = 3900,
      },
      .yaku_flags = MJSCORE_FLAG_RIICHI | MJSCORE_FLAG_IPPATSU | MJSCORE_FLAG_PINFU,
    }
  },
};

/* 役満手のテストケース */
static const struct MJScoreTestCase yakuman_test_cases[] = {
  {
    .agarihai = MJTILE_9MAN,
    .tiles = {
      { MJTILE_5MAN, 3 }, { MJTILE_9MAN, 3 }, { MJTILE_8SOU, 3 },
      { MJTILE_1PIN, 2 }, { MJTILE_8PIN, 3 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_TON,
    .tsumo = true, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 13, .fu = 0,
      .point = {
        .point = 48000,
        .feed.tsumo.oya = 0,
        .feed.tsumo.ko = 16000,
      },
      .yaku_flags = MJSCORE_FLAG_SUANKO,
    }
  },
  {
    .agarihai = MJTILE_CHUN,
    .tiles = {
      { MJTILE_6PIN, 3 }, { MJTILE_7PIN, 3 }, { MJTILE_1SOU, 3 },
      { MJTILE_HATU, 3 }, { MJTILE_CHUN, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_SHA,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 26, .fu = 0,
      .point = {
        .point = 64000,
        .feed.point = 64000,
      },
      .yaku_flags = MJSCORE_FLAG_SUANKOTANKI,
    }
  },
  {
    .agarihai = MJTILE_1MAN,
    .tiles = {
      { MJTILE_1MAN, 1 }, { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 },
      { MJTILE_4MAN, 2 }, { MJTILE_CHUN, 3 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_HAKU, MJTILE_HAKU, MJTACHA_TYPE_TOIMEN  },
      { MJFURO_TYPE_PUNG, MJTILE_HATU, MJTILE_HATU, MJTACHA_TYPE_KAMICHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_SHA,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 13, .fu = 0,
      .point = {
        .point = 32000,
        .feed.point = 32000,
      },
      .yaku_flags = MJSCORE_FLAG_DAISANGEN,
    }
  },
  {
    .agarihai = MJTILE_1SOU,
    .tiles = {
      { MJTILE_1MAN, 1 }, { MJTILE_9MAN, 1 }, { MJTILE_1PIN, 1 },
      { MJTILE_9PIN, 1 }, { MJTILE_1SOU, 1 }, { MJTILE_9SOU, 1 },
      { MJTILE_TON,  2 }, { MJTILE_NAN,  1 }, { MJTILE_SHA,  1 },
      { MJTILE_PEE,  1 }, { MJTILE_HAKU, 1 }, { MJTILE_HATU, 1 },
      { MJTILE_CHUN, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_SHA,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 13, .fu = 0,
      .point = {
        .point = 32000,
        .feed.point = 32000,
      },
      .yaku_flags = MJSCORE_FLAG_KOKUSHIMUSOU,
    }
  },
  {
    .agarihai = MJTILE_TON,
    .tiles = {
      { MJTILE_1MAN, 1 }, { MJTILE_9MAN, 1 }, { MJTILE_1PIN, 1 },
      { MJTILE_9PIN, 1 }, { MJTILE_1SOU, 1 }, { MJTILE_9SOU, 1 },
      { MJTILE_TON,  2 }, { MJTILE_NAN,  1 }, { MJTILE_SHA,  1 },
      { MJTILE_PEE,  1 }, { MJTILE_HAKU, 1 }, { MJTILE_HATU, 1 },
      { MJTILE_CHUN, 1 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_SHA,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 26, .fu = 0,
      .point = {
        .point = 64000,
        .feed.point = 64000,
      },
      .yaku_flags = MJSCORE_FLAG_KOKUSHIMUSOU13,
    }
  },
  {
    .agarihai = MJTILE_6SOU,
    .tiles = {
      { MJTILE_2SOU, 1 }, { MJTILE_3SOU, 1 }, { MJTILE_4SOU, 1 },
      { MJTILE_6SOU, 3 }, { MJTILE_8SOU, 2 }, { MJTILE_6SOU, 3 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_HATU, MJTILE_HATU, MJTACHA_TYPE_KAMICHA },
      { MJFURO_TYPE_CHOW, MJTILE_2SOU, MJTILE_3SOU, MJTACHA_TYPE_TOIMEN  },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_SHA,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 13, .fu = 0,
      .point = {
        .point = 32000,
        .feed.point = 32000,
      },
      .yaku_flags = MJSCORE_FLAG_RYUISO,
    }
  },
  {
    .agarihai = MJTILE_NAN,
    .tiles = {
      { MJTILE_NAN,  3 }, { MJTILE_PEE,  2 }, { MJTILE_HAKU, 3 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_HATU, MJTILE_HATU, MJTACHA_TYPE_TOIMEN  },
      { MJFURO_TYPE_PUNG, MJTILE_SHA,  MJTILE_SHA,  MJTACHA_TYPE_KAMICHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_TON,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 13, .fu = 0,
      .point = {
        .point = 48000,
        .feed.point = 48000,
      },
      .yaku_flags = MJSCORE_FLAG_TSUISO,
    }
  },
  {
    .agarihai = MJTILE_9MAN,
    .tiles = {
      { MJTILE_1MAN, 2 }, { MJTILE_9MAN, 3 }, { MJTILE_9SOU, 3 },
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_9PIN, MJTILE_9PIN, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_PUNG, MJTILE_1SOU, MJTILE_1SOU, MJTACHA_TYPE_KAMICHA  },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 13, .fu = 0,
      .point = {
        .point = 32000,
        .feed.point = 32000,
      },
      .yaku_flags = MJSCORE_FLAG_CHINROTO,
    }
  },
  {
    .agarihai = MJTILE_8MAN,
    .tiles = {
      { MJTILE_8MAN, 2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_MINKAN, MJTILE_1MAN, MJTILE_1MAN, MJTACHA_TYPE_KAMICHA  },
      { MJFURO_TYPE_ANKAN,  MJTILE_3PIN, MJTILE_3PIN, MJTACHA_TYPE_INVALID  },
      { MJFURO_TYPE_MINKAN, MJTILE_7SOU, MJTILE_7SOU, MJTACHA_TYPE_SHIMOCHA },
      { MJFURO_TYPE_MINKAN, MJTILE_PEE,  MJTILE_PEE,  MJTACHA_TYPE_SHIMOCHA },
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 13, .fu = 0,
      .point = {
        .point = 32000,
        .feed.point = 32000,
      },
      .yaku_flags = MJSCORE_FLAG_SUKANTSU,
    }
  },
  {
    .agarihai = MJTILE_1MAN,
    .tiles = {
      { MJTILE_1MAN, 2 }, { MJTILE_SHA,  3 }, { MJTILE_PEE,  3 }, 
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_NAN, MJTILE_NAN, MJTACHA_TYPE_TOIMEN  },
      { MJFURO_TYPE_PUNG, MJTILE_TON, MJTILE_TON, MJTACHA_TYPE_KAMICHA },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 13, .fu = 0,
      .point = {
        .point = 32000,
        .feed.point = 32000,
      },
      .yaku_flags = MJSCORE_FLAG_DAISUSHI,
    }
  },
  {
    .agarihai = MJTILE_1MAN,
    .tiles = {
      { MJTILE_1MAN, 3 }, { MJTILE_TON,  3 }, { MJTILE_SHA,  3 }, 
      { MJTILE_PEE,  2 }, { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_PUNG, MJTILE_NAN, MJTILE_NAN, MJTACHA_TYPE_SHIMOCHA  },
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 13, .fu = 0,
      .point = {
        .point = 32000,
        .feed.point = 32000,
      },
      .yaku_flags = MJSCORE_FLAG_SYOSUSHI,
    }
  },
  {
    .agarihai = MJTILE_2MAN,
    .tiles = {
      { MJTILE_1MAN, 3 }, { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 }, 
      { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 1 }, { MJTILE_6MAN, 1 }, 
      { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { MJTILE_9MAN, 4 }, 
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 13, .fu = 0,
      .point = {
        .point = 32000,
        .feed.point = 32000,
      },
      .yaku_flags = MJSCORE_FLAG_CHURENPOUTON,
    }
  },
  {
    .agarihai = MJTILE_5MAN,
    .tiles = {
      { MJTILE_1MAN, 3 }, { MJTILE_2MAN, 1 }, { MJTILE_3MAN, 1 }, 
      { MJTILE_4MAN, 1 }, { MJTILE_5MAN, 2 }, { MJTILE_6MAN, 1 }, 
      { MJTILE_7MAN, 1 }, { MJTILE_8MAN, 1 }, { MJTILE_9MAN, 3 }, 
      { -1, 0 },
    },
    .meld = {
      { MJFURO_TYPE_INVALID, 0, 0, 0 }
    },
    .num_dora = 0,
    .num_honba = 0, .num_riichibo = 0,
    .bakaze = MJKAZE_TON, .jikaze = MJKAZE_PEE,
    .tsumo = false, .riichi = false, .ippatsu = false,
    .answer = {
      .han = 26, .fu = 0,
      .point = {
        .point = 64000,
        .feed.point = 64000,
      },
      .yaku_flags = MJSCORE_FLAG_CHURENPOUTON9,
    }
  },
};

static int MJScoreTest_Initialize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

static int MJScoreTest_Finalize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

/* テストケースを和了情報に変換 */
static void MJScoreTest_ConvertTestCaseToAgariInformation(const struct MJScoreTestCase *test_case, struct MJAgariInformation *agari_info)
{
  int32_t i;

  assert((test_case != NULL) && (agari_info != NULL));

  /* 和了情報を一旦クリア */
  memset(agari_info, 0, sizeof(struct MJAgariInformation));

  /* 和了牌 */
  agari_info->agarihai = test_case->agarihai;

  /* 手牌を変換 */
  for (i = 0; i < 14; i++) {
    const struct TileInfo *info = &test_case->tiles[i];
    if (info->type < 0) {
      break;
    }
    agari_info->hand.hand[info->type] = info->maisu;
  }

  /* 副露を変換 */
  for (i = 0; i < 4; i++) {
    if (test_case->meld[i].type == MJFURO_TYPE_INVALID) {
      break;
    }
    agari_info->meld[i] = test_case->meld[i];
  }
  agari_info->num_meld = (uint8_t)i;

  agari_info->num_dora     = test_case->num_dora;
  agari_info->num_honba    = test_case->num_honba;
  agari_info->num_riichibo = test_case->num_riichibo;
  agari_info->bakaze       = test_case->bakaze;
  agari_info->jikaze       = test_case->jikaze;
  agari_info->tsumo        = test_case->tsumo;
  agari_info->riichi       = test_case->riichi;
  agari_info->ippatsu      = test_case->ippatsu;

  /* 未対応フラグはfalseをセット */
  agari_info->double_riichi = false;
  agari_info->haitei = false;
  agari_info->rinshan = false;
  agari_info->chankan = false;
  agari_info->nagashimangan = false;
  agari_info->tenho = false;
  agari_info->chiho = false;
}

/* 成立した役の表示 */
static void MJScoreTest_PrintEstablishedYakus(uint64_t yaku_flags)
{
#define SEPARATOR ", "
  uint32_t index;

  /* フラグが立っている役を全て表示 */
  for (index = 0; index < sizeof(yaku_name_table) / sizeof(yaku_name_table[0]); index++) {
    if (MJSCORE_IS_YAKU_ESTABLISHED(yaku_flags, index)) {
      printf("%s", yaku_name_table[index]);
      printf("%s", SEPARATOR);
    }
  }
#undef SEPARATOR
}

/* 得点情報の表示 */
static void MJScoreTest_PrintMJScore(const struct MJScore *score)
{
  assert(score != NULL);

  MJScoreTest_PrintEstablishedYakus(score->yaku_flags);
  printf("Han:%d, Fu:%d, ", score->han, score->fu);
  printf("Point:%d, FeedPoint:%d, FeedTsumoKo:%d, FeedTsumoOya:%d ",
      score->point.point, score->point.feed.point, 
      score->point.feed.tsumo.ko, score->point.feed.tsumo.oya);
}

/* 計算結果の一致確認 */
static bool MJScoreTest_CheckScoreResult(bool tsumo, const struct MJScore *get, const struct MJScore *answer)
{
  assert((get != NULL) && (answer != NULL));
  
  /* メンバに不一致項目があればfalse */
  if (get->han != answer->han) {
    return false;
  }
  if (get->fu != answer->fu) {
    return false;
  }
  if (get->yaku_flags != answer->yaku_flags) {
    return false;
  }
  if (tsumo) {
    if (get->point.feed.tsumo.ko != answer->point.feed.tsumo.ko) {
      return false;
    }
    if (get->point.feed.tsumo.oya != answer->point.feed.tsumo.oya) {
      return false;
    }
  } else {
    if (get->point.feed.point != answer->point.feed.point) {
      return false;
    }
  }

  return true;
}

/* テストケースに対して計算テスト実行 */
static void MJScoreTest_CalculateTestForList(const struct MJScoreTestCase *test_cases, uint32_t num_test)
{
  uint32_t i, is_ok;

  /* 全ケースで確認 */
  is_ok = 1;
  for (i = 0; i < num_test; i++) {
    struct MJScore score;
    struct MJAgariInformation agari_info;
    MJScoreCalculationResult ret;
    const struct MJScoreTestCase *pcase = &test_cases[i];
    MJScoreTest_ConvertTestCaseToAgariInformation(pcase, &agari_info);
    if ((ret = MJScore_CalculateScore(&agari_info, &score)) != MJSCORE_CALCRESULT_OK) {
      printf("NG at test case index:%d api result:%d \n", i, ret);
      is_ok = 0;
      break;
    }
    if (!MJScoreTest_CheckScoreResult(pcase->tsumo, &score, &(pcase->answer))) {
      printf("NG at test case index:%d \n", i);
      printf("Answer: "); MJScoreTest_PrintMJScore(&(pcase->answer)); printf("\n");
      printf("Get:    "); MJScoreTest_PrintMJScore(&score); printf("\n");
      is_ok = 0;
      break;
    }
  }

  /* 全正解を期待 */
  Test_AssertEqual(is_ok, 1);
}

/* 手作りの簡単なケースに対する得点計算 */
static void MJScoreTest_CalculateForEasycases(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  {
    MJScoreTest_CalculateTestForList(normal_test_cases, sizeof(normal_test_cases) / sizeof(normal_test_cases[0]));
    MJScoreTest_CalculateTestForList(yakuman_test_cases, sizeof(yakuman_test_cases) / sizeof(yakuman_test_cases[0]));
  }
}

/* 偶然役に対する得点計算 */
static void MJScoreTest_CalculateForOcassionalcases(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* 通常役に対しての海底・嶺上開花・槍槓・天和・地和の成立テスト */
  {
    uint32_t i, is_ok;
    const uint32_t num_test_cases = sizeof(normal_test_cases) / sizeof(normal_test_cases[0]);

    is_ok = 1;
    for (i = 0; i < num_test_cases; i++) {
      struct MJScore score, modified_score;
      struct MJAgariInformation agari_info, modified_info;
      MJScoreCalculationResult ret;

      /* まず偶然役がない場合の得点計算 */
      MJScoreTest_ConvertTestCaseToAgariInformation(&normal_test_cases[i], &agari_info);
      if ((ret = MJScore_CalculateScore(&agari_info, &score)) != MJSCORE_CALCRESULT_OK) {
        printf("%d: NG at test case index:%d api result:%d \n", __LINE__, i, ret);
        is_ok = 0;
        break;
      }

      /* ダブルリーチ */
      if (agari_info.riichi) {
        /* 立直をダブルリーチに差し替える */
        modified_info = agari_info;
        modified_info.riichi = false; /* 立直は消す */
        modified_info.double_riichi = true;
        if ((ret = MJScore_CalculateScore(&modified_info, &modified_score)) != MJSCORE_CALCRESULT_OK) {
          printf("%d: NG at test case index:%d api result:%d \n", __LINE__, i, ret);
          is_ok = 0;
          break;
        }
        /* 1翻上がっているはず */
        if (modified_score.han != (score.han + 1)) {
          printf("%d: NG at test case index:%d base fan:%d modified fan:%d \n",
              __LINE__, i, score.han, modified_score.han);
          is_ok = 0;
          break;
        }
        /* 役フラグのチェック */
        if (!MJSCORE_IS_YAKU_ESTABLISHED(modified_score.yaku_flags, MJSCORE_YAKU_DOUBLERIICHI)) {
          printf("%d: NG at test case index:%d \n", __LINE__, i);
          is_ok = 0;
          break;
        }
      }

      /* 海底 */
      modified_info = agari_info;
      modified_info.haitei = true;
      if ((ret = MJScore_CalculateScore(&modified_info, &modified_score)) != MJSCORE_CALCRESULT_OK) {
        printf("%d: NG at test case index:%d api result:%d \n", __LINE__, i, ret);
        is_ok = 0;
        break;
      }
      /* 1翻上がっているはず */
      if (modified_score.han != (score.han + 1)) {
        printf("%d: NG at test case index:%d base fan:%d modified fan:%d \n",
            __LINE__, i, score.han, modified_score.han);
        is_ok = 0;
        break;
      }
      /* 役フラグのチェック */
      if (!(agari_info.tsumo && MJSCORE_IS_YAKU_ESTABLISHED(modified_score.yaku_flags, MJSCORE_YAKU_HAITEITSUMO))
          && !(!agari_info.tsumo && MJSCORE_IS_YAKU_ESTABLISHED(modified_score.yaku_flags, MJSCORE_YAKU_HOUTEIRON))) {
        printf("%d: NG at test case index:%d \n", __LINE__, i);
        is_ok = 0;
        break;
      }

      /* 嶺上開花 */
      {
        struct MJHand merged_hand;

        /* カンが含まれているか確かめるために手牌にマージ */
        MJScore_MergeFuroToHand(&agari_info, &merged_hand);

        /* カンが含まれていれば嶺上開花を加えてテスト */
        if (MJScore_CountNumKan(&merged_hand) > 0) {
          modified_info = agari_info;
          modified_info.rinshan = true;
          if ((ret = MJScore_CalculateScore(&modified_info, &modified_score)) != MJSCORE_CALCRESULT_OK) {
            printf("%d: NG at test case index:%d api result:%d \n", __LINE__, i, ret);
            is_ok = 0;
            break;
          }
          /* 1翻上がっているはず */
          if (modified_score.han != (score.han + 1)) {
            printf("Origin: "); MJScoreTest_PrintMJScore(&score); printf("\n");
            printf("Get:    "); MJScoreTest_PrintMJScore(&modified_score); printf("\n");
            printf("%d: NG at test case index:%d base fan:%d modified fan:%d \n",
                __LINE__, i, score.han, modified_score.han);
            is_ok = 0;
            break;
          }
          /* 役フラグのチェック */
          if (!MJSCORE_IS_YAKU_ESTABLISHED(modified_score.yaku_flags, MJSCORE_YAKU_RINSHAN)) {
            printf("%d: NG at test case index:%d \n", __LINE__, i);
            is_ok = 0;
            break;
          }
        }
      }

      /* 槍槓: 明らかに成立しない七対子以外で調査 */
      /* 注意: 単騎待ちでないと成立しないがそれ以外も槍槓をつけてチェックしている */
      if (!MJSCORE_IS_YAKU_ESTABLISHED(score.yaku_flags, MJSCORE_YAKU_CHITOITSU)) {
        modified_info = agari_info;
        modified_info.chankan = true;
        if ((ret = MJScore_CalculateScore(&modified_info, &modified_score)) != MJSCORE_CALCRESULT_OK) {
          printf("%d: NG at test case index:%d api result:%d \n", __LINE__, i, ret);
          is_ok = 0;
          break;
        }
        /* 1翻上がっているはず */
        if (modified_score.han != (score.han + 1)) {
          printf("%d: NG at test case index:%d base fan:%d modified fan:%d \n",
              __LINE__, i, score.han, modified_score.han);
          is_ok = 0;
          break;
        }
        /* 役フラグのチェック */
        if (!MJSCORE_IS_YAKU_ESTABLISHED(modified_score.yaku_flags, MJSCORE_YAKU_CHANKAN)) {
          printf("%d: NG at test case index:%d \n", __LINE__, i);
          is_ok = 0;
          break;
        }
      }

      /* 天和 */
      modified_info = agari_info;
      modified_info.tenho = true;
      if ((ret = MJScore_CalculateScore(&modified_info, &modified_score)) != MJSCORE_CALCRESULT_OK) {
        printf("%d: NG at test case index:%d api result:%d \n", __LINE__, i, ret);
        is_ok = 0;
        break;
      }
      /* 役満以上になるはず */
      if (modified_score.han < MJSCORE_HAN_YAKUMAN) {
        printf("%d: NG at test case index:%d base fan:%d modified fan:%d \n",
            __LINE__, i, score.han, modified_score.han);
        is_ok = 0;
        break;
      }
      /* 役フラグのチェック */
      if (!MJSCORE_IS_YAKU_ESTABLISHED(modified_score.yaku_flags, MJSCORE_YAKU_TENHO)) {
        printf("%d: NG at test case index:%d \n", __LINE__, i);
        is_ok = 0;
        break;
      }

      /* 地和 */
      modified_info = agari_info;
      modified_info.chiho = true;
      if ((ret = MJScore_CalculateScore(&modified_info, &modified_score)) != MJSCORE_CALCRESULT_OK) {
        printf("%d: NG at test case index:%d api result:%d \n", __LINE__, i, ret);
        is_ok = 0;
        break;
      }
      /* 役満以上になるはず */
      if (modified_score.han < MJSCORE_HAN_YAKUMAN) {
        printf("%d: NG at test case index:%d base fan:%d modified fan:%d \n",
            __LINE__, i, score.han, modified_score.han);
        is_ok = 0;
        break;
      }
      /* 役フラグのチェック */
      if (!MJSCORE_IS_YAKU_ESTABLISHED(modified_score.yaku_flags, MJSCORE_YAKU_CHIHO)) {
        printf("%d: NG at test case index:%d \n", __LINE__, i);
        is_ok = 0;
        break;
      }
    }

    /* 全正解を期待 */
    Test_AssertEqual(is_ok, 1);
  }

  /* 天和・地和と他の役満の複合テスト（積み込みか？） */
  {
    uint32_t i, is_ok;
    const uint32_t num_test_cases = sizeof(yakuman_test_cases) / sizeof(yakuman_test_cases[0]);

    is_ok = 1;
    for (i = 0; i < num_test_cases; i++) {
      struct MJScore score, modified_score;
      struct MJAgariInformation agari_info, modified_info;
      MJScoreCalculationResult ret;

      /* まず偶然役がない場合の得点計算 */
      MJScoreTest_ConvertTestCaseToAgariInformation(&yakuman_test_cases[i], &agari_info);
      if ((ret = MJScore_CalculateScore(&agari_info, &score)) != MJSCORE_CALCRESULT_OK) {
        printf("%d: NG at test case index:%d api result:%d \n", __LINE__, i, ret);
        is_ok = 0;
        break;
      }

      /* 天和 */
      modified_info = agari_info;
      modified_info.tenho = true;
      if ((ret = MJScore_CalculateScore(&modified_info, &modified_score)) != MJSCORE_CALCRESULT_OK) {
        printf("%d: NG at test case index:%d api result:%d \n", __LINE__, i, ret);
        is_ok = 0;
        break;
      }
      /* ダブル役満以上になるはず */
      if (modified_score.han < MJSCORE_HAN_2YAKUMAN) {
        printf("%d: NG at test case index:%d base fan:%d modified fan:%d \n",
            __LINE__, i, score.han, modified_score.han);
        is_ok = 0;
        break;
      }
      /* 役フラグのチェック */
      if (!MJSCORE_IS_YAKU_ESTABLISHED(modified_score.yaku_flags, MJSCORE_YAKU_TENHO)) {
        printf("%d: NG at test case index:%d \n", __LINE__, i);
        is_ok = 0;
        break;
      }

      /* 地和 */
      modified_info = agari_info;
      modified_info.chiho = true;
      if ((ret = MJScore_CalculateScore(&modified_info, &modified_score)) != MJSCORE_CALCRESULT_OK) {
        printf("%d: NG at test case index:%d api result:%d \n", __LINE__, i, ret);
        is_ok = 0;
        break;
      }
      /* ダブル役満以上になるはず */
      if (modified_score.han < MJSCORE_HAN_2YAKUMAN) {
        printf("%d: NG at test case index:%d base fan:%d modified fan:%d \n",
            __LINE__, i, score.han, modified_score.han);
        is_ok = 0;
        break;
      }
      /* 役フラグのチェック */
      if (!MJSCORE_IS_YAKU_ESTABLISHED(modified_score.yaku_flags, MJSCORE_YAKU_CHIHO)) {
        printf("%d: NG at test case index:%d \n", __LINE__, i);
        is_ok = 0;
        break;
      }
    }

    /* 全正解を期待 */
    Test_AssertEqual(is_ok, 1);
  }

  /* 流し満貫テスト */
  {
    struct MJScore score;
    struct MJAgariInformation agari_info;

    /* まっさらな状態で流し満貫フラグだけ立てる */
    memset(&agari_info, 0, sizeof(struct MJAgariInformation));
    agari_info.nagashimangan = true;

    Test_AssertEqual(MJScore_CalculateScore(&agari_info, &score), MJSCORE_CALCRESULT_OK);
    /* 満貫(5翻)になるはず */
    Test_AssertEqual(score.han, 5);
    /* 役フラグチェック */
    Test_AssertEqual(score.yaku_flags, MJSCORE_FLAG_NAGASHIMANGAN);
  }
}

void MJScoreTest_Setup(void)
{
  struct TestSuite *suite
    = Test_AddTestSuite("MJ Score Test Suite",
        NULL, MJScoreTest_Initialize, MJScoreTest_Finalize);

  Test_AddTest(suite, MJScoreTest_CalculateForEasycases);
  Test_AddTest(suite, MJScoreTest_CalculateForOcassionalcases);
}
