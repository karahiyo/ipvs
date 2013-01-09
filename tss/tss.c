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
int threeStepSearch(float *premap, float *crtmap, float *vecy, float *vecx)
{
  double sum, min;
  register int i, j, m, n, x, y, xx, yy;

  for (y = 0, yy = 0; y < SRC_Y_SIZE; y += MB_SIZE, yy++)
    for (x = 0, xx = 0; x < SRC_X_SIZE; x += MB_SIZE, xx++) {
      /* 探索範囲 */
      min = DBL_MAX;

      int sThin = 4; // TSSの各探索ステップごとの探索ブロックの間引く間隔
      int steps = 1; // ステップ数のカウンタ
      int now_x,now_y; // 各ステップごとの開始点を記憶
      now_x = 0, now_y = 0; // ともに0で初期化
      int vec_v_counter = 1; // 動きベクトルを更新するタイミングを制御するためのカウンタ
      
      while(steps < 4) {
          //fprintf(stderr,"[STEP %d]\n",steps);
          /* 探索するブロックは (n*sThin, m*sThin) となる */
          //fprintf(stderr,"now_y,x=%d,%d\n",now_y,now_x);
          for (n = -1; n <= 1; n++) 
            for (m = -1; m <= 1; m++) {
                /* 動きベクトルの更新 */
                if ( steps > vec_v_counter) {
                    vecy[yy*MB_X_NUM + xx] += now_y;
                    vecx[yy*MB_X_NUM + xx] += now_x;
                    //fprintf(stderr,"vecy=%d,vecx=%d\n",vecy[yy*MB_X_NUM + xx],vecx[yy*MB_X_NUM + xx]);
                    vec_v_counter++;
                }   
                /* 画像端部の例外処理 */
                if((yy + n*sThin + vecy[yy*MB_X_NUM + xx] < 0) || 
                    (xx + m*sThin + vecx[yy*MB_X_NUM + xx] < 0) ||
                    (MB_Y_NUM < yy + n*sThin + vecy[yy*MB_X_NUM + xx] + 1) ||
                    (MB_X_NUM < xx + m*sThin + vecx[yy*MB_X_NUM + xx] + 1)) continue;

                /* 誤差の計算 */
	            sum = 0.0;
                bmcount++; //ブロックマッチング回数カウント
                for (j = 0; j < MB_SIZE; j++)
                  for (i = 0; i < MB_SIZE; i++) {
                      //fprintf(stderr,"n,sThin=%d,%d",n,sThin);
                      sum += fabs(crtmap[(y + j)*SRC_X_SIZE + (x + i)] -
			            premap[(y + n*sThin + j)*SRC_X_SIZE + (x + m*sThin + i)]);
                }

	            /* 動きベクトルの更新 */
                if (sum < min) {
                    //fprintf(stderr,"sum=%f < min=%f\n",sum,min);
                    min = sum;
                    now_y = n*sThin;
                    now_x = m*sThin;
                    //fprintf(stderr,"min=%f   \t",min);
                    //fprintf(stderr,"(n,m,sthin)=(%d,%d,%d)\n",n,m,sThin);
                }
            } /* </for(SW)> */
          
          /* ブロックマッチング回数の平均計算*/
          if(bmc_ave != 0) 
              bmc_ave = (bmc_ave + bmcount)/2;
          else
              bmc_ave = bmcount;
          
          steps++;
          sThin /= 2; // 次の探索の間引き幅は、現在のものを "/2" したものとなる
      } /* </while>*/
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



