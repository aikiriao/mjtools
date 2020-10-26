#include <stdio.h> 
#include <stdint.h>
#include <stdlib.h>

#include "../mj_shanten_use_table.c"

/* 出力ファイル名 */
#define MJSHANTEN_TABLE_FILE_NAME   "mj_shanten_table.c"

/* 空エントリ */
static const struct ShantenTableEntry empty = { 5 /* 登場し得ない牌状態 */, 0, 0 };

int main(void)
{
  FILE *fp;
  uint32_t i, num_entry, num_collusions;
  char linebuf[256];
  struct ShantenTableEntry *table;

  /* 向聴数データファイルオープン */
  fp = fopen("shanten.dat", "r");

  /* テーブル作成 */
  table = malloc(sizeof(struct ShantenTableEntry) * MJSHANTEN_TABLE_SIZE);
  for (i = 0; i < MJSHANTEN_TABLE_SIZE; i++) {
    table[i] = empty;
  }

  /* データファイルの行ごとにエントリ登録 */
  num_entry = num_collusions = 0;
  while (fgets(linebuf, sizeof(linebuf), fp) != NULL) {
    uint32_t value, key;
    int num_mentsu_A, num_tatsu_A, num_mentsu_B, num_tatsu_B;
    int num_mentsu, num_tatsu;

    /* 1行パース */
    sscanf(linebuf, "%09d %d %d %d %d",
        &value, &num_mentsu_A, &num_tatsu_A, &num_mentsu_B, &num_tatsu_B);

    /* 向聴数の比較・面子数と塔子数の確定 */
    if ((2 * num_mentsu_A + num_tatsu_A) >= (2 * num_mentsu_B + num_tatsu_B)) {
      num_mentsu = num_mentsu_A; num_tatsu = num_tatsu_A;
    } else {
      num_mentsu = num_mentsu_B; num_tatsu = num_tatsu_B;
    }

    /* 開番地法: 空きエントリ探索 */
    key = MJSHANTEN_MAKE_HASH(value) & (MJSHANTEN_TABLE_SIZE - 1);
    while (table[key].value != empty.value) {
      num_collusions++;
      key = (key + 1) & (MJSHANTEN_TABLE_SIZE - 1);
    }

    /* エントリ登録 */
    table[key].value = value;
    table[key].num_mentsu = num_mentsu;
    table[key].num_tatsu = num_tatsu;

    num_entry++;
  }

  /* 平均衝突回数 */
  printf("Avg. Collusion: %f \n", (double)num_collusions / num_entry);

  fclose(fp);

  /* 書き出し */
  fp = fopen(MJSHANTEN_TABLE_FILE_NAME, "w");
  for (i = 0; i < MJSHANTEN_TABLE_SIZE; i++) {
    const struct ShantenTableEntry *ptable = &table[i];
    fprintf(fp, "{%d,%d,%d},", ptable->value, ptable->num_mentsu, ptable->num_tatsu);
  }
  /* 改行してEOFとしないと警告が発生するため */
  fprintf(fp, "\n");
  fclose(fp);

  free(table);

  return 0;
}
