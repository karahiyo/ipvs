#include "includes.h"

int bmcount;
int bmc_ave;
extern int bmcount;
extern int bmc_ave;
    
/*===============================================================
  routine    : threeStepSearch
  return val : int
  input  par : float *premap
               float *crtmap
	       float *vecy
	       float *vecx
  function   : full search
 ===============================================================*/
int diamondSearch(float *premap, float *crtmap, float *vecy, float *vecx)
{
  double sum, min;
  register int i, j, m, n, x, y, xx, yy;

  struct _diamond_struc {
      int y;
      int x;
  };

  /* LDSP */
  struct _diamond_struc ldsp[9] = 
            {{0,0},{-2,0},{-1,1},{0,2},{1,1},{2,0},{1,-1},{0,-2},{-1,-1}};

  /* SDSP */
  struct _diamond_struc sdsp[5] =
            {{0,0},{-1,0},{0,1},{1,0},{0,-1}};

  /* 探索範囲 */
  for (y = 0, yy = 0; y < SRC_Y_SIZE; y += MB_SIZE, yy++)
    for (x = 0, xx = 0; x < SRC_X_SIZE; x += MB_SIZE, xx++) {

      static int now_x, now_y; // 各ステップごとの開始点を記憶
      double round_min; // diamondサーチの一回の試行の中での最小値の値
      int yet[MB_SIZE][MB_SIZE]; // マクロブロックサイズの配列。各画素位置が探索済かどうかを保持
      int ldspGreed; // ldsp探索終了マーク
      int sdspGreed; // sdsp探索終了マーク
          
      min = DBL_MAX;
      now_x = 0, now_y = 0; // 探索開始点(0,0)
      ldspGreed = 1;
      int isblind = 0; // 探索空間が行き止まりかフラグ。0なら行き止まり

      /* マクロブロックサイズの配列をゼロ(=未探索)で初期化 */
      for(n = -SW_SIZE; n <= SW_SIZE; n++)
        for(m = -SW_SIZE; m <= SW_SIZE; m++){
           yet[n + SW_SIZE][m + SW_SIZE] = 0; // 配列の添字が負の値にならないようにSW_SIZEを加算
      }

      
      while(ldspGreed != 0) {          
          round_min = DBL_MAX;
          
          /* LDSP探索 */
          for (n = 0; n < 9; n++) {
              /* 画像端部の例外処理 */
                if((y + now_y + ldsp[n].y < 0) || 
                    (x + now_x + ldsp[n].x < 0) ||
                    (SRC_Y_SIZE < y + now_y + ldsp[n].y + MB_SIZE) ||
                    (SRC_X_SIZE < x + now_x + ldsp[n].x + MB_SIZE)) continue;

                if((now_y + ldsp[n].y < -7) ||
                    (now_x + ldsp[n].x < -7) ||
                    (now_y + ldsp[n].y > 7) ||
                    (now_x + ldsp[n].x > 7)) continue;

                /* 探索済の点か判定 */
                if (yet[now_y + ldsp[n].y + SW_SIZE][now_x + ldsp[n].x + SW_SIZE]==1)
                    continue;
                else
                    yet[now_y + ldsp[n].y + SW_SIZE][now_x + ldsp[n].x + SW_SIZE] = 1;

                /* 誤差の計算 */
	            sum = 0.0;
                bmcount++; //ブロックマッチング回数カウント
                isblind = 1; // 行き止まりではないフラグ ON
                for (j = 0; j < MB_SIZE; j++)
                  for (i = 0; i < MB_SIZE; i++) {
                      sum += fabs(crtmap[(y + j)*SRC_X_SIZE + (x + i)] -
			            premap[(y + now_y + ldsp[n].y + j)*SRC_X_SIZE + (x + now_x + ldsp[n].x + i)]);
                }

	            /* 一試行の最小値(round_min)と動きベクトルの更新 */
                if (sum < round_min) {
                    round_min = sum;
                    vecy[yy*MB_X_NUM + xx] = ldsp[n].y;
                    vecx[yy*MB_X_NUM + xx] = ldsp[n].x;
                }
            } /* }}} for(n)*for(m) */

           /* 探索開始点の移動 */
           if(isblind == 1) {
                now_y += vecy[yy*MB_X_NUM + xx];
                now_x += vecx[yy*MB_X_NUM + xx];
           } else {
                vecy[yy*MB_X_NUM + xx] = now_y;
                vecx[yy*MB_X_NUM + xx] = now_x;
                ldspGreed = 0;
           }

            /* 終了条件の判定 */
            if (round_min < min )
                min = round_min;
            else {
                vecy[yy*MB_X_NUM + xx] = now_y;
                vecx[yy*MB_X_NUM + xx] = now_x;
                ldspGreed = 0;
            }
    } /* }}} while(ldspGreed) */
    
  
  int tmp_vecy=0,tmp_vecx=0;
  /* SDSP探索 */
  for(n=0; n<6; n++) {
    /* 画像端部の例外処理 */
    if((y + now_y + sdsp[n].y < 0) || 
        (x + now_x + sdsp[n].x < 0) ||
        (SRC_Y_SIZE < y + now_y + sdsp[n].y + MB_SIZE) ||
        (SRC_X_SIZE < x + now_x + sdsp[n].x + MB_SIZE)) continue;

    if((now_y + sdsp[n].y < -7) ||
        (now_x + sdsp[n].x < -7) ||
        (now_y + sdsp[n].y > 7) ||
        (now_x + sdsp[n].x > 7)) continue;

    /* 誤差の計算 */
    sum = 0.0;
    bmcount++; //ブロックマッチング回数カウント
    for (j = 0; j < MB_SIZE; j++)
        for (i = 0; i < MB_SIZE; i++) {
            sum += fabs(crtmap[(y + j)*SRC_X_SIZE + (x + i)] -
                premap[(y + now_y + sdsp[n].y + j)*SRC_X_SIZE + (x + now_x + sdsp[n].x + i)]);
        }

    /* 一試行の最小値(round_min)と動きベクトルの更新 */
    if (sum < min) {
        min = sum;
        tmp_vecy = sdsp[n].y;
        tmp_vecx = sdsp[n].x;
    }
  } /* }}} for(sdsp) */

    /* 探索開始点の移動 */
    vecy[yy*MB_X_NUM + xx] += tmp_vecy;
    vecx[yy*MB_X_NUM + xx] += tmp_vecx;

  } /* }}} for(SRC_Y_SIZE)*for(SRC_X_SIZE) */
  fprintf(stderr,"min=%f \n",min);
  return 0;
}

/*===============================================================
  routine    : makePrediction
  return val : int
  input  par : float *premap
               float *mcmap
               float *vecy
               float *vecx
  function   : prediction image making
 ===============================================================*/
int makePrediction(float *premap, float *mcmap, float *vecy, float *vecx)
{
  int vx, vy;
  register int i, j, x, y, xx, yy;

  /* 予測画像の作成 */
  for (y = 0, yy = 0; y < SRC_Y_SIZE; y += MB_SIZE, yy++)
    for (x = 0, xx = 0; x < SRC_X_SIZE; x += MB_SIZE, xx++) {
      vy = vecy[yy*MB_X_NUM + xx];
      vx = vecx[yy*MB_X_NUM + xx];
      for (j = 0; j < MB_SIZE; j++)
        for (i = 0; i < MB_SIZE; i++) {
          mcmap[(y + j)*SRC_X_SIZE + (x + i)] = 
            premap[(y + vy + j)*SRC_X_SIZE + (x + vx + i)];
        }
    }

  return 0;
}

/*===============================================================
  routine    : getPsnrFloat
  return val : double
  input  par : float *srcmap
               float *decmap
  function   : calculation of PSNR
 ===============================================================*/
double getPsnrFloat(float *srcmap, float *decmap)
{
  double psnr, rms;
  register int i;

  /* PSNR の計算 */
  rms = 0.;
  for (i = 0; i < SRC_Y_SIZE*SRC_X_SIZE; i++)
      rms += (decmap[i] - srcmap[i])*(decmap[i] - srcmap[i]);
  rms /= SRC_Y_SIZE*SRC_X_SIZE;
  fprintf(stderr,"(1)rms=%f\n",rms); 
  rms = sqrt(rms);
  fprintf(stderr,"(2)rms=%f\n",rms);
  if (rms == 0.) {
    fprintf(stderr, "PSNR can't calclate...\n");
    exit(1);
  }
  psnr = 20.*log10(255./rms);

  return psnr;
}



