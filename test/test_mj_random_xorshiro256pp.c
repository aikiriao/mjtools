#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "test.h"

/* テスト対象のモジュール */
#include "../mj_random_xoshiro256pp.c"

/* テストのセットアップ関数 */
void MJRandomXoshiro256ppTest_Setup(void);

static int MJRandomXoshiro256ppTest_Initialize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

static int MJRandomXoshiro256ppTest_Finalize(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);
  return 0;
}

/* 生成破棄テスト */
static void MJRandomXoshiro256ppTest_CreateDestroyTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* ワーク領域渡しによる生成・破棄 */
  {
    int32_t work_size;
    void *work;
    struct MJRandomXoshiro256pp *random = NULL;

    work_size = MJRandomXoshiro256pp_CalculateWorksize();
    Test_AssertCondition(work_size > 0);

    work = malloc(work_size);
    random = MJRandomXoshiro256pp_Create(work, work_size);
    Test_AssertCondition(random != NULL);
    Test_AssertEqual(random->alloced_by_own, false);
    Test_AssertCondition(random->work == work);

    MJRandomXoshiro256pp_Destroy(random);
    free(work);
  }

  /* ワーク領域自前確保による生成・破棄 */
  {
    struct MJRandomXoshiro256pp *random = NULL;

    random = MJRandomXoshiro256pp_Create(NULL, 0);
    Test_AssertCondition(random != NULL);
    Test_AssertEqual(random->alloced_by_own, true);
    Test_AssertCondition(random->work != NULL);

    MJRandomXoshiro256pp_Destroy(random);
  }
}

/* 簡易エントロピー確認テスト */
static void MJRandomXoshiro256ppTest_EntropyTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);

  /* 何度か繰り返してエントロピーが高いか簡単に確かめる */
  {
#define NUM_SAMPLES (2048)
    double entropy;
    int32_t i, ite, is_ok;
    int32_t count[256];
    struct MJRandomXoshiro256pp *random;

    random = MJRandomXoshiro256pp_Create(NULL, 0);

    is_ok = 1;
    for (ite = 0; ite < 20; ite++) {
      /* カウントをリセット */
      memset(&count[0], 0, sizeof(count));

      /* 乱数生成 */
      for (i = 0; i < NUM_SAMPLES; i++) {
        int32_t rnd = MJRandomXoshiro256pp_GetRandom(random, 0, 255);
        if (rnd > 255) {
          printf("generated number(%d) is outer range at %d \n", rnd, ite);
          is_ok = 0;
          goto TEST_EXIT;
        }
        count[rnd]++;
      }

      /* エントロピー計測 */
      entropy = 0.0f;
      for (i = 0; i < 256; i++) {
        double prob = (double)count[i] / NUM_SAMPLES;
        if (count[i] > 0) {
          entropy -= prob * log2(prob);
        }
      }

      if (entropy <= 7.8) {
        printf("low entropy at %d: %f \n", ite, entropy);
        is_ok = 0;
        goto TEST_EXIT;
      }
    }

TEST_EXIT:
    Test_AssertEqual(is_ok, 1);
    MJRandomXoshiro256pp_Destroy(random);

#undef NUM_SAMPLES 
  }
}

/* シード値設定テスト */
static void MJRandomXoshiro256ppTest_SetSeedTest(void *obj)
{
  TEST_UNUSED_PARAMETER(obj);


  /* 同じシード値から生成した乱数列が同じになるかチェック */
  {
#define NUM_SEQUENCES 100
#define NUM_SAMPLES 100
#define RANDMIN 1e2
#define RANDMAX 1e8
    int32_t is_ok, seq, smpl;
    struct MJRandomXoshiro256pp *random;
    struct MJRandomXoshiro256ppSeed seed;
    int32_t rand_seq[NUM_SAMPLES];

    random = MJRandomXoshiro256pp_Create(NULL, 0);

    is_ok = 1;
    for (seq = 0; seq < NUM_SEQUENCES; seq++) {
      /* 基準となる乱数列を生成 */
      seed.seed[0] = seed.seed[1] = seed.seed[2] = seed.seed[3] = ~seq;
      MJRandomXoshiro256pp_SetSeed(random, &seed);
      for (smpl = 0; smpl < NUM_SAMPLES; smpl++) {
        int32_t rnd = MJRandomXoshiro256pp_GetRandom(random, RANDMIN, RANDMAX);
        if ((rnd < RANDMIN) || (rnd > RANDMAX)) {
          is_ok = 0;
          printf("generated number(%d) is outer range at %d,%d \n", rnd, seq, smpl);
          goto TEST_EXIT1;
        }
        rand_seq[smpl] = rnd;
      }

      /* 同じシードを再度セット */
      seed.seed[0] = seed.seed[1] = seed.seed[2] = seed.seed[3] = ~seq;
      MJRandomXoshiro256pp_SetSeed(random, &seed);
      /* 同じ列が再現するか？ */
      for (smpl = 0; smpl < NUM_SAMPLES; smpl++) {
        int32_t rnd = MJRandomXoshiro256pp_GetRandom(random, RANDMIN, RANDMAX);
        if ((rnd < RANDMIN) || (rnd > RANDMAX)) {
          is_ok = 0;
          printf("generated number(%d) is outer range at %d,%d \n", rnd, seq, smpl);
          goto TEST_EXIT1;
        }
        if (rand_seq[smpl] != rnd) {
          is_ok = 0;
          printf("random number reproducibility broken: seed:%d sample:%d \n", seq, smpl);
          goto TEST_EXIT1;
        }
      }
    }

TEST_EXIT1:
    Test_AssertEqual(is_ok, 1);
    MJRandomXoshiro256pp_Destroy(random);
#undef RANDMIN
#undef RANDMAX
#undef NUM_SEQUENCES
#undef NUM_SAMPLES
  }

  /* 異なるシード値から生成した乱数列が異なるかチェック */
  {
#define NUM_SEQUENCES 100
#define NUM_SAMPLES 10
#define RANDMIN 5
#define RANDMAX 10
    int32_t is_ok, seq, seq2, smpl;
    struct MJRandomXoshiro256pp *random;
    struct MJRandomXoshiro256ppSeed seed;
    int32_t rand_seq[NUM_SEQUENCES][NUM_SAMPLES];

    random = MJRandomXoshiro256pp_Create(NULL, 0);

    is_ok = 1;
    for (seq = 0; seq < NUM_SEQUENCES; seq++) {
      /* シード値をセット */
      seed.seed[0] = seed.seed[1] = seed.seed[2] = seed.seed[3] = ~seq;
      MJRandomXoshiro256pp_SetSeed(random, &seed);
      for (smpl = 0; smpl < NUM_SAMPLES; smpl++) {
        int32_t rnd = MJRandomXoshiro256pp_GetRandom(random, RANDMIN, RANDMAX);
        if ((rnd < RANDMIN) || (rnd > RANDMAX)) {
          is_ok = 0;
          printf("generated number(%d) is outer range at %d,%d \n", rnd, seq, smpl);
          goto TEST_EXIT2;
        }
        rand_seq[seq][smpl] = rnd;
      }
    }

    /* 同じシーケンスが現れないことを期待 */
    for (seq = 0; seq < NUM_SEQUENCES; seq++) {
      for (seq2 = (seq + 1); seq2 < NUM_SEQUENCES; seq2++) {
        if (memcmp(&rand_seq[seq], &rand_seq[seq2], sizeof(int32_t) * NUM_SAMPLES) == 0) {
          is_ok = 0;
          printf("seq %d and seq %d are same. \n", seq, seq2);
          goto TEST_EXIT2;
        }
      }
    }

TEST_EXIT2:
    Test_AssertEqual(is_ok, 1);
    MJRandomXoshiro256pp_Destroy(random);
#undef RANDMIN
#undef RANDMAX
#undef NUM_SEQUENCES
#undef NUM_SAMPLES
  }
}

void MJRandomXoshiro256ppTest_Setup(void)
{
  struct TestSuite *suite
    = Test_AddTestSuite("MJ Random Xoshiro256++ Test Suite",
        NULL, MJRandomXoshiro256ppTest_Initialize, MJRandomXoshiro256ppTest_Finalize);

  Test_AddTest(suite, MJRandomXoshiro256ppTest_CreateDestroyTest);
  Test_AddTest(suite, MJRandomXoshiro256ppTest_EntropyTest);
  Test_AddTest(suite, MJRandomXoshiro256ppTest_SetSeedTest);
}
