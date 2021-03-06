#ifndef MJSCORE_H_INCLUDED
#define MJSCORE_H_INCLUDED

#include "mj_types.h"
#include <stdint.h>
#include <stdbool.h>

/* 役の識別番号 */
#define MJSCORE_YAKU_RIICHI            0   /* 立直 */
#define MJSCORE_YAKU_DOUBLERIICHI      1   /* ダブルリーチ */
#define MJSCORE_YAKU_IPPATSU           2   /* 一発 */
#define MJSCORE_YAKU_TSUMO             3   /* 門前清自摸和 */
#define MJSCORE_YAKU_TANYAO            4   /* 断么九 */
#define MJSCORE_YAKU_PINFU             5   /* 平和 */
#define MJSCORE_YAKU_IPEKO             6   /* 一盃口 */
#define MJSCORE_YAKU_BAKAZE            7   /* 場風 */
#define MJSCORE_YAKU_JIKAZE            8   /* 自風 */
#define MJSCORE_YAKU_HAKU              9   /* 白 */
#define MJSCORE_YAKU_HATU             10   /* 發 */
#define MJSCORE_YAKU_CHUN             11   /* 中 */
#define MJSCORE_YAKU_RINSHAN          12   /* 嶺上開花 */
#define MJSCORE_YAKU_CHANKAN          13   /* 槍槓 */
#define MJSCORE_YAKU_HAITEITSUMO      14   /* 海底摸月 */
#define MJSCORE_YAKU_HOUTEIRON        15   /* 河底撈魚 */
#define MJSCORE_YAKU_SANSYOKUDOUJYUN  16   /* 三色同順 */
#define MJSCORE_YAKU_IKKITSUKAN       17   /* 一気通貫 */
#define MJSCORE_YAKU_CHANTA           18   /* 混全帯么九 */
#define MJSCORE_YAKU_CHITOITSU        19   /* 七対子 */
#define MJSCORE_YAKU_TOITOIHO         20   /* 対々和 */
#define MJSCORE_YAKU_SANANKO          21   /* 三暗刻 */
#define MJSCORE_YAKU_HONROUTO         22   /* 混老頭 */
#define MJSCORE_YAKU_SANSYOKUDOUKOKU  23   /* 三色同刻 */
#define MJSCORE_YAKU_SANKANTSU        24   /* 三槓子 */
#define MJSCORE_YAKU_SYOSANGEN        25   /* 小三元 */
#define MJSCORE_YAKU_HONITSU          26   /* 混一色 */
#define MJSCORE_YAKU_JYUNCHANTA       27   /* 純全帯么九 */
#define MJSCORE_YAKU_RYANPEKO         28   /* 二盃口 */
#define MJSCORE_YAKU_CHINITSU         29   /* 清一色 */
#define MJSCORE_YAKU_TENHO            30   /* 天和 */
#define MJSCORE_YAKU_CHIHO            31   /* 地和 */
#define MJSCORE_YAKU_KOKUSHIMUSOU     32   /* 国士無双 */
#define MJSCORE_YAKU_KOKUSHIMUSOU13   33   /* 国士無双一三面待ち */
#define MJSCORE_YAKU_CHURENPOUTON     34   /* 九蓮宝燈 */
#define MJSCORE_YAKU_CHURENPOUTON9    35   /* 九蓮宝燈九面待ち */
#define MJSCORE_YAKU_SUANKO           36   /* 四暗刻 */
#define MJSCORE_YAKU_SUANKOTANKI      37   /* 四暗刻単騎待ち */
#define MJSCORE_YAKU_DAISUSHI         38   /* 大四喜 */
#define MJSCORE_YAKU_SYOSUSHI         39   /* 小四喜 */
#define MJSCORE_YAKU_DAISANGEN        40   /* 大三元 */
#define MJSCORE_YAKU_TSUISO           41   /* 字一色 */
#define MJSCORE_YAKU_CHINROTO         42   /* 清老頭 */
#define MJSCORE_YAKU_RYUISO           43   /* 緑一色 */
#define MJSCORE_YAKU_SUKANTSU         44   /* 四槓子 */
#define MJSCORE_YAKU_DORA             45   /* ドラ */
#define MJSCORE_YAKU_NAGASHIMANGAN    46   /* 流し満貫 */

/* 役整数をフラグ化 */
#define MJSCORE_YAKU_FLAG(yaku) (1ULL << (yaku))

/* 役の成立フラグが立っているか確認するマクロ */
#define MJSCORE_IS_YAKU_ESTABLISHED(flag, yaku) (((flag) & MJSCORE_YAKU_FLAG(yaku)) != 0)

/* 計算結果型 */
typedef enum MJScoreCalculationResultTag {
  MJSCORE_CALCRESULT_OK = 0,
  MJSCORE_CALCRESULT_INVALID_ARGUMENT,        /* 引数が不正 */
  MJSCORE_CALCRESULT_INVALID_AGARIHAI,        /* 和了牌が異常 */
  MJSCORE_CALCRESULT_MELD_RIICHI,             /* 鳴いてリーチしている */
  MJSCORE_CALCRESULT_INVALID_NUM_HAND,        /* 和了牌ふくめ牌数が14枚でない */
  MJSCORE_CALCRESULT_NOT_AGARI,               /* 和了っていない */
  MJSCORE_CALCRESULT_NOT_YAKU,                /* 役がつかなかった */
  MJSCORE_CALCRESULT_RINSHAN_WITHOUT_KAN,     /* カン無しで嶺上開花 */
  MJSCORE_CALCRESULT_RINSHAN_WITHOUT_TSUMO,   /* 自摸無しで嶺上開花 */
  MJSCORE_CALCRESULT_RIICHI_AND_DOUBLERIICHI, /* 立直とダブルリーチが両立している */
  MJSCORE_CALCRESULT_INVALID_WIND,            /* 無効な風情報が指定されている */
  MJSCORE_CALCRESULT_INVALID_HAND,            /* 手牌が不正 */
  MJSCORE_CALCRESULT_NG, 
} MJScoreCalculationResult;

/* 和了時の状態 */
struct MJAgariInformation {
  MJTile        winning_tile;   /* 和了牌 */
  struct MJHand hand;           /* 手牌 */
  int32_t       num_dora;       /* ドラ牌数(赤牌含む) */
  int32_t       num_honba;      /* 本場数 */
  int32_t       num_riichibo;   /* 供託リーチ棒数 */
  MJWind        round_wind;     /* 場風 */
  MJWind        player_wind;    /* 和了者の風（家） */
  bool          tsumo;          /* 自摸和了？ */
  bool          riichi;         /* 立直？ */
  bool          ippatsu;        /* 一発？ */
  bool          double_riichi;  /* ダブルリーチ？ */
  bool          haitei;         /* 海底？ */
  bool          rinshan;        /* 嶺上開花？ */
  bool          chankan;        /* 槍槓？ */
  bool          nagashimangan;  /* 流し満貫？ */
  bool          tenho;          /* 天和？ */
  bool          chiho;          /* 地和？ */
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
  bool nagashimangan;         /* 流し満貫あり？ */
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ルールコンフィグの取得 */
void MJScore_GetRuleConfig(struct MJScoreRuleConfig *rule_config);

/* ルールコンフィグの設定 */
void MJScore_SetRuleConfig(const struct MJScoreRuleConfig *rule_config);

/* ドラ数の数え上げ */
int32_t MJScore_CountNumDoras(const struct MJDoraTile *dora, const struct MJHand *hand, MJTile winning_tile);

/* 得点計算 */
MJScoreCalculationResult MJScore_CalculateScore(const struct MJAgariInformation *info, struct MJScore *score);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MJSCORE_H_INCLUDED */
