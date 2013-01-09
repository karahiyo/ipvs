#include "includes.h"

/*===============================================================
  routine    : int loadRawFloat
  return val : int
  input  par : char *filename
               float *rawmap
  function   : load function of raw image (float map)
 ===============================================================*/
int loadRawFloat(char *filename, float *rawmap)
{
  FILE *fp;
  unsigned char tmpmap[SRC_Y_SIZE*SRC_X_SIZE];
  register int i;
  
  /* ファイルのオープン */
  if ((fp = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "can't open raw image file \"%s\"...\n", filename);
    exit(1);
  }
  /* 画像データの読み込み */
  if (fread(tmpmap, sizeof(char), SRC_Y_SIZE*SRC_X_SIZE, fp) != 
      SRC_Y_SIZE*SRC_X_SIZE) {
    fprintf(stderr, "can't read raw image data...\n");
    exit(1);
  }
  /* ファイルのクローズ */
  fclose(fp);
  /* unsigned char から float に変換 */
  for (i = 0; i < SRC_Y_SIZE*SRC_X_SIZE; i++)
    rawmap[i] = tmpmap[i];
  
  return 0;
}

/*===============================================================
  routine    : int savePgmFloat
  return val : int
  input  par : char *filename
               float *map
  function   : save function of PGM foramt (float map)
 ===============================================================*/
int savePgmFloat(char *filename, float *map)
{
  FILE *fp;
  unsigned char tmpmap[SRC_Y_SIZE*SRC_X_SIZE];
  register int i;

  /* float から unsigned char に変換 */
  for (i = 0; i < SRC_Y_SIZE*SRC_X_SIZE; i++) {
    if (map[i] > 255) tmpmap[i] = 255;  /* 上限リミッタ */
    else if (map[i] < 0) tmpmap[i] = 0; /* 下限リミッタ */
    else tmpmap[i] = ROUND(map[i]);     /* 四捨五入     */
  }
  /* ファイルのオープン */
  if ((fp = fopen(filename, "w")) == NULL) {
    fprintf(stderr, "can't open PGM format file \"%s\"...\n", filename);
    exit(1);
  }
  /* ヘッダの書き込み */
  fprintf(fp, "P5\n%d %d\n255\n", SRC_X_SIZE, SRC_Y_SIZE);
  /* 画像データの書き込み */
  if (fwrite(tmpmap, sizeof(char), SRC_Y_SIZE*SRC_X_SIZE, fp) !=
      SRC_Y_SIZE*SRC_X_SIZE) {
    fprintf(stderr, "can't write PGM format file...\n");
    exit(1);
  }
  /* ファイルのクローズ */
  fclose(fp);

  return 0;
}


/*===============================================================
  routine    : makeFileName
  return val : int
  input  par : char *filename
               char *basename
               int num
               char *magic
  function   : ファイル名の作成
 ===============================================================*/
int makeFileName(char *filename, char *basename, int num, char *magic)
{

  if (0 <= num && num < 10)
    sprintf(filename, "%s00%d%s", basename, num, magic);
  else if (num < 100)
    sprintf(filename, "%s0%d%s", basename, num, magic);
  else if (num < 1000)
    sprintf(filename, "%s%d%s", basename, num, magic);

  return 0;
}
