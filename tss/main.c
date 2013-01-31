#include "includes.h"

/* global variable */
static float premap[SRC_Y_SIZE*SRC_X_SIZE];
static float crtmap[SRC_Y_SIZE*SRC_X_SIZE];
static float mcmap[SRC_Y_SIZE*SRC_X_SIZE];

static float vecx[MB_Y_NUM*MB_X_NUM];    /* motion vector (horizontal) */
static float vecy[MB_Y_NUM*MB_X_NUM];    /* motion vector (vertical)   */


/*===============================================================
  routine    : main
  return val : int
  input  par : int argc
               char *argv[]
  function   : main function
 ===============================================================*/
int main(int argc, char *argv[])
{
  double psnr;
  int headframe, tailframe, skipframe;
  char *srcname, c, *dd, filename[255];
  register int i, j;

  clock_t start, end;

  fprintf(stderr, "\nstarting program...\n");
  system("date");
  start = clock();

  /* パラメータの初期化 */
  srcname   = SRC_NAME;
  headframe = HEAD_FRAME;
  tailframe = TAIL_FRAME;
  skipframe = SKIP_FRAME;

  /* パラメータの設定 */
  if (argc < 1) {
    fprintf(stderr, "usage : %s [option]\n", argv[0]);
    fprintf(stderr, "  -s <string>  source image filename\n");
    fprintf(stderr, "  -h           help\n");
    exit(1);
  }  
  while (--argc > 0) {
    c = (*++argv)[0];
    if (!(c == '-')) fprintf(stderr, "\nunknown option...\n");
    else {
      c = (*argv)[1];  dd = &(*argv)[1];
      switch (c) {
      case 'h':
        fprintf(stderr, "\nusage : command [option]\n");
        fprintf(stderr, "  -s <string>  source image filename\n");
        fprintf(stderr, "  -h           help\n");
        exit(1);
        break;
      case 's':
        if (--argc > 0) {
          c = (*++argv)[0];
          if (c == '-') fprintf(stderr, "\noption error...\n");
          else srcname = (*argv);
        } else {
          fprintf(stderr, "\noption error \"%s\"...\n", (*argv));
          exit(1);
        }
        break;
      default:
        fprintf(stderr, "\nunknown option...\n");
        break;
      }
    }
  }
  fprintf(stderr, "image size = %dx%d pixel\n", SRC_X_SIZE, SRC_Y_SIZE);
  fprintf(stderr, "macro block size = %dx%d pixel\n", MB_SIZE, MB_SIZE);
  fprintf(stderr, "search window  x = [-%d:+%d], y = [-%d:+%d]\n",
          SW_SIZE, SW_SIZE, SW_SIZE, SW_SIZE);
  fprintf(stderr, "head frame number = %d, tail frame number = %d (skip:%d)\n",
          headframe, tailframe, skipframe);

  /* 先頭フレーム名の作成 */
  makeFileName(filename, srcname, headframe, ".cif");
  fprintf(stderr, "\n[frame no.%d]\n", headframe);
  fprintf(stderr, "now loading frame data from \"%s\"...\n", filename);
  /* 先頭フレームの読み込み */
  loadRawFloat(filename, premap);

  for (j = headframe + skipframe; j <= tailframe; j += skipframe) {  
    /* 動きベクトルの初期化 */
    for (i = 0; i < MB_X_NUM*MB_Y_NUM; i++) 
      vecx[i] = vecy[i] = 0.0;
    /* フレーム番号情報の表示 */
    fprintf(stderr, "\n[frame no.%d]-[frame no.%d]\n", j - skipframe, j);
    /* 次フレーム名の作成 */
    makeFileName(filename, srcname, j, ".cif");
    fprintf(stderr, "now loading frame data from \"%s\"...\n", filename);
    /* 次フレームの読み込み */
    loadRawFloat(filename, crtmap);
    /* Three Step Search */
    threeStepSearch(premap, crtmap, vecy, vecx);
    /* 予測画像の作成 */
    makePrediction(premap, mcmap, vecy, vecx);
    /* PSNR の算出 */
    psnr = getPsnrFloat(crtmap, mcmap);
    fprintf(stderr, "psnr = %f [dB]\n", psnr);
    /* フレームの複写 */
    for (i = 0; i < SRC_Y_SIZE*SRC_X_SIZE; i++)
      premap[i] = crtmap[i];
  }

  fprintf(stderr, "\ndone!!\n");
  system("date");
  end = clock();

  fprintf(stderr, "TIME=%d[ms]\n",end-start);
  return 0;
}
