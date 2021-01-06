#ifndef MJUTILITY_H_INCLUDED
#define MJUTILITY_H_INCLUDED

/* 静的アサート */
#define MJUTILITY_STATIC_ASSERT(cond) void static_assetion_failed(char static_assetion_failed[(cond) ? 1 : -1])
/* 未使用引数警告回避 */
#define MJUTILITY_UNUSED_ARGUMENT(arg) { if (arg) { ; } }
/* nの倍数への切り上げ */
#define MJUTILITY_ROUND_UP(val, n)    ((((val) + ((n) - 1)) / (n)) * (n))
/* nの倍数への切り捨て */
#define MJUTILITY_ROUND_DOWN(val, n)  (((val) / (n)) * (n))
/* 2の冪乗数か判定 */
#define MJUTILITY_IS_POWERED_OF_2(val)  (!((val) & ((val) - 1)))

#endif /* MJUTILITY_H_INCLUDED */
