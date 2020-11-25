#include "mj_random_xoshiro256pp.h"
#include "mj_utility.h"
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

/* http://prng.di.unimi.it/xoshiro256plusplus.c を参考 
 * パブリックドメインにしているので問題ない...はず */

/* アラインメント */
#define MJRANDOMXOSHRO256PP_ALIGNMENT 16

/* 乱数生成ハンドル */
struct MJRandomXoshiro256pp {
  uint64_t  s[4];           /* 内部状態 */
  bool      alloced_by_own; /* メモリは自前確保か？ */
  void      *work;          /* ワークメモリ先頭アドレス */
};

/* ワークサイズ計算 */
static int32_t MJRandomXoshiro256pp_CalculateWorksize(void);
/* インスタンス生成 */
static void* MJRandomXoshiro256pp_Create(void *work, int32_t work_size);
/* インスタンス破棄 */
static void MJRandomXoshiro256pp_Destroy(void *random);
/* シード値の設定 */
static void MJRandomXoshiro256pp_SetSeed(void *random, const void *seed);
/* 乱数生成 */
static uint32_t MJRandomXoshiro256pp_GetRandom(void *random, uint32_t min, uint32_t max);

/* インターフェース定義 */
static const struct MJRandomGeneratorInterface st_xoshiro128_interface = {
  MJRandomXoshiro256pp_CalculateWorksize,
  MJRandomXoshiro256pp_Create,
  MJRandomXoshiro256pp_Destroy,
  MJRandomXoshiro256pp_SetSeed,
  MJRandomXoshiro256pp_GetRandom,
};

/* インスタンス生成直後にセットするシード */
static const struct MJRandomXoshiro256ppSeed default_seed = {
  .seed = { 0xDEADBEAFDEADBEAFULL, 0xABADCAFEABADCAFEULL, 0xCDCDCDCDCDCDCDCDULL, 0xA5A5A5A5A5A5A5A5ULL }
};

/* インターフェース取得 */
const struct MJRandomGeneratorInterface *MJRandomXoshiro256pp_GetInterface(void)
{
  return &st_xoshiro128_interface;
}

/* ビットローテート */
static uint64_t MJRandomXoshiro256pp_RotateLeft(const uint64_t x, int32_t k)
{
  assert(k >= 0);
  return (x << k) | (x >> (64 - k));
}

/* 次の乱数値の取得 */
static uint64_t MJRandomXoshiro256pp_Next(struct MJRandomXoshiro256pp *random)
{
  uint64_t *s;
  uint64_t result, t;

  assert(random != NULL);

  s = &random->s[0];

  result = MJRandomXoshiro256pp_RotateLeft(s[0] + s[3], 23) + s[0];
  t = s[1] << 17;

  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];
  s[2] ^= t;
  s[3] = MJRandomXoshiro256pp_RotateLeft(s[3], 45);

  return result;
}

/* [0,max]の範囲の一様乱数を生成 */
static uint32_t MJRandomXoshiro256pp_UniformRand(struct MJRandomXoshiro256pp *random, uint32_t max)
{
  uint64_t rnd;
  double uniform;

  /* 64bit一様乱数の生成 */
  rnd = MJRandomXoshiro256pp_Next(random);

  /* doubleの小数部が53bitであることを用いて[0..1)の範囲に直す */
  /* 参考: http://prng.di.unimi.it "Generating uniform doubles in the unit interval" */
  uniform = (rnd >> 11) * 0x1.0p-53;

  return (uint32_t)(uniform * ((double)max + 1));
}

/* ワークサイズ計算 */
static int32_t MJRandomXoshiro256pp_CalculateWorksize(void)
{
  return sizeof(struct MJRandomXoshiro256pp) + MJRANDOMXOSHRO256PP_ALIGNMENT;
}

/* インスタンス生成 */
static void* MJRandomXoshiro256pp_Create(void *work, int32_t work_size)
{
  struct MJRandomXoshiro256pp *random;
  uint8_t *work_ptr;
  bool tmp_alloced_by_own = false;

  /* 領域自前確保の場合 */
  if ((work == NULL) && (work_size == 0)) {
    if ((work_size = MJRandomXoshiro256pp_CalculateWorksize()) < 0) {
      return NULL;
    }
    work = malloc((size_t)work_size);
    tmp_alloced_by_own = true;
  }

  /* 引数チェック */
  if ((work == NULL) || (work_size < MJRandomXoshiro256pp_CalculateWorksize())) {
    return NULL;
  }

  /* アドレスのアラインメント調整 */
  work_ptr = (uint8_t *)work;
  work_ptr = (uint8_t *)MJUTILITY_ROUND_UP((uintptr_t)work_ptr, MJRANDOMXOSHRO256PP_ALIGNMENT);

  /* インスタンス領域の確保 */
  random = (struct MJRandomXoshiro256pp *)work_ptr;
  /* メモリ先頭アドレスを記録 */
  random->work = work;
  /* 自前確保フラグを記録 */
  random->alloced_by_own = tmp_alloced_by_own;

  work_ptr += sizeof(struct MJRandomXoshiro256pp);

  /* シード値をデフォルト値にセット */
  MJRandomXoshiro256pp_SetSeed(random, &default_seed);

  return random;
}

/* インスタンス破棄 */
static void MJRandomXoshiro256pp_Destroy(void *random)
{
  struct MJRandomXoshiro256pp *xor = (struct MJRandomXoshiro256pp *)random;

  if (xor != NULL) {
    /* 自分で領域確保していたら解放 */
    if (xor->alloced_by_own) {
      free(xor->work);
    }
  }
}

/* シード値の設定 */
static void MJRandomXoshiro256pp_SetSeed(void *random, const void *seed)
{
  struct MJRandomXoshiro256pp *xor = (struct MJRandomXoshiro256pp *)random;
  const struct MJRandomXoshiro256ppSeed *xor_seed = (const struct MJRandomXoshiro256ppSeed *)seed;

  assert(random != NULL);
  assert(seed != NULL);

  /* オール0が現れたらプログラミングミスとして落とす */
  assert(!((xor_seed->seed[0] == 0) && (xor_seed->seed[1] == 0)
        && (xor_seed->seed[2] == 0) && (xor_seed->seed[3] == 0)));

  /* シードをセット */
  xor->s[0] = xor_seed->seed[0];
  xor->s[1] = xor_seed->seed[1];
  xor->s[2] = xor_seed->seed[2];
  xor->s[3] = xor_seed->seed[3];
}

/* 乱数生成 */
static uint32_t MJRandomXoshiro256pp_GetRandom(void *random, uint32_t min, uint32_t max)
{
  uint32_t result;
  struct MJRandomXoshiro256pp *xor = (struct MJRandomXoshiro256pp *)random;

  assert(random != NULL);
  assert(max > min);

  /* [0,max-min] の一様乱数を生成 */
  result = MJRandomXoshiro256pp_UniformRand(xor, max - min);

  /* minを加えて[min,max]の乱数を得る */
  return result + min;
}
