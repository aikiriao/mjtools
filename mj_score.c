#include "mj_score.h"
#include "mj_tile.h"
#include "mj_utility.h"

#include <string.h>
#include <assert.h>

/* 役の成立フラグをセットする（立たせる）マクロ */
#define MJSCORE_SET_YAKUFLAG(flag, yaku) ((flag) |= (yaku))

/* nの倍数に切り上げ */
#define MJSCORE_ROUND_UP(val, n)          ((((val) + ((n) - 1)) / (n)) * (n))

/* 役満の翻数 */
#define MJSCORE_FAN_YAKUMAN    13
#define MJSCORE_FAN_2YAKUMAN   26
#define MJSCORE_FAN_3YAKUMAN   39
#define MJSCORE_FAN_4YAKUMAN   52
#define MJSCORE_FAN_5YAKUMAN   65
#define MJSCORE_FAN_6YAKUMAN   78
#define MJSCORE_FAN_7YAKUMAN   91

/* 手牌を構成する面子の種類 */
typedef enum MJMentsuTypeTag {
  MJMENTSU_TYPE_INVALID = 0, /* 無効値 */
  MJMENTSU_TYPE_PUNG,         /* ポン   */
  MJMENTSU_TYPE_CHOW,         /* チー   */
  MJMENTSU_TYPE_ANKAN,       /* 暗槓   */
  MJMENTSU_TYPE_MINKAN,      /* 明槓   */
  MJMENTSU_TYPE_ANKO,        /* 暗刻   */
  MJMENTSU_TYPE_SYUNTSU,     /* 順子   */
  MJMENTSU_TYPE_TOITSU       /* 対子   */
} MJMentsuType;

/* 面子の情報 */
struct MJMentsu {
  MJMentsuType  type;
  uint8_t       minhai;       /* 面子を構成する最小の牌 */
};

/* 面子が切り分けられた手牌 */
struct MJDividedHand {
  uint8_t atama;                /* 頭牌 */
  struct MJMentsu mentsu[4];    /* 面子 */
};

/* 役満手の判定 成立していたらtrueを返す 同時に翻数を計算 */
static bool MJScore_CalculateYakuman(
  const struct MJAgariInformation *info, const struct MJHand *merged_hand, struct MJScore *score);
/* 通常手の得点計算 高点法に従い、最大点数を持つ結果をscoreに入れる */
static void MJScore_CalculateNormal(
  const struct MJAgariInformation *info, const struct MJHand *merged_hand, struct MJScore *score);
/* 面子の組み合わせに関係のない翻数を加算 */
static void MJScore_AppendNormalHan(
    const struct MJAgariInformation *info, const struct MJHand *merged_hand, struct MJScore *score);
/* 翻/符から得点を計算 */
static void MJScore_CalculatePointFromHanFu(
    const struct MJAgariInformation *info, int32_t han, int32_t fu, struct MJPoint *point);

/* 面子の切り分け */
static void MJScore_DivideMentsu(
  const struct MJAgariInformation *info, const struct MJHand *merged_hand, 
  struct MJHand *hand, struct MJDividedHand *div_hand, int32_t num_mentsu, struct MJScore *score);
/* 通常手の翻/符/役特定 */
static void MJScore_CalculateYakuHanFu(
    const struct MJAgariInformation *info, const struct MJHand *merged_hand, const struct MJDividedHand *div_hand, struct MJScore *score);
/* 役の特定/翻数計算 */
static void MJScore_CalculateYakuHan(
    const struct MJAgariInformation *info, const struct MJHand *merged_hand, const struct MJDividedHand *div_hand, uint64_t *yaku_flags, int32_t *han);
/* 符計算 */
static void MJScore_CalculateFu(
    const struct MJAgariInformation *info, const struct MJDividedHand *div_hand, int32_t *fu);

/* 盃口数（同一順子数）のカウント */
static int32_t MJScore_CountNumPeko(const struct MJDividedHand *hand);
/* 手牌から九蓮宝燈が成立しているかチェック */
static bool MJScore_IsChurenpoutonCore(const struct MJHand *hand);

/* 各役が成立しているかチェックする関数 */
/* 面子の切り分けが必要な役 */
static bool MJScore_IsPinfu(const struct MJAgariInformation *info, const struct MJDividedHand *hand);           /* 平和 */
static bool MJScore_IsIpeko(const struct MJAgariInformation *info, const struct MJDividedHand *hand);           /* 一盃口 */
static bool MJScore_IsSansyokudoujyun(const struct MJAgariInformation *info, const struct MJDividedHand *hand); /* 三色同順 */
static bool MJScore_IsIkkitsukan(const struct MJAgariInformation *info, const struct MJDividedHand *hand);      /* 一気通貫 */
static bool MJScore_IsChanta(const struct MJAgariInformation *info, const struct MJDividedHand *hand);          /* 全帯幺 */
static bool MJScore_IsToitoiho(const struct MJAgariInformation *info, const struct MJDividedHand *hand);        /* 対々和 */
static bool MJScore_IsSananko(const struct MJAgariInformation *info, const struct MJDividedHand *hand);         /* 三暗刻 */
static bool MJScore_IsSansyokudoukoku(const struct MJAgariInformation *info, const struct MJDividedHand *hand); /* 三色同刻 */
static bool MJScore_IsJyunchanta(const struct MJAgariInformation *info, const struct MJDividedHand *hand);      /* 純全帯幺 */
static bool MJScore_IsRyanpeko(const struct MJAgariInformation *info, const struct MJDividedHand *hand);        /* 二盃口 */
/* 手牌の切り分けが不要な役 */
static bool MJScore_IsTanyao(const struct MJAgariInformation *info, const struct MJHand *merged_hand);          /* 断么九 */
static bool MJScore_IsHonrouto(const struct MJAgariInformation *info, const struct MJHand *merged_hand);        /* 混老頭 */
static bool MJScore_IsHonitsu(const struct MJAgariInformation *info, const struct MJHand *merged_hand);         /* 混一色 */
static bool MJScore_IsChinitsu(const struct MJAgariInformation *info, const struct MJHand *merged_hand);        /* 清一色 */
static bool MJScore_IsSyosangen(const struct MJAgariInformation *info, const struct MJHand *merged_hand);       /* 小三元 */
static bool MJScore_IsSankantsu(const struct MJAgariInformation *info, const struct MJHand *merged_hand);       /* 三槓子 */
static bool MJScore_IsKokushimusou13(const struct MJAgariInformation *info, const struct MJHand *merged_hand);  /* 国士無双13面待ち */
static bool MJScore_IsChurenpouton(const struct MJAgariInformation *info, const struct MJHand *merged_hand);    /* 九蓮宝燈 */
static bool MJScore_IsChurenpouton9(const struct MJAgariInformation *info, const struct MJHand *merged_hand);   /* 九蓮宝燈9面待ち */
static bool MJScore_IsSuanko(const struct MJAgariInformation *info, const struct MJHand *merged_hand);          /* 四暗刻 */
static bool MJScore_IsSuankoTanki(const struct MJAgariInformation *info, const struct MJHand *merged_hand);     /* 四暗刻単騎 */
static bool MJScore_IsDaisushi(const struct MJAgariInformation *info, const struct MJHand *merged_hand);        /* 大四喜 */
static bool MJScore_IsSyosushi(const struct MJAgariInformation *info, const struct MJHand *merged_hand);        /* 小四喜 */
static bool MJScore_IsDaisangen(const struct MJAgariInformation *info, const struct MJHand *merged_hand);       /* 大三元 */
static bool MJScore_IsTsuiso(const struct MJAgariInformation *info, const struct MJHand *merged_hand);          /* 字一色 */
static bool MJScore_IsChinroto(const struct MJAgariInformation *info, const struct MJHand *merged_hand);        /* 清老頭 */
static bool MJScore_IsRyuiso(const struct MJAgariInformation *info, const struct MJHand *merged_hand);          /* 緑一色 */
static bool MJScore_IsSukantsu(const struct MJAgariInformation *info, const struct MJHand *merged_hand);        /* 四槓子 */

/* ルール設定項目 */
static struct MJScoreRuleConfig st_rule_config = {
  .kuitan = true,
  .kokushi13_as_double = true,
  .suankotanki_as_double = true,
  .ba1500 = false,
  .mangan_roundup = false,
};

/* ルールコンフィグの取得 */
MJScoreApiResult MJScore_GetRuleConfig(struct MJScoreRuleConfig *rule_config)
{
  /* 引数チェック */
  if (rule_config == NULL) {
    return MJSCORE_APIRESULT_INVALID_ARGUMENT;
  }

  (*rule_config) = st_rule_config;
  return MJSCORE_APIRESULT_OK;
}

/* ルールコンフィグの設定 */
MJScoreApiResult MJScore_SetRuleConfig(const struct MJScoreRuleConfig *rule_config)
{
  /* 引数チェック */
  if (rule_config == NULL) {
    return MJSCORE_APIRESULT_INVALID_ARGUMENT;
  }

  st_rule_config = (*rule_config);
  return MJSCORE_APIRESULT_OK;
}

/* 得点計算 */
MJScoreApiResult MJScore_CalculateScore(const struct MJAgariInformation *info, struct MJScore *score)
{
  struct MJScore tmp_score;
  struct MJHand merged_hand;
  int32_t normal_syanten, chitoi_syanten, kokusi_syanten;

  /* 引数チェック */
  if ((info == NULL) || (score == NULL)) {
    return MJSCORE_APIRESULT_INVALID_ARGUMENT;
  }

  /* 和了牌が異常 */
  if (!MJTILE_IS_SUHAI(info->agarihai) && !MJTILE_IS_JIHAI(info->agarihai)) {
    return MJSCORE_APIRESULT_INVALID_AGARIHAI;
  }

  /* 風情報が無効 */
  if ((info->jikaze == MJKAZE_INVALID) || (info->bakaze == MJKAZE_INVALID)) {
    return MJSCORE_APIRESULT_INVALID_KAZE;
  }

  /* 鳴いてリーチしている */
  if ((info->num_meld > 0) && (info->riichi || info->double_riichi)) {
    return MJSCORE_APIRESULT_INVALID_KAZE;
  }

  /* 牌数が14枚あるかチェック */
  {
    int32_t i, num_hai;
    num_hai = 0;
    for (i = 0; i < MJTILE_MAX; i++) {
      num_hai += info->hand.hand[i];
    }
    num_hai += 3 * info->num_meld;
    if (num_hai != 14) {
      return MJSCORE_APIRESULT_ABNORMAL_NUM_HAI;
    }
  }

  /* 向聴数取得 */
  normal_syanten = MJShanten_CalculateNormalShanten(&(info->hand)) - 2 * info->num_meld;
  chitoi_syanten = MJShanten_CalculateChitoitsuShanten(&(info->hand));
  kokusi_syanten = MJShanten_CalculateKokushimusouShanten(&(info->hand));
  /* 和了ってない */
  if ((normal_syanten != -1)
      && (chitoi_syanten != -1) && (kokusi_syanten != -1)) {
    return MJSCORE_APIRESULT_NOT_AGARI;
  }

  /* 副露牌を手牌にマージ */
  {
    int32_t i;
    uint8_t *hai = &(merged_hand.hand[0]);
    const struct MJMeld *pmeld;

    merged_hand = info->hand;
    for (i = 0; i < info->num_meld; i++) {
      pmeld = &(info->meld[i]);
      switch (pmeld->type) {
        case MJFURO_TYPE_CHOW:
          hai[pmeld->minhai]++; hai[pmeld->minhai + 1]++; hai[pmeld->minhai + 2]++;
          break;
        case MJFURO_TYPE_PUNG:
          hai[pmeld->minhai] += 3;
          break;
        case MJFURO_TYPE_ANKAN:
        case MJFURO_TYPE_MINKAN:
        case MJFURO_TYPE_KAKAN:
          hai[pmeld->minhai] += 4;
          break;
        default:
          assert(0);
      }
    }
  }

  /* 得点情報の初期化 */
  tmp_score.han = tmp_score.fu = 0;
  memset(&(tmp_score.point), 0, sizeof(struct MJPoint));
  tmp_score.yaku_flags = 0;

  /* 流し満貫判定 */
  if (info->nagashimangan) {
    tmp_score.han = 5;
    /* 得点計算へ */
    goto CALCULATE_SCORE;
  }

  /* 国士無双判定 */
  if (kokusi_syanten == -1) {
    tmp_score.han = MJSCORE_FAN_YAKUMAN;
    /* 天和/地和判定 */
    if (info->tenho) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_TENHO);
      tmp_score.han = MJSCORE_FAN_2YAKUMAN;
    } else if (info->chiho) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_CHIHO);
      tmp_score.han = MJSCORE_FAN_2YAKUMAN;
    }
    /* 13面待ち */
    if (MJScore_IsKokushimusou13(info, &merged_hand)) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_KOKUSHIMUSOU13);
      if (st_rule_config.kokushi13_as_double) {
        tmp_score.han = MJSCORE_FAN_2YAKUMAN;
      }
    } else {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_KOKUSHIMUSOU);
    }
    /* 得点計算へ */
    goto CALCULATE_SCORE;
  }

  /* 七対子判定 */
  if ((chitoi_syanten == -1) && (normal_syanten > -1)) {
    /* 天和/地和判定 */
    if (info->tenho) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_TENHO);
      tmp_score.han = MJSCORE_FAN_YAKUMAN;
    } else if (info->chiho) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_CHIHO);
      tmp_score.han = MJSCORE_FAN_YAKUMAN;
    }
    /* 字一色 */
    if (MJScore_IsTsuiso(info, &merged_hand)) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_TSUISO);
      tmp_score.han = (tmp_score.han == MJSCORE_FAN_YAKUMAN) ? MJSCORE_FAN_2YAKUMAN : MJSCORE_FAN_YAKUMAN;
    }
    /* 役満成立時は終わり */
    if (tmp_score.han >= MJSCORE_FAN_YAKUMAN) {
      goto CALCULATE_SCORE;
    }
    /* 七対子の基本翻/符をセット */
    MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_CHITOITSU);
    tmp_score.han = 2; tmp_score.fu = 25;
    /* 自摸 */
    if (info->tsumo) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_TSUMO);
      tmp_score.han += 1;
    }
    /* 立直/ダブルリーチ */
    if (info->double_riichi) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_DOUBLERIICHI);
      tmp_score.han += 2;
    } else if (info->riichi) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_RIICHI);
      tmp_score.han += 1;
    }
    /* 一発 */
    if ((info->riichi) && (info->ippatsu)) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_IPPATSU);
      tmp_score.han += 1;
    }
    /* 海底摸月/河底撈魚 */
    if ((info->haitei) && (info->tsumo)) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_HAITEITSUMO);
      tmp_score.han += 1;
    } else if ((info->haitei) && !(info->tsumo)) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_HOUTEIRON);
      tmp_score.han += 1;
    }
    /* ドラ */
    if (info->num_dora > 0) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_DORA);
      tmp_score.han += info->num_dora;
    }
    /* 混老頭 */
    if (MJScore_IsHonrouto(info, &merged_hand)) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_HONROUTO);
      tmp_score.han += 2;
    }
    /* 清一色 */
    if (MJScore_IsChinitsu(info, &merged_hand)) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_CHINITSU);
      tmp_score.han += 6;
    }
    /* 混一色 */
    if (MJScore_IsHonitsu(info, &merged_hand)) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_HONITSU);
      tmp_score.han += 3;
    }
    /* 断么九 */
    if (MJScore_IsTanyao(info, &merged_hand)) {
      MJSCORE_SET_YAKUFLAG(tmp_score.yaku_flags, MJSCORE_FLAG_TANYAO);
      tmp_score.han += 1;
    }
    /* 得点計算へ */
    goto CALCULATE_SCORE;
  }

  /* 国士無双以外の役満判定/翻符計算 */
  if (MJScore_CalculateYakuman(info, &merged_hand, &tmp_score)) {
    /* 得点計算へ */
    goto CALCULATE_SCORE;
  }

  /* 通常役の判定/翻符計算 */
  MJScore_CalculateNormal(info, &merged_hand, &tmp_score);
  /* 面子の関係ない部分での翻数を加算 */
  MJScore_AppendNormalHan(info, &merged_hand, &tmp_score);

  /* 翻/符から実際の得点計算へ */
CALCULATE_SCORE:

  /* 役がついていない（おそらく実装ミス） */
  if (tmp_score.han == 0) {
    return MJSCORE_APIRESULT_NOT_YAKU;
  }

  /* 得点計算 */
  MJScore_CalculatePointFromHanFu(info, tmp_score.han, tmp_score.fu, &(tmp_score.point));

  /* 成功 */
  (*score) = tmp_score;

  return MJSCORE_APIRESULT_OK;
}

/* 役満手の判定 成立していたらtrueを返す 同時に翻数を計算 */
static bool MJScore_CalculateYakuman(
  const struct MJAgariInformation *info, const struct MJHand *merged_hand, struct MJScore *score)
{
  struct MJScore tmp;

  assert((info != NULL) && (score != NULL));

  tmp.han = tmp.fu = 0;
  tmp.yaku_flags = 0;

  /* 天和 */
  if (info->tenho) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_TENHO);
    tmp.han += MJSCORE_FAN_YAKUMAN;
  }
  /* 地和 */
  if (info->chiho) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_CHIHO);
    tmp.han += MJSCORE_FAN_YAKUMAN;
  }
  /* 九蓮宝燈 */
  if (MJScore_IsChurenpouton(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_CHURENPOUTON);
    tmp.han += MJSCORE_FAN_YAKUMAN;
    goto YAKUMAN_EXIT;
  }
  /* 九蓮宝燈9面待ち */
  if (MJScore_IsChurenpouton9(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_CHURENPOUTON9);
    tmp.han += MJSCORE_FAN_2YAKUMAN;
    goto YAKUMAN_EXIT;
  }
  /* 四暗刻単騎待ち */
  if (MJScore_IsSuankoTanki(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_SUANKOTANKI);
    tmp.han += MJSCORE_FAN_YAKUMAN;
    /* 四暗刻単騎をダブル役満にする場合 */
    if (st_rule_config.suankotanki_as_double) {
      tmp.han += MJSCORE_FAN_YAKUMAN;
    }
  }
  /* 四暗刻 */
  if (MJScore_IsSuanko(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_SUANKO);
    tmp.han += MJSCORE_FAN_YAKUMAN;
  }
  /* 緑一色 */
  if (MJScore_IsRyuiso(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_RYUISO);
    tmp.han += MJSCORE_FAN_YAKUMAN;
  }
  /* 清老頭 */
  if (MJScore_IsChinroto(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_CHINROTO);
    tmp.han += MJSCORE_FAN_YAKUMAN;
  }
  /* 大四喜 */
  if (MJScore_IsDaisushi(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_DAISUSHI);
    tmp.han += MJSCORE_FAN_YAKUMAN;
  }
  /* 小四喜 */
  if (MJScore_IsSyosushi(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_SYOSUSHI);
    tmp.han += MJSCORE_FAN_YAKUMAN;
  }
  /* 字一色 */
  if (MJScore_IsTsuiso(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_TSUISO);
    tmp.han += MJSCORE_FAN_YAKUMAN;
  }
  /* 四槓子 */
  if (MJScore_IsSukantsu(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_SUKANTSU);
    tmp.han += MJSCORE_FAN_YAKUMAN;
  }
  /* 大三元 */
  if (MJScore_IsDaisangen(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_DAISANGEN);
    tmp.han += MJSCORE_FAN_YAKUMAN;
  }

YAKUMAN_EXIT:
  /* 役満成立 */
  if (tmp.han >= MJSCORE_FAN_YAKUMAN) {
    (*score) = tmp;
    return true;
  }

  return false;
}

/* 通常手の得点計算 高点法に従い、最大翻/符を持つ結果をscoreに入れる */
static void MJScore_CalculateNormal(
    const struct MJAgariInformation *info, const struct MJHand *merged_hand, struct MJScore *score)
{
  int32_t i;
  struct MJHand hand;
  struct MJDividedHand div_hand;

  assert((info != NULL) && (score != NULL));

  /* 分割済み手牌のクリア */
  memset(&div_hand, 0, sizeof(struct MJDividedHand));

  /* 副露牌を分割済み手牌に入れる */
  for (i = 0; i < info->num_meld; i++) {
    struct MJMentsu *pmentsu = &(div_hand.mentsu[i]);
    pmentsu->minhai = info->meld[i].minhai;
    /* 副露の種類の読み替え */
    switch (info->meld[i].type) {
      case MJFURO_TYPE_PUNG: pmentsu->type = MJMENTSU_TYPE_PUNG; break;
      case MJFURO_TYPE_CHOW: pmentsu->type = MJMENTSU_TYPE_CHOW; break;
      case MJFURO_TYPE_ANKAN: pmentsu->type = MJMENTSU_TYPE_ANKAN; break;
      case MJFURO_TYPE_MINKAN:
      case MJFURO_TYPE_KAKAN:
        pmentsu->type = MJMENTSU_TYPE_MINKAN;
        break;
      default: assert(0);
    }
  }

  /* 手牌の面子を切り分けつつ翻/符の計算へ */
  hand = info->hand;
  for (i = 0; i < MJTILE_MAX; i++) {
    /* 頭を抜いて調べる */
    if (hand.hand[i] >= 2) {
      hand.hand[i] -= 2;
      div_hand.atama = (uint8_t)i;
      /* 面子の切り分けに進む */
      MJScore_DivideMentsu(info, merged_hand, &hand, &div_hand, info->num_meld, score);
      div_hand.atama = 0;
      hand.hand[i] += 2;
    }
  }
}

/* 面子の切り分け */
static void MJScore_DivideMentsu(
  const struct MJAgariInformation *info, const struct MJHand *merged_hand, 
  struct MJHand *hand, struct MJDividedHand *div_hand, int32_t num_mentsu, struct MJScore *score)
{
  int32_t i;
  uint8_t *hai;
  struct MJMentsu *pmentsu;

  assert((info != NULL) && (hand != NULL) && (score != NULL));

  /* 面子が4つになったら切り分け終了 */
  if (num_mentsu >= 4) {
    /* 翻/符の計算へ */
    MJScore_CalculateYakuHanFu(info, merged_hand, div_hand, score);
    return;
  }

  hai = &(hand->hand[0]);
  pmentsu = &(div_hand->mentsu[num_mentsu]);
  for (i = 0; i < MJTILE_MAX; i++) {
    /* 暗刻を抜き出して調べる */
    if (hai[i] >= 3) {
      pmentsu->minhai = (uint8_t)i;
      if ((i == info->agarihai) && !(info->tsumo)) {
        /* 和了牌の場合は明刻に */
        pmentsu->type = MJMENTSU_TYPE_PUNG;
      } else {
        pmentsu->type = MJMENTSU_TYPE_ANKO;
      }
      hai[i] -= 3;
      MJScore_DivideMentsu(info, merged_hand, hand, div_hand, num_mentsu + 1, score);
      hai[i] += 3;
      pmentsu->minhai = 0;
      pmentsu->type = MJMENTSU_TYPE_INVALID;
    }
    /* 順子を抜き出して調べる */
    if (MJTILE_IS_SUHAI(i) && (hai[i] > 0)
        && (hai[i + 1] > 0) && (hai[i + 2] > 0)) {
      pmentsu->minhai = (uint8_t)i;
      pmentsu->type = MJMENTSU_TYPE_SYUNTSU;
      hai[i]--; hai[i + 1]--; hai[i + 2]--;
      MJScore_DivideMentsu(info, merged_hand, hand, div_hand, num_mentsu + 1, score);
      hai[i]++; hai[i + 1]++; hai[i + 2]++;
      pmentsu->minhai = 0;
      pmentsu->type = MJMENTSU_TYPE_INVALID;
    }
  }
}

/* 通常手の翻/符/役特定 */
static void MJScore_CalculateYakuHanFu(
    const struct MJAgariInformation *info, const struct MJHand *merged_hand, const struct MJDividedHand *div_hand, struct MJScore *score)
{
  int32_t han, fu;
  uint64_t yaku_flags;

  assert((info != NULL) && (div_hand != NULL) && (score != NULL));

  /* 仮の情報をクリア */
  han = fu = 0;
  yaku_flags = 0;

  /* 役判定/翻数確定 */
  MJScore_CalculateYakuHan(info, merged_hand, div_hand, &yaku_flags, &han);

  /* 符計算 */
  MJScore_CalculateFu(info, div_hand, &fu);

  /* ここまでで最大の翻符数だったら特典情報更新 */
  if ((han > score->han) || ((han == score->han) && (fu > score->fu))) {
    score->han = han;
    score->fu = fu;
    score->yaku_flags = yaku_flags;
  }
}

/* 役の特定/翻数計算 */
static void MJScore_CalculateYakuHan(
    const struct MJAgariInformation *info, const struct MJHand *merged_hand, const struct MJDividedHand *div_hand,
    uint64_t *yaku_flags, int32_t *han)
{
  int32_t tmp_han;
  uint64_t tmp_yaku_flags;

  assert((info != NULL) && (div_hand != NULL) && (yaku_flags != NULL) && (han != NULL));

  /* 情報をクリア */
  tmp_han = 0;
  tmp_yaku_flags = 0;

  /* 副露なしの役 */
  if (info->num_meld == 0) {
    /* 平和 */
    if (MJScore_IsPinfu(info, div_hand)) {
      MJSCORE_SET_YAKUFLAG(tmp_yaku_flags, MJSCORE_FLAG_PINFU);
      tmp_han += 1;
    }
    /* 二盃口 */
    if (MJScore_IsRyanpeko(info, div_hand)) {
      MJSCORE_SET_YAKUFLAG(tmp_yaku_flags, MJSCORE_FLAG_RYANPEKO);
      tmp_han += 2;
    } 
    /* 一盃口 */
    if (MJScore_IsIpeko(info, div_hand)) {
      MJSCORE_SET_YAKUFLAG(tmp_yaku_flags, MJSCORE_FLAG_IPEKO);
      tmp_han += 1;
    }
  }
  /* 一気通貫 */
  if (MJScore_IsIkkitsukan(info, div_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp_yaku_flags, MJSCORE_FLAG_IKKITSUKAN);
    /* 食い下がり */
    if (info->num_meld > 0) {
      tmp_han += 1;
    } else {
      tmp_han += 2;
    }
  }
  /* 三色同順 */
  if (MJScore_IsSansyokudoujyun(info, div_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp_yaku_flags, MJSCORE_FLAG_SANSYOKUDOUJYUN);
    /* 食い下がり */
    if (info->num_meld > 0) {
      tmp_han += 1;
    } else {
      tmp_han += 2;
    }
  }
  /* 三色同刻 */
  if (MJScore_IsSansyokudoukoku(info, div_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp_yaku_flags, MJSCORE_FLAG_SANSYOKUDOUKOKU);
    tmp_han += 2;
  }
  /* 混老頭 */
  if (MJScore_IsHonrouto(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp_yaku_flags, MJSCORE_FLAG_HONROUTO);
    tmp_han += 2;
  } 
  /* 純全帯么九 */
  if (MJScore_IsJyunchanta(info, div_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp_yaku_flags, MJSCORE_FLAG_JYUNCHANTA);
    /* 食い下がり */
    if (info->num_meld > 0) {
      tmp_han += 2;
    } else {
      tmp_han += 3;
    }
  } 
  /* 混全帯么九 */
  if (MJScore_IsChanta(info, div_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp_yaku_flags, MJSCORE_FLAG_CHANTA);
    /* 食い下がり */
    if (info->num_meld > 0) {
      tmp_han += 1;
    } else {
      tmp_han += 2;
    }
  }
  /* 対々和 */
  if (MJScore_IsToitoiho(info, div_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp_yaku_flags, MJSCORE_FLAG_TOITOIHO);
    tmp_han += 2;
  }
  /* 三暗刻 */
  if (MJScore_IsSananko(info, div_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp_yaku_flags, MJSCORE_FLAG_SANANKO);
    tmp_han += 2;
  }

  /* 結果の反映 */
  (*han) = tmp_han;
  (*yaku_flags) = tmp_yaku_flags;
}

/* 符計算 */
static void MJScore_CalculateFu(
    const struct MJAgariInformation *info, const struct MJDividedHand *div_hand, int32_t *fu)
{
  int32_t i;
  int32_t tmp_fu;
  const struct MJMentsu *pmentsu;

  assert((info != NULL) && (div_hand != NULL) && (fu != NULL));

  /* 平和 */
  if (MJScore_IsPinfu(info, div_hand)) {
    if (info->tsumo) {
      (*fu) = 20;
    } else {
      (*fu) = 30;
    }
    /* 計算終了 */
    return;
  }

  /* 副底による符 */
  tmp_fu = 20;
  /* 門前による符 */
  if ((info->num_meld == 0) && !(info->tsumo)) {
    tmp_fu += 10;
  }
  /* 自摸による符 */
  if (info->tsumo) {
    tmp_fu += 2;
  }
  /* 待ち牌による符 */
  /* 単騎待ち */
  if (info->agarihai == div_hand->atama) {
    tmp_fu += 2;
  } else {
    for (i = 0; i < 4; i++) {
      pmentsu = &(div_hand->mentsu[i]);
      /* 辺張/嵌張待ち */
      if (pmentsu->type == MJMENTSU_TYPE_SYUNTSU) {
        if ((MJTILE_NUMBER_IS(info->agarihai, 3) && (info->agarihai == (pmentsu->minhai + 2)))
            || (MJTILE_NUMBER_IS(info->agarihai, 1) && (info->agarihai == (pmentsu->minhai)))
            || (info->agarihai == (pmentsu->minhai + 1))) {
          tmp_fu += 2;
          break;
        }
      }
    }
  }
  /* 雀頭の状態による符 */
  /* 役牌 */
  if (MJTILE_IS_SANGEN(div_hand->atama)
      || ((div_hand->atama == MJTILE_TON) && (info->bakaze == MJKAZE_TON))
      || ((div_hand->atama == MJTILE_NAN) && (info->bakaze == MJKAZE_NAN))
      || ((div_hand->atama == MJTILE_SHA) && (info->bakaze == MJKAZE_SHA))
      || ((div_hand->atama == MJTILE_PEE) && (info->bakaze == MJKAZE_PEE))) {
    tmp_fu += 2;
  }
  /* 連風牌 */
  if (   ((div_hand->atama == MJTILE_TON) && (info->jikaze == MJKAZE_TON))
      || ((div_hand->atama == MJTILE_NAN) && (info->jikaze == MJKAZE_NAN))
      || ((div_hand->atama == MJTILE_SHA) && (info->jikaze == MJKAZE_SHA))
      || ((div_hand->atama == MJTILE_PEE) && (info->jikaze == MJKAZE_PEE))) {
    tmp_fu += 2;
  }
  /* 面子の構成による符 */
  for (i = 0; i < 4; i++) {
    pmentsu = &(div_hand->mentsu[i]);
    switch (pmentsu->type) {
      /* 暗刻 */
      case MJMENTSU_TYPE_ANKO:
        if (MJTILE_IS_YAOCHU(pmentsu->minhai)) {
          tmp_fu += 8;
        } else {
          tmp_fu += 4;
        }
        break;
        /* 明刻（ポン） */
      case MJMENTSU_TYPE_PUNG:
        if (MJTILE_IS_YAOCHU(pmentsu->minhai)) {
          tmp_fu += 4;
        } else {
          tmp_fu += 2;
        }
        break;
        /* 暗槓 */
      case MJMENTSU_TYPE_ANKAN:
        if (MJTILE_IS_YAOCHU(pmentsu->minhai)) {
          tmp_fu += 32;
        } else {
          tmp_fu += 16;
        }
        break;
        /* 明槓（加槓） */
      case MJMENTSU_TYPE_MINKAN:
        if (MJTILE_IS_YAOCHU(pmentsu->minhai)) {
          tmp_fu += 16;
        } else {
          tmp_fu += 8;
        }
        break;
      default:
        break;
    }
  }
  /* ここまで副底からの加符がないなら、30符に */
  if (tmp_fu == 20) {
    tmp_fu = 30;
  }
  /* 1の位を切り上げる */
  tmp_fu = MJSCORE_ROUND_UP(tmp_fu, 10);

  /* 結果の反映 */
  (*fu) = tmp_fu;
}

/* 面子の組み合わせに関係のない翻数を加算 */
static void MJScore_AppendNormalHan(
    const struct MJAgariInformation *info, const struct MJHand *merged_hand, struct MJScore *score)
{
  struct MJScore tmp;

  assert((info != NULL) && (merged_hand != NULL) && (score != NULL));

  tmp = (*score);

  /* 門前自摸 */
  if ((info->num_meld == 0) && (info->tsumo)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_TSUMO);
    tmp.han += 1;
  }
  /* 立直/ダブルリーチ */
  if (info->riichi) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_RIICHI);
    tmp.han += 1;
  } else if (info->double_riichi) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_DOUBLERIICHI);
    tmp.han += 2;
  }
  /* 一発 */
  if (info->ippatsu && ((info->riichi) || (info->double_riichi))) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_IPPATSU);
    tmp.han += 1;
  }
  /* ドラ */
  if (info->num_dora > 0) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_DORA);
    tmp.han += info->num_dora;
  }
  /* 海底摸月/河底撈魚 */
  if ((info->haitei) && (info->tsumo)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_HAITEITSUMO);
    tmp.han += 1;
  } else if ((info->haitei) && !(info->tsumo)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_HOUTEIRON);
    tmp.han += 1;
  }
  /* 混老頭 */
  if (MJScore_IsHonrouto(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_HONROUTO);
    tmp.han += 2;
  }
  /* 清一色 */
  if (MJScore_IsChinitsu(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_CHINITSU);
    /* 食い下がり */
    if (info->num_meld > 0) {
      tmp.han += 5;
    } else {
      tmp.han += 6;
    }
  }
  /* 混一色 */
  if (MJScore_IsHonitsu(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_HONITSU);
    /* 食い下がり */
    if (info->num_meld > 0) {
      tmp.han += 2;
    } else {
      tmp.han += 3;
    }
  }
  /* 断么九 */
  if (MJScore_IsTanyao(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_TANYAO);
    tmp.han += 1;
  }
  /* 三槓子 */
  if (MJScore_IsSankantsu(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_SANKANTSU);
    tmp.han += 2;
  }
  /* 小三元 */
  if (MJScore_IsSyosangen(info, merged_hand)) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_SYOSANGEN);
    tmp.han += 2;
  }
  /* 槍槓 */
  if (info->rinsyan) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_CHANKAN);
    tmp.han += 1;
  }
  /* 嶺上開花 */
  if (info->rinsyan) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_RINSHAN);
    tmp.han += 1;
  }
  /* 役牌 */
  /* 白 */
  if (merged_hand->hand[MJTILE_HAKU] >= 3) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_HAKU);
    tmp.han += 1;
  }
  /* 發 */
  if (merged_hand->hand[MJTILE_HATU] >= 3) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_HATU);
    tmp.han += 1;
  }
  /* 中 */
  if (merged_hand->hand[MJTILE_CHUN] >= 3) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_CHUN);
    tmp.han += 1;
  }
  /* 場風 */
  if (   ((info->bakaze == MJKAZE_TON) && (merged_hand->hand[MJTILE_TON] >= 3))
      || ((info->bakaze == MJKAZE_NAN) && (merged_hand->hand[MJTILE_NAN] >= 3))
      || ((info->bakaze == MJKAZE_SHA) && (merged_hand->hand[MJTILE_SHA] >= 3))
      || ((info->bakaze == MJKAZE_PEE) && (merged_hand->hand[MJTILE_PEE] >= 3))) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_BAKAZE);
    tmp.han += 1;
  }
  /* 自風 */
  if (   ((info->jikaze == MJKAZE_TON) && (merged_hand->hand[MJTILE_TON] >= 3))
      || ((info->jikaze == MJKAZE_NAN) && (merged_hand->hand[MJTILE_NAN] >= 3))
      || ((info->jikaze == MJKAZE_SHA) && (merged_hand->hand[MJTILE_SHA] >= 3))
      || ((info->jikaze == MJKAZE_PEE) && (merged_hand->hand[MJTILE_PEE] >= 3))) {
    MJSCORE_SET_YAKUFLAG(tmp.yaku_flags, MJSCORE_FLAG_JIKAZE);
    tmp.han += 1;
  }

  /* 成功終了 */
  (*score) = tmp;
}

/* 翻/符から得点を計算 */
static void MJScore_CalculatePointFromHanFu(
    const struct MJAgariInformation *info, int32_t han, int32_t fu, struct MJPoint *point)
{
  int32_t oya_point, ko_point;
  int32_t basic_point, tsumibo_point;
  struct MJPoint tmp;
  /* 支払い情報 */
  struct {
    int32_t ko_ron;     /* 子のロン和了で支払う点数 */
    int32_t oya_ron;    /* 親のロン和了で支払う点数 */
    int32_t oya_tsumo;  /* 親の自摸和了で子が支払う点数 */
    struct {            /* 子の自摸和了で支払う点数 */
      int32_t oya;      /* 親 */
      int32_t ko;       /* 子 */
    } ko_tsumo;         
  } payment;

  assert((info != NULL) && (point != NULL));

  /* 積み棒 */
  if (st_rule_config.ba1500) {
    tsumibo_point = 1500 * info->num_honba;
  } else {
    tsumibo_point = 300 * info->num_honba;
  }

  /* 基本点の決定 */
  switch (han) {
    /* 4翻以下 */
    case 1: case 2: case 3: case 4:
      basic_point = fu * (1 << (han + 2));
      basic_point = (basic_point >= 2000) ? 2000 : basic_point;
      break;
    /* 満貫 */
    case 5:                    basic_point =     2000; break;
    /* 跳満 */
    case 6: case 7:            basic_point =     3000; break;
    /* 倍満 */
    case 8: case 9: case 10:   basic_point =     4000; break;
    /* 3倍満 */
    case 11: case 12:          basic_point =     6000; break;
    /* 役満 */
    case MJSCORE_FAN_YAKUMAN:  basic_point =     8000; break;
    /* ダブル役満 */
    case MJSCORE_FAN_2YAKUMAN: basic_point = 2 * 8000; break;
    /* トリプル役満 */
    case MJSCORE_FAN_3YAKUMAN: basic_point = 3 * 8000; break;
    /* 4倍役満 */
    case MJSCORE_FAN_4YAKUMAN: basic_point = 4 * 8000; break;
    /* 5倍役満 */
    case MJSCORE_FAN_5YAKUMAN: basic_point = 5 * 8000; break;
    /* 6倍役満 */
    case MJSCORE_FAN_6YAKUMAN: basic_point = 6 * 8000; break;
    /* 7倍役満 */
    case MJSCORE_FAN_7YAKUMAN: basic_point = 7 * 8000; break;
    /* 実装ミス */
    default: assert(0);
  }

  /* 満貫切り上げ */
  if (st_rule_config.mangan_roundup) {
		if (((han == 4) && (fu == 30)) || ((han == 3) && (fu == 60))) {
			basic_point = 2000;
    }
  }

  /* 支払う点数の振り分け(100点切り上げ) */
  payment.oya_ron      = MJSCORE_ROUND_UP(6 * basic_point, 100);
  payment.ko_ron       = MJSCORE_ROUND_UP(4 * basic_point, 100);
  payment.oya_tsumo    = MJSCORE_ROUND_UP(2 * basic_point, 100);
  payment.ko_tsumo.oya = MJSCORE_ROUND_UP(2 * basic_point, 100);
  payment.ko_tsumo.ko  = MJSCORE_ROUND_UP(1 * basic_point, 100);

  /* 積み棒を加算 */
  payment.oya_ron      += tsumibo_point;
  payment.ko_ron       += tsumibo_point;
  payment.oya_tsumo    += tsumibo_point / 3;
  payment.ko_tsumo.oya += tsumibo_point / 3;
  payment.ko_tsumo.ko  += tsumibo_point / 3;

  /* 得点には供託リーチ棒を加算 */
  oya_point = payment.oya_ron + 1000 * info->num_riichibo;
  ko_point  = payment.ko_ron  + 1000 * info->num_riichibo;

  /* 出力バッファに結果を設定 */
  memset(&tmp, 0, sizeof(struct MJPoint));
  if (info->jikaze == MJKAZE_TON) {
    /* 親 */
    tmp.point             = oya_point;
    if (info->tsumo) {
      tmp.feed.tsumo.ko   = payment.oya_tsumo;
      tmp.feed.tsumo.oya  = 0;
    } else {
      tmp.feed.point      = payment.oya_ron;
    }
  } else {
    /* 子 */
    tmp.point             = ko_point;
    if (info->tsumo) {
      tmp.feed.tsumo.ko   = payment.ko_tsumo.ko;
      tmp.feed.tsumo.oya  = payment.ko_tsumo.oya;
    } else {
      tmp.feed.point      = payment.ko_ron;
    }
  }

  /* 成功終了 */
  (*point) = tmp;
}

/* 盃口数（同一順子数）のカウント */
static int32_t MJScore_CountNumPeko(const struct MJDividedHand *hand)
{
  int32_t i, j, num_peko;

  assert(hand != NULL);

  /* 同一種の順子をカウント */
  num_peko = 0;
  for (i = 0; i < 4; i++) {
    if ((hand->mentsu[i].type == MJMENTSU_TYPE_SYUNTSU) || (hand->mentsu[i].type == MJMENTSU_TYPE_CHOW)) {
      for (j = i + 1; j < 4; j++) {
        if (hand->mentsu[i].minhai == hand->mentsu[j].minhai) {
          num_peko++;
        }
      }
    }
  }

  return num_peko;
}

/* 平和が成立しているか？ */
static bool MJScore_IsPinfu(
    const struct MJAgariInformation *info, const struct MJDividedHand *hand)
{
  int32_t i;
  const struct MJMentsu *pmentsu;

  assert((info != NULL) && (hand != NULL));

  /* 鳴いている */
  if (info->num_meld > 0) {
    return false;
  }

  /* 雀頭が役牌 */
  if (MJTILE_IS_JIHAI(hand->atama)) {
    if ((hand->atama == MJTILE_HAKU)
        || (hand->atama == MJTILE_HATU) || (hand->atama == MJTILE_CHUN)) {
      return false;
    }
    if (((hand->atama == MJTILE_TON) && (info->bakaze == MJKAZE_TON))
        || ((hand->atama == MJTILE_NAN) && (info->bakaze == MJKAZE_NAN))
        || ((hand->atama == MJTILE_SHA) && (info->bakaze == MJKAZE_SHA))
        || ((hand->atama == MJTILE_PEE) && (info->bakaze == MJKAZE_PEE))
        || ((hand->atama == MJTILE_TON) && (info->jikaze == MJKAZE_TON))
        || ((hand->atama == MJTILE_NAN) && (info->jikaze == MJKAZE_NAN))
        || ((hand->atama == MJTILE_SHA) && (info->jikaze == MJKAZE_SHA))
        || ((hand->atama == MJTILE_PEE) && (info->jikaze == MJKAZE_PEE))) {
      return false;
    }
  }

  /* 順子以外が出現 */
  for (i = 0; i < 4; i++) {
    pmentsu = &hand->mentsu[i];
    if (pmentsu->type != MJMENTSU_TYPE_SYUNTSU) {
      return false;
    }
  }

  /* 両面待ちだったか？ */
  for (i = 0; i < 4; i++) {
    pmentsu = &hand->mentsu[i];
    /* 辺張待ちは弾く */
    if (((info->agarihai == pmentsu->minhai) && !MJTILE_NUMBER_IS(pmentsu->minhai + 1, 8))
        || ((info->agarihai == (pmentsu->minhai + 2)) && !MJTILE_NUMBER_IS(pmentsu->minhai, 1))) {
      return true;
    }
  }

  return false;
}

/* 一盃口が成立しているか？ */
static bool MJScore_IsIpeko(
    const struct MJAgariInformation *info, const struct MJDividedHand *hand)
{
  assert((info != NULL) && (hand != NULL));

  /* 鳴いている */
  if (info->num_meld > 0) {
    return false;
  }

  /* 盃口数で判断 */
  return (MJScore_CountNumPeko(hand) == 1) ? true : false;
}

/* 三色同順が成立しているか？ */
static bool MJScore_IsSansyokudoujyun(
    const struct MJAgariInformation *info, const struct MJDividedHand *hand)
{
  int32_t i;

  assert((info != NULL) && (hand != NULL));

  /* 数牌かつ同じ3つの並びを探す */
  for (i = 0; i < 2; i++) {
    if (MJTILE_IS_SUHAI(hand->mentsu[i].minhai)
        && MJTILE_IS_SUHAI(hand->mentsu[i + 1].minhai)
        && MJTILE_IS_SUHAI(hand->mentsu[i + 2].minhai)) {
      uint8_t ref_number = hand->mentsu[i].minhai % 10;
      if (MJTILE_NUMBER_IS(hand->mentsu[i + 1].minhai, ref_number)
          && MJTILE_NUMBER_IS(hand->mentsu[i + 2].minhai, ref_number)) {
        return true;
      }
    }
  }

  /* 無かった */
  return false;
}

/* 一気通貫が成立しているか？ */
static bool MJScore_IsIkkitsukan(
    const struct MJAgariInformation *info, const struct MJDividedHand *hand)
{
  int32_t i, j;
  uint8_t suhai_count[30] = { 0, };

  /* 数牌の並びに関してチェック */
  MJUTILITY_STATIC_ASSERT(
         (MJTILE_1MAN ==  1) && (MJTILE_9MAN ==  9)
      && (MJTILE_1PIN == 11) && (MJTILE_9PIN == 19)
      && (MJTILE_1SOU == 21) && (MJTILE_9SOU == 29));

  assert((info != NULL) && (hand != NULL));

  /* 数牌の出現数をカウント */
  for (i = 0; i < 4; i++) {
    uint8_t minhai = hand->mentsu[i].minhai;
    if (MJTILE_IS_SUHAI(minhai)
        && ((hand->mentsu[i].type == MJMENTSU_TYPE_SYUNTSU) || (hand->mentsu[i].type == MJMENTSU_TYPE_CHOW))) {
      suhai_count[minhai]++; suhai_count[minhai + 1]++; suhai_count[minhai + 2]++;
    }
  }

  /* 一つの種で揃っていたら成立 */
  for (i = 0; i < 3; i++) {
    for (j = 1; j <= 9; j++) {
      if (suhai_count[10 * i + j] == 0) {
        break;
      }
    }
    /* 最後まで行っていたら成立 */
    if (j == 10) {
      return true;
    }
  }

  /* 揃っている種が無かった */
  return false;
}

/* 全帯幺が成立しているか？ */
static bool MJScore_IsChanta(const struct MJAgariInformation *info, const struct MJDividedHand *hand)
{
  int32_t i;
  const struct MJMentsu *pmentsu;
  bool jihai;

  assert((info != NULL) && (hand != NULL));

  jihai = false;

  /* 頭牌は么九牌か？ */
  if (!MJTILE_IS_YAOCHU(hand->atama)) {
    return false;
  } else if (MJTILE_IS_JIHAI(hand->atama)) {
    jihai = true;
  }

  /* 全ての面子に対してチェック */
  for (i = 0; i < 4; i++) {
    pmentsu = &(hand->mentsu[i]);
    if ((pmentsu->type == MJMENTSU_TYPE_PUNG) || (pmentsu->type == MJMENTSU_TYPE_ANKAN)
        || (pmentsu->type == MJMENTSU_TYPE_MINKAN) || (pmentsu->type == MJMENTSU_TYPE_ANKO)
        || (pmentsu->type == MJMENTSU_TYPE_TOITSU)) {
      /* 構成牌が么九牌でない */
      if (!MJTILE_IS_YAOCHU(pmentsu->minhai)) {
        return false;
      }
    } else {
      /* チー/順子の構成牌が么九牌に絡んでいない */
      assert((pmentsu->type == MJMENTSU_TYPE_CHOW) || (pmentsu->type == MJMENTSU_TYPE_SYUNTSU));
      if (!MJTILE_NUMBER_IS(pmentsu->minhai, 1) && !MJTILE_NUMBER_IS(pmentsu->minhai, 7)) {
        return false;
      }
    }
    /* 字牌の出現をマーク */
    if (MJTILE_IS_JIHAI(pmentsu->minhai)) {
      jihai = true;
    }
  }

  /* 字牌の出現を要求 */
  return jihai;
}

/* 対々和が成立しているか？ */
static bool MJScore_IsToitoiho(const struct MJAgariInformation *info, const struct MJDividedHand *hand)
{
  int32_t i;
  const struct MJMentsu *pmentsu;

  assert((info != NULL) && (hand != NULL));

  /* 順子（チー）が含まれていたら不成立 */
  for (i = 0; i < 4; i++) {
    pmentsu = &(hand->mentsu[i]);
    if ((pmentsu->type == MJMENTSU_TYPE_CHOW) || (pmentsu->type == MJMENTSU_TYPE_SYUNTSU)) {
      return false;
    }
  }

  return true;
}

/* 三暗刻が成立しているか？ */
static bool MJScore_IsSananko(const struct MJAgariInformation *info, const struct MJDividedHand *hand)
{
  int32_t i, num_anko;
  const struct MJMentsu *pmentsu;

  assert((info != NULL) && (hand != NULL));

  /* 暗刻（暗槓）をカウント */
  num_anko = 0;
  for (i = 0; i < 4; i++) {
    pmentsu = &(hand->mentsu[i]);
    if ((pmentsu->type == MJMENTSU_TYPE_ANKO) || (pmentsu->type == MJMENTSU_TYPE_ANKAN)) {
      num_anko++;
    }
  }

  return (num_anko >= 3) ? true : false;
}

/* 三色同刻が成立しているか？ */
static bool MJScore_IsSansyokudoukoku(const struct MJAgariInformation *info, const struct MJDividedHand *hand)
{
  int32_t i, j, num_dokoku;

  assert((info != NULL) && (hand != NULL));

  /* 3種類の面子が同じ牌からなるポン/暗刻/カンか？ */
  for (i = 0; i < 4; i++) {
    if (MJTILE_IS_SUHAI(hand->mentsu[i].minhai)) {
      /* 同刻数を数える */
      num_dokoku = 0;
      for (j = i + 1; j < 4; j++) {
        if (MJTILE_IS_SAME_NUMBER(hand->mentsu[i].minhai, hand->mentsu[j].minhai)
            && ((hand->mentsu[i].type != MJMENTSU_TYPE_CHOW) && (hand->mentsu[i].type != MJMENTSU_TYPE_SYUNTSU))
            && ((hand->mentsu[j].type != MJMENTSU_TYPE_CHOW) && (hand->mentsu[j].type != MJMENTSU_TYPE_SYUNTSU))) {
          num_dokoku++;
        }
      }
      /* 同刻が自分以外に2つあれば成立 */
      if (num_dokoku == 2) {
        return true;
      }
    }
  }

  return false;
}

/* 純全帯幺が成立しているか？ */
static bool MJScore_IsJyunchanta(const struct MJAgariInformation *info, const struct MJDividedHand *hand)
{
  int32_t i;
  const struct MJMentsu *pmentsu;
  bool syuntsu;

  assert((info != NULL) && (hand != NULL));

  /* 頭牌は老頭牌か？ */
  if (!MJTILE_IS_ROUTOU(hand->atama)) {
    return false;
  }

  /* 全ての面子に対してチェック */
  syuntsu = false;
  for (i = 0; i < 4; i++) {
    pmentsu = &(hand->mentsu[i]);
    if ((pmentsu->type == MJMENTSU_TYPE_PUNG) || (pmentsu->type == MJMENTSU_TYPE_ANKAN)
        || (pmentsu->type == MJMENTSU_TYPE_MINKAN) || (pmentsu->type == MJMENTSU_TYPE_ANKO)
        || (pmentsu->type == MJMENTSU_TYPE_TOITSU)) {
      /* 構成牌が老頭牌でない */
      if (!MJTILE_IS_ROUTOU(pmentsu->minhai)) {
        return false;
      }
    } else {
      /* チー/順子の構成牌が么九牌に絡んでいない */
      assert((pmentsu->type == MJMENTSU_TYPE_CHOW) || (pmentsu->type == MJMENTSU_TYPE_SYUNTSU));
      if (!MJTILE_NUMBER_IS(pmentsu->minhai, 1) && !MJTILE_NUMBER_IS(pmentsu->minhai, 7)) {
        return false;
      }
      /* 順子の出現をマーク */
      syuntsu = true;
    }
    /* 字牌が出現したら不成立 */
    if (MJTILE_IS_JIHAI(pmentsu->minhai)) {
      return false;
    }
  }

  /* 順子が出現したかどうかで判定 memo:混老頭との混同防止 */
  return syuntsu;
}

/* 二盃口が成立しているか？ */
static bool MJScore_IsRyanpeko(const struct MJAgariInformation *info, const struct MJDividedHand *hand)
{
  assert((info != NULL) && (hand != NULL));

  /* 鳴いている */
  if (info->num_meld > 0) {
    return false;
  }

  /* 盃口数で判断 */
  return (MJScore_CountNumPeko(hand) == 2) ? true : false;
}

/* 断么九が成立しているか？ */
static bool MJScore_IsTanyao(
    const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i;

  assert((info != NULL) && (merged_hand != NULL));

  /* 喰いタン判定 喰いタンなしで、ポン・チー・カンがあったらfalse */
  if (!(st_rule_config.kuitan)) {
    if (info->num_meld > 0) {
      return false;
    }
  }

  /* 么九牌が含まれていたらfalse */
  for (i = 0; i < MJTILE_MAX; i++) {
    if (MJTILE_IS_YAOCHU(i) && (merged_hand->hand[i] > 0)) {
      return false;
    }
  }

  return true;
}

/* 混老頭が成立しているか？ */
static bool MJScore_IsHonrouto(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i;
  const uint8_t *hai;
  bool routou, jihai;

  assert((info != NULL) && (merged_hand != NULL));
  
  routou = jihai = false;
  hai = &(merged_hand->hand[0]);
  for (i = 0; i < MJTILE_MAX; i++) {
    if (!MJTILE_IS_YAOCHU(i)) {
      /* 么九牌以外が含まれていたら不成立 */
      if (hai[i] > 0) {
        return false;
      }
    } else if (hai[i] > 0) {
      /* 老頭牌/字牌の出現をマーク */
      if (MJTILE_IS_ROUTOU(i)) {
        routou = true;
      } else if (MJTILE_IS_JIHAI(i)) {
        jihai = true;
      }
    }
  }

  /* 老頭牌と字牌両方の出現を要求 */
  return (routou && jihai);
}

/* 混一色が成立しているか？ */
static bool MJScore_IsHonitsu(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i, num_types, type;
  const uint8_t *hai;
  bool jihai;

  assert((info != NULL) && (merged_hand != NULL));

  num_types = 0;
  hai = &(merged_hand->hand[0]);

  jihai = false;
  for (i = 0; i < MJTILE_MAX; i++) {
    /* 字牌の出現をマーク */
    if (MJTILE_IS_JIHAI(i) && (hai[i] > 0)) {
      jihai = true;
    }
    if (MJTILE_IS_SUHAI(i) && (hai[i] > 0)) {
      if (num_types == 0) {
        /* 最初の種類を記録 */
        num_types++;
        type = i / 10;
      }
      if (!MJTILE_TYPE_IS(i, type)) {
        /* 2種類以上ある */
        return false;
      }
    }
  }

  /* 字牌の出現を要求 */
  return jihai;
}

/* 清一色が成立しているか？ */
static bool MJScore_IsChinitsu(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i, num_types, type;
  const uint8_t *hai;

  assert((info != NULL) && (merged_hand != NULL));

  num_types = 0;
  hai = &(merged_hand->hand[0]);
  for (i = 0; i < MJTILE_MAX; i++) {
    /* 字牌は出現してはならない */
    if (MJTILE_IS_JIHAI(i) && (hai[i] > 0)) {
      return false;
    }
    if (MJTILE_IS_SUHAI(i) && (hai[i] > 0)) {
      if (num_types == 0) {
        /* 最初の種類を記録 */
        num_types++;
        type = i / 10;
      }
      if (!MJTILE_TYPE_IS(i, type)) {
        /* 2種類以上ある */
        return false;
      }
    }
  }

  return true;
}

/* 小三元が成立しているか？ */
static bool MJScore_IsSyosangen(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i, num_sangen_mentsu;
  bool sangen_atama;
  const uint8_t *hai;

  assert((info != NULL) && (merged_hand != NULL));

  /* 三元牌が2つ入っているか？ */
  num_sangen_mentsu = 0;
  sangen_atama = false;
  hai = &(merged_hand->hand[0]);
  for (i = 0; i < MJTILE_MAX; i++) {
    if (MJTILE_IS_SANGEN(i)) {
      if (hai[i] == 2) {
        sangen_atama = true;
      } else if (hai[i] >= 3) {
        num_sangen_mentsu++;
      }
    }
  }

  /* 三元牌による頭と面子2つを要請 */
  return sangen_atama && (num_sangen_mentsu == 2);
}

/* 三槓子が成立しているか？ */
static bool MJScore_IsSankantsu(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i, num_kan;

  assert((info != NULL) && (merged_hand != NULL));

  /* 3副露必須（簡易判定） */
  if (info->num_meld != 3) {
    return false;
  }

  /* カン数をカウント */
  num_kan = 0;
  for (i = 0; i < MJTILE_MAX; i++) {
    if (merged_hand->hand[i] == 4) {
      num_kan++;
    }
  }

  return (num_kan == 3) ? true : false;
}

/* 国士無双13面待ちが成立しているか？ */
static bool MJScore_IsKokushimusou13(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i;
  struct MJHand tmp;

  /* 13面待ち時の牌姿 */
  static const uint8_t pattern[13] = { 
    MJTILE_1MAN, MJTILE_9MAN, MJTILE_1PIN, MJTILE_9PIN, 
    MJTILE_1SOU, MJTILE_9SOU, MJTILE_TON,  MJTILE_NAN,
    MJTILE_SHA,  MJTILE_PEE,  MJTILE_HAKU, MJTILE_HATU,
    MJTILE_CHUN 
  };

  assert((info != NULL) && (merged_hand != NULL));

  /* 鳴いている */
  if (info->num_meld > 0) {
    return false;
  }

  /* 和了牌が么九牌か？ */
  if (!MJTILE_IS_YAOCHU(info->agarihai)) {
    return false;
  }

  /* 作業領域に保存 */
  tmp = (*merged_hand);

  /* 和了牌を除く */
  tmp.hand[info->agarihai]--;

  /* 和了牌がない状態で上記の牌を全て持っている必要がある */
  for (i = 0; i < 13; i++) {
    if (tmp.hand[pattern[i]] != 1) {
      return false;
    }
  }

  return true;
}

/* 手牌から九蓮宝燈が成立しているか？ */
static bool MJScore_IsChurenpoutonCore(const struct MJHand *hand)
{
  int32_t i, type, type_offset;
  const uint8_t *hai;

  assert(hand != NULL);

  hai = &(hand->hand[0]);

  /* 字牌が含まれていたら不成立 */
  for (i = MJTILE_TON; i <= MJTILE_CHUN; i++) {
    if (hai[i] > 0) {
      return false;
    }
  }

  /* 持っている牌の種類を特定 */
  for (i = 0; i <= MJTILE_9SOU; i++) {
    if (hai[i] > 0) {
      type = i / 10;
      break;
    }
  }
  assert(i != MJTILE_9SOU);

  /* 1, 9は3枚以上持つべし */
  type_offset = 10 * type;
  if ((hai[type_offset + 1] < 3) || (hai[type_offset + 9] < 3)) {
    return false;
  }
  /* 2-8は1枚以上持つべし */
  for (i = type_offset + 2; i <= type_offset + 8; i++) {
    if (hai[i] == 0) {
      return false;
    }
  }

  return true;
}

/* 九蓮宝燈が成立しているか？ */
static bool MJScore_IsChurenpouton(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  assert((info != NULL) && (merged_hand != NULL));

  /* 鳴いている */
  if (info->num_meld > 0) {
    return false;
  }

  /* 9面待ちの場合を弾く */
  if (MJTILE_IS_CHUNCHAN(info->agarihai)) {
    if (merged_hand->hand[info->agarihai] == 2) {
      return false;
    }
  } else if (MJTILE_IS_ROUTOU(info->agarihai)) {
    if (merged_hand->hand[info->agarihai] == 4) {
      return false;
    }
  }

  /* コア処理実行 */
  return MJScore_IsChurenpoutonCore(merged_hand);
}

/* 九蓮宝燈9面待ちが成立しているか？ */
static bool MJScore_IsChurenpouton9(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  struct MJHand tmp;

  assert((info != NULL) && (merged_hand != NULL));

  /* 鳴いている */
  if (info->num_meld > 0) {
    return false;
  }

  /* 作業領域に保存 */
  tmp = (*merged_hand);

  /* 和了牌を抜いてからチェック */
  tmp.hand[info->agarihai]--;
  return MJScore_IsChurenpoutonCore(&tmp);
}

/* 四暗刻が成立しているか？ */
static bool MJScore_IsSuanko(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i;
  int32_t num_anko, num_toitsu;
  const uint8_t *hai;

  assert((info != NULL) && (merged_hand != NULL));

  /* 鳴いている or ツモ和了りじゃない */
  if ((info->num_meld > 0) || !(info->tsumo)) {
    return false;
  }

  hai = &(merged_hand->hand[0]);
  
  /* 頭と和了牌は一致しない（単騎待ちの場合を排除） */
  if (hai[info->agarihai] == 2) {
    return false;
  }

  /* 暗刻と対子をカウント */
  num_anko = num_toitsu = 0;
  for (i = 0; i < MJTILE_MAX; i++) {
    switch (hai[i]) {
      case 3: num_anko++;  break;
      case 2: num_toitsu++; break;
      default: break;
    }
  }

  /* 暗刻数が4つ & 対子数が1つ */
  return ((num_anko == 4) && (num_toitsu == 1)) ? true : false;
}

/* 四暗刻単騎が成立しているか？ */
static bool MJScore_IsSuankoTanki(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i;
  int32_t num_anko;
  const uint8_t *hai;
  struct MJHand tmp;

  assert((info != NULL) && (merged_hand != NULL));

  /* 鳴いている */
  if (info->num_meld > 0) {
    return false;
  }

  /* 和了牌を抜く */
  tmp = (*merged_hand);
  tmp.hand[info->agarihai]--;

  hai = &(tmp.hand[0]);

  /* 暗刻をカウント */
  num_anko = 0;
  for (i = 0; i < MJTILE_MAX; i++) {
    switch (hai[i]) {
      case 0: break;
      case 1: 
        /* 抜いた牌のはず */
        if (i != info->agarihai) {
          return false;
        }
        break;
      case 3: num_anko++; break;
      default: 
        return false;
    }
  }

  /* 暗刻数が4つ */
  return (num_anko == 4) ? true : false;
}

/* 大四喜が成立しているか？ */
static bool MJScore_IsDaisushi(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  assert((info != NULL) && (merged_hand != NULL));

  /* 東南西北がすべて3枚以上あるか？ */
  return ((merged_hand->hand[MJTILE_TON] >= 3) && (merged_hand->hand[MJTILE_NAN] >= 3) 
      && (merged_hand->hand[MJTILE_SHA] >= 3) && (merged_hand->hand[MJTILE_PEE] >= 3)) ? true : false;
}

/* 小四喜が成立しているか？ */
static bool MJScore_IsSyosushi(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i, num_anko, num_toitsu;

  assert((info != NULL) && (merged_hand != NULL));

  num_anko = num_toitsu = 0;
  for (i = MJTILE_TON; i <= MJTILE_PEE; i++) {
    switch (merged_hand->hand[i]) {
      case 3: num_anko++; break;
      case 2: num_toitsu++; break;
    }
  }

  return ((num_anko == 3) && (num_toitsu == 1)) ? true : false;
}

/* 大三元が成立しているか？ */
static bool MJScore_IsDaisangen(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  assert((info != NULL) && (merged_hand != NULL));

  /* 白發中がすべて3枚以上あるか？ */
  return ((merged_hand->hand[MJTILE_HAKU] >= 3) && (merged_hand->hand[MJTILE_HATU] >= 3) 
      && (merged_hand->hand[MJTILE_CHUN] >= 3)) ? true : false;
}

/* 字一色が成立しているか？ */
static bool MJScore_IsTsuiso(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i;

  assert((info != NULL) && (merged_hand != NULL));

  /* 1枚でも字牌以外が含まれていたら不成立 */
  for (i = 0; i <= MJTILE_9SOU; i++) {
    if (merged_hand->hand[i] > 0) {
      return false;
    }
  }

  return true;
}

/* 清老頭が成立しているか？ */
static bool MJScore_IsChinroto(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i;

  assert((info != NULL) && (merged_hand != NULL));

  /* 1枚でも老頭牌以外が含まれていたら不成立 */
  for (i = 0; i < MJTILE_MAX; i++) {
    if (!MJTILE_IS_ROUTOU(i) && (merged_hand->hand[i] > 0)) {
      return false;
    }
  }

  return true;
}

/* 緑一色が成立しているか？ */
static bool MJScore_IsRyuiso(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i;

  assert((info != NULL) && (merged_hand != NULL));

  /* 1枚でも緑牌以外が含まれていたら不成立 */
  for (i = 0; i < MJTILE_MAX; i++) {
    if ((i == MJTILE_2SOU) || (i == MJTILE_3SOU)
        || (i == MJTILE_4SOU) || (i == MJTILE_6SOU)
        || (i == MJTILE_8SOU) || (i == MJTILE_HATU)) {
      continue;
    }
    if (merged_hand->hand[i] > 0) {
      return false;
    }
  }

  return true;
}

/* 四槓子が成立しているか？ */
static bool MJScore_IsSukantsu(const struct MJAgariInformation *info, const struct MJHand *merged_hand)
{
  int32_t i, num_kan;

  assert((info != NULL) && (merged_hand != NULL));

  /* 4副露必須（簡易判定） */
  if (info->num_meld != 4) {
    return false;
  }

  /* カン数をカウント */
  num_kan = 0;
  for (i = 0; i < MJTILE_MAX; i++) {
    if (merged_hand->hand[i] == 4) {
      num_kan++;
    }
  }

  return (num_kan == 4) ? true : false;
}
