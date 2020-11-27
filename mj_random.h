#ifndef MJRANDOM_H_INCLUDED
#define MJRANDOM_H_INCLUDED

#include <stdint.h>

/* 乱数生成インターフェース */
struct MJRandomGeneratorInterface {
  /* 乱数生成インスタンス作成に必要なワークサイズの計算 */
  int32_t (*CalculateWorkSize)(void);
  /* 乱数生成インスタンスの作成 失敗時はNULLを返す */
  void* (*Create)(void *work, int32_t work_size);
  /* 乱数生成インスタンスの破棄 */
  void (*Destroy)(void *random);
  /* 乱数シード値の設定 */
  void (*SetSeed)(void *random, const void *seed);
  /* min以上max以下の整数乱数を発生させる */
  int32_t (*GetRandom)(void *random, int32_t min, int32_t max);
};

#endif /* MJRANDOM_H_INCLUDED */
