#include "includes.h"

int bmcount; // ブロックのマッチング回数を数える
int bmc_ave; // ブロックのマッチング回数の平均値
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
int greedySearch(float *premap, float *crtmap, float *vecy, float *vecx)
{
  double sum, min;
  register int i, j, m, n, x, y, xx, yy;

  /* 探索範囲 */
  for (y = 0, yy = 0; y < SRC_Y_SIZE; y += MB_SIZE, yy++)
    for (x = 0, xx = 0; x < SRC_X_SIZE; x += MB_SIZE, xx++) {
      
        min = DBL_MAX;
        double round_min = DBL_MAX; // greedyサーチの一回の試行の中での最小値の値
        int now_x,now_y; // 各ステップごとの開始点を記憶
        now_x = 0, now_y = 0; // 探索開始点(0,0)
        int yet[MB_SIZE][MB_SIZE]; // 探索空間のサイズの配列要素を持ち、既に探索を行った点を保持する
        int isGreed = 1; // 満足ならゼロ

        /* ゼロ(=未探索)で初期化 */
        for(n = -SW_SIZE; n <= SW_SIZE; n++)
          for(m = -SW_SIZE; m <= SW_SIZE; m++){
            yet[n + SW_SIZE][m + SW_SIZE] = 0; // 配列の添字が負の値にならないようにSW_SIZEを加算
          }
      
        while(isGreed) {
            /* 探索開始点のまわりを探索 */
            for (n = -1; n <= 1; n++) 
              for (m = -1; m <= 1; m++) {
                /* 画像端部の例外処理 */
                if((y + now_y + n < 0) || 
                    (x + now_x + m < 0) ||
                    (SRC_Y_SIZE < y + now_y + n) ||
                    (SRC_X_SIZE < x + now_x + m)) continue;

                /* 探索済ならマッチングしない */
                if (yet[now_y + n + SW_SIZE][now_x + m + SW_SIZE] == 1) 
                    continue;
                else
                    yet[now_y + n + SW_SIZE][now_x + m + SW_SIZE] = 1;
                    
                /* 誤差の計算 */
	            sum = 0.0;
                bmcount++; //ブロックマッチング回数カウント
                for (j = 0; j < MB_SIZE; j++)
                  for (i = 0; i < MB_SIZE; i++) {
                      //fprintf(stderr,"n,m=%d,%d",n,m);
                      sum += fabs(crtmap[(y + j)*SRC_X_SIZE + (x + i)] -
			            premap[(y + now_y + n + j)*SRC_X_SIZE + (x + now_x + m + i)]);
                }

	            /* 動きベクトルの更新 */
                if (sum < min) {
                    /*
                    if ( min > 1000000 )
                        fprintf(stderr,"sum=%f < min=***\n",sum);
                    else
                        fprintf(stderr,"sum=%f < min=%f\n",sum,min);
                    */
                    min = sum;
                    now_y = now_y + n;
                    now_x = now_x + m;
                }  
            } /* }}} for(n)*for(m) */
         
          /* ブロックマッチング回数の平均計算*/
          if(bmc_ave != 0) 
              bmc_ave = (bmc_ave + bmcount)/2;
          else
              bmc_ave = bmcount;
 
          if( round_min < min)
            round_min = min;
          else 
            isGreed = 0;
      } /* }}} while(isGreedy) */
    }
  
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
  rms = sqrt(rms);
  if (rms == 0.) {
    fprintf(stderr, "PSNR can't calclate...\n");
    exit(1);
  }
  psnr = 20.*log10(255./rms);

  return psnr;
}



