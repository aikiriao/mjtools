#ifndef MJMJTILE_H_INCLUDED
#define MJMJTILE_H_INCLUDED

/* 数牌の種類 */
#define MJTILETYPE_MANZU  0 /* 萬子 */
#define MJTILETYPE_PINZU  1 /* 筒子 */
#define MJTILETYPE_SOUZU  2 /* 索子 */

/* 牌の識別整数を定義 とつげき東北氏作のMJexeIO.DLLに準拠
 * 格納先の変数がenum(int)より小さい場合に備えてdefine */

/* 萬子 */
#define MJTILE_1MAN   1  /* 一萬 */
#define MJTILE_2MAN   2  /* 二萬 */
#define MJTILE_3MAN   3  /* 三萬 */
#define MJTILE_4MAN   4  /* 四萬 */
#define MJTILE_5MAN   5  /* 五萬 */
#define MJTILE_6MAN   6  /* 六萬 */
#define MJTILE_7MAN   7  /* 七萬 */
#define MJTILE_8MAN   8  /* 八萬 */
#define MJTILE_9MAN   9  /* 九萬 */
/* 筒子 */
#define MJTILE_1PIN  11  /* 一筒 */
#define MJTILE_2PIN  12  /* 二筒 */
#define MJTILE_3PIN  13  /* 三筒 */
#define MJTILE_4PIN  14  /* 四筒 */
#define MJTILE_5PIN  15  /* 五筒 */
#define MJTILE_6PIN  16  /* 六筒 */
#define MJTILE_7PIN  17  /* 七筒 */
#define MJTILE_8PIN  18  /* 八筒 */
#define MJTILE_9PIN  19  /* 九筒 */
/* 索子 */
#define MJTILE_1SOU  21  /* 一索 */
#define MJTILE_2SOU  22  /* 二索 */
#define MJTILE_3SOU  23  /* 三索 */
#define MJTILE_4SOU  24  /* 四索 */
#define MJTILE_5SOU  25  /* 五索 */
#define MJTILE_6SOU  26  /* 六索 */
#define MJTILE_7SOU  27  /* 七索 */
#define MJTILE_8SOU  28  /* 八索 */
#define MJTILE_9SOU  29  /* 九索 */
/* 字牌 */
#define MJTILE_TON   31  /* 東 */
#define MJTILE_NAN   32  /* 南 */
#define MJTILE_SHA   33  /* 西 */
#define MJTILE_PEE   34  /* 北 */
#define MJTILE_HAKU  35  /* 白 */
#define MJTILE_HATU  36  /* 發 */
#define MJTILE_CHUN  37  /* 中 */
/* 最大値 */
#define MJTILE_MAX   38

/* 牌判定マクロ */
/* 牌は字牌か？ */
#define MJTILE_IS_JIHAI(t)  (((t) >= MJTILE_TON) && ((t) <= MJTILE_CHUN))
/* 牌は数牌か？ */
#define MJTILE_IS_SUHAI(t)                            \
  (   (((t) >= MJTILE_1MAN) && ((t) <= MJTILE_9MAN))  \
   || (((t) >= MJTILE_1PIN) && ((t) <= MJTILE_9PIN))  \
   || (((t) >= MJTILE_1SOU) && ((t) <= MJTILE_9SOU)))
/* 牌は中張牌か？ */
#define MJTILE_IS_CHUNCHAN(t)                         \
  (   (((t) >= MJTILE_2MAN) && ((t) <= MJTILE_8MAN))  \
   || (((t) >= MJTILE_2PIN) && ((t) <= MJTILE_8PIN))  \
   || (((t) >= MJTILE_2SOU) && ((t) <= MJTILE_8SOU)))
/* 牌は老頭（一九）牌か？ */
#define MJTILE_IS_ROUTOU(t) (MJTILE_IS_SUHAI(t) && !MJTILE_IS_CHUNCHAN(t))
/* 牌は么九牌か？ */
#define MJTILE_IS_YAOCHU(t) (MJTILE_IS_ROUTOU(t) || MJTILE_IS_JIHAI(t))
/* 牌は三元牌か？ */
#define MJTILE_IS_SANGEN(t) (((t) == MJTILE_HAKU) || ((t) == MJTILE_HATU) || ((t) == MJTILE_CHUN))
/* 数牌の種類判定 */
#define MJTILE_TYPE_IS(t, type) (MJTILE_IS_SUHAI(t) && ((t) >= (10 * (type))) && ((t) <= (10 * ((type) + 1))))
/* 種類を無視した数牌判定 */
#define MJTILE_NUMBER_IS(t, number) (MJTILE_IS_SUHAI(t) && (((t) % 10) == (number)))
/* 2つの数牌が同じ数字か？ */
#define MJTILE_IS_SAME_NUMBER(t1, t2) (MJTILE_IS_SUHAI(t1) && MJTILE_IS_SUHAI(t2) && (((t1) % 10) == ((t2) % 10)))

#endif /* MJMJTILE_H_INCLUDED */
