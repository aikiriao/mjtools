#ifndef MJ_SCORE_H_INCLUDED
#define MJ_SCORE_H_INCLUDED

#include "mj_shanten.h"
#include <stdint.h>
#include <stdbool.h>

/* 役成立フラグ */
#define MJSCORE_FLAG_RIICHI           (1ULL <<  0)   /* 立直 */
#define MJSCORE_FLAG_DOUBLERIICHI     (1ULL <<  1)   /* ダブルリーチ */
#define MJSCORE_FLAG_IPPATSU          (1ULL <<  2)   /* 一発 */
#define MJSCORE_FLAG_TSUMO            (1ULL <<  3)   /* 門前清自摸和 */
#define MJSCORE_FLAG_TANYAO           (1ULL <<  4)   /* 断么九 */
#define MJSCORE_FLAG_PINFU            (1ULL <<  5)   /* 平和 */
#define MJSCORE_FLAG_IPEKO            (1ULL <<  6)   /* 一盃口 */
#define MJSCORE_FLAG_BAKAZE           (1ULL <<  7)   /* 場風 */
#define MJSCORE_FLAG_JIKAZE           (1ULL <<  8)   /* 自風 */
#define MJSCORE_FLAG_HAKU             (1ULL <<  9)   /* 白 */
#define MJSCORE_FLAG_HATU             (1ULL << 10)   /* 發 */
#define MJSCORE_FLAG_CHUN             (1ULL << 11)   /* 中 */
#define MJSCORE_FLAG_RINSHAN          (1ULL << 12)   /* 嶺上開花 */
#define MJSCORE_FLAG_CHANKAN          (1ULL << 13)   /* 槍槓 */
#define MJSCORE_FLAG_HAITEITSUMO      (1ULL << 14)   /* 海底摸月 */
#define MJSCORE_FLAG_HOUTEIRON        (1ULL << 15)   /* 河底撈魚 */
#define MJSCORE_FLAG_SANSYOKUDOUJYUN  (1ULL << 16)   /* 三色同順 */
#define MJSCORE_FLAG_IKKITSUKAN       (1ULL << 17)   /* 一気通貫 */
#define MJSCORE_FLAG_CHANTA           (1ULL << 18)   /* 混全帯么九 */
#define MJSCORE_FLAG_CHITOITSU        (1ULL << 19)   /* 七対子 */
#define MJSCORE_FLAG_TOITOIHO         (1ULL << 20)   /* 対々和 */
#define MJSCORE_FLAG_SANANKO          (1ULL << 21)   /* 三暗刻 */
#define MJSCORE_FLAG_HONROUTO         (1ULL << 22)   /* 混老頭 */
#define MJSCORE_FLAG_SANSYOKUDOUKOKU  (1ULL << 23)   /* 三色同刻 */
#define MJSCORE_FLAG_SANKANTSU        (1ULL << 24)   /* 三槓子 */
#define MJSCORE_FLAG_SYOSANGEN        (1ULL << 25)   /* 小三元 */
#define MJSCORE_FLAG_HONITSU          (1ULL << 26)   /* 混一色 */
#define MJSCORE_FLAG_JYUNCHANTA       (1ULL << 27)   /* 純全帯么九 */
#define MJSCORE_FLAG_RYANPEKO         (1ULL << 28)   /* 二盃口 */
#define MJSCORE_FLAG_CHINITSU         (1ULL << 29)   /* 清一色 */
#define MJSCORE_FLAG_TENHO            (1ULL << 30)   /* 天和 */
#define MJSCORE_FLAG_CHIHO            (1ULL << 31)   /* 地和 */
#define MJSCORE_FLAG_KOKUSHIMUSOU     (1ULL << 32)   /* 国士無双 */
#define MJSCORE_FLAG_KOKUSHIMUSOU13   (1ULL << 33)   /* 国士無双一三面待ち */
#define MJSCORE_FLAG_CHURENPOUTON     (1ULL << 34)   /* 九蓮宝燈 */
#define MJSCORE_FLAG_CHURENPOUTON9    (1ULL << 35)   /* 九蓮宝燈九面待ち */
#define MJSCORE_FLAG_SUANKO           (1ULL << 36)   /* 四暗刻 */
#define MJSCORE_FLAG_SUANKOTANKI      (1ULL << 37)   /* 四暗刻単騎待ち */
#define MJSCORE_FLAG_DAISUSHI         (1ULL << 38)   /* 大四喜 */
#define MJSCORE_FLAG_SYOSUSHI         (1ULL << 39)   /* 小四喜 */
#define MJSCORE_FLAG_DAISANGEN        (1ULL << 40)   /* 大三元 */
#define MJSCORE_FLAG_TSUISO           (1ULL << 41)   /* 字一色 */
#define MJSCORE_FLAG_CHINROTO         (1ULL << 42)   /* 清老頭 */
#define MJSCORE_FLAG_RYUISO           (1ULL << 43)   /* 緑一色 */
#define MJSCORE_FLAG_SUKANTSU         (1ULL << 44)   /* 四槓子 */
#define MJSCORE_FLAG_DORA             (1ULL << 45)   /* ドラ */
#define MJSCORE_FLAG_NAGASHIMANGAN    (1ULL << 46)   /* 流し満貫 */

/* 役の成立フラグが立っているか確認するマクロ 0で不成立, 0以外で成立 */
#define MJSCORE_GET_YAKUFLAG(flag, yaku) ((flag) & (yaku))

/* API結果型 */
typedef enum MJScoreApiResultTag {
  MJSCORE_APIRESULT_OK = 0,
  MJSCORE_APIRESULT_INVALID_ARGUMENT, /* 引数が不正 */
  MJSCORE_APIRESULT_INVALID_AGARIHAI, /* 和了牌が異常 */
  MJSCORE_APIRESULT_INVALID_KAZE,     /* 風情報が無効 */
  MJSCORE_APIRESULT_FURO_RIICHI,      /* 鳴いてリーチしている */
  MJSCORE_APIRESULT_ABNORMAL_NUM_HAI, /* 牌数が14枚でない */
  MJSCORE_APIRESULT_NOT_AGARI,        /* 和了っていない */
  MJSCORE_APIRESULT_NOT_YAKU,         /* 役がつかなかった */
  MJSCORE_APIRESULT_NG, 
} MJScoreApiResult;

/* 副露の識別型 */
typedef enum MJMeldTypeTag {
  MJFURO_TYPE_INVALID = 0, /* 無効値 */
  MJFURO_TYPE_PUNG,        /* ポン   */
  MJFURO_TYPE_CHOW,        /* チー   */
  MJFURO_TYPE_ANKAN,       /* 暗槓   */
  MJFURO_TYPE_MINKAN,      /* 明槓   */
  MJFURO_TYPE_KAKAN,       /* 加槓   */
} MJMeldType;

/* 他家の識別型 */
typedef enum MJTachaTypeTag {
  MJTACHA_TYPE_INVALID = 0,  /* 無効値 */
  MJTACHA_TYPE_SHIMOCHA,     /* 下家   */
  MJTACHA_TYPE_TOIMEN,       /* 対面   */
  MJTACHA_TYPE_KAMICHA,      /* 上家   */
} MJTachaType;

/* 風 */
typedef enum MJKazeTag {
  MJKAZE_INVALID = 0,  /* 無効値 */
  MJKAZE_TON,          /* 東 */
  MJKAZE_NAN,          /* 南 */
  MJKAZE_SHA,          /* 西 */
  MJKAZE_PEE,          /* 北 */
} MJKaze;

/* 副露の状態 */
struct MJMeld {
  MJMeldType  type;     /* 種類 */
  uint8_t     minhai;   /* 副露を構成する牌で最小の識別整数を持つ牌 */
  uint8_t     nakihai;  /* 副露を構成する牌で鳴いた牌 */ /* TODO: もしかしたらいらない？ */
  MJTachaType tacha;    /* 上がった人から見た、鳴いた他家 */ /* TODO: もしかしたらいらない？ */
};

/* 和了時の状態 */
struct MJAgariInformation {
  uint8_t         agarihai;       /* 和了牌 */
  struct MJHand   hand;           /* 手牌(和了牌含む, 副露除く) */
  struct MJMeld   meld[4];        /* 副露情報 */
  uint8_t         num_meld;       /* 副露数 */
  uint8_t         num_dora;       /* ドラ牌数(赤牌含む) */
  uint8_t         num_honba;      /* 本場数 */
  uint8_t         num_riichibo;   /* 供託リーチ棒数 */
  MJKaze          bakaze;         /* 場風 */
  MJKaze          jikaze;         /* 自風 */
  bool            tsumo;          /* 自摸和了？ */
  bool            riichi;         /* 立直？ */
  bool            ippatsu;        /* 一発？ */
  bool            double_riichi;  /* ダブルリーチ？ */
  bool            haitei;         /* 海底？ */
  bool            rinsyan;        /* 嶺上開花？ */
  bool            cyankan;        /* 槍槓？ */
  bool            nagashimangan;  /* 流し満貫？ */
  bool            tenho;          /* 天和？ */
  bool            chiho;          /* 地和？ */
};

/* 得点情報 */
struct MJPoint {
  int32_t point;     /* 和了者の得点 */
  union {
    int32_t point;   /* 振り込んだ人の支払い点数 */
    struct {
      int32_t ko;    /* 自摸時の子の支払い点数 */
      int32_t oya;   /* 自摸時の親の支払い点数（親の和了なら0） */
    } tsumo;
  } feed;
};

/* 得点計算結果 */
struct MJScore {
  int32_t   han;              /* 翻                 */
  int32_t   fu;               /* 符                 */
  uint64_t  yaku_flags;       /* 成立した役のフラグ */
  struct    MJPoint point;    /* 得点情報           */
};

/* ルールに関する設定項目 */
struct MJScoreRuleConfig {
  bool kuitan;                /* 喰いタンあり？ */
  bool kokushi13_as_double;   /* 13面待ち国士無双をダブル役満にする？ */
  bool suankotanki_as_double; /* 四暗刻単騎待ちをダブル役満にする？ */
  bool ba1500;                /* 場1500？ */
  bool mangan_roundup;        /* 30符4飜/60符3飜を満貫として扱うか？ */
};

#ifdef __cplusplus
extern "C"
#endif /* __cplusplus */

/* ルールコンフィグの取得 */
MJScoreApiResult MJScore_GetRuleConfig(struct MJScoreRuleConfig *rule_config);

/* ルールコンフィグの設定 */
MJScoreApiResult MJScore_SetRuleConfig(const struct MJScoreRuleConfig *rule_config);

/* 得点計算 */
MJScoreApiResult MJScore_CalculateScore(const struct MJAgariInformation *info, struct MJScore *score);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MJ_SCORE_H_INCLUDED */
