#ifndef MJRANDOM_XOSHIRO256PP_H_INCLUDED
#define MJRANDOM_XOSHIRO256PP_H_INCLUDED

#include "mj_random.h"
#include <stdint.h>

/* シード値 */
struct MJRandomXoshiro256ppSeed {
  uint64_t seed[4]; /* シード配列（どれか1つの要素は0以外にセットしてください） */
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* XOshiro256++乱数生成器のインターフェースを取得 */
const struct MJRandomGeneratorInterface *MJRandomXoshiro256pp_GetInterface(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MJRANDOM_XOSHIRO256PP_H_INCLUDED */
