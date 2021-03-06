#include "includes.h"

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
  int bmcount=0;
  int b_num = MB_X_NUM * MB_Y_NUM;

  /* 探索範囲 */
  for (y = 0, yy = 0; y < SRC_Y_SIZE; y += MB_SIZE, yy++)
    for (x = 0, xx = 0; x < SRC_X_SIZE; x += MB_SIZE, xx++) {

      int sThin; // TSSの各探索ステップごとの探索ブロック間の幅
      int steps; // ステップ数のカウンタ
      int now_x, now_y; // 各ステップごとの開始点を記憶
      int vec_v_counter; // 動きベクトルを更新するタイミングを制御するためのカウンタ
      
      min = DBL_MAX;
      sThin = 4;
      steps = 1;
      now_x = 0, now_y = 0; // 探索のスタート点。なんでもいい。
      vec_v_counter = 1;

      while(steps <= 3) {
          /* 探索するブロックのポジションは (n*sThin, m*sThin) */
          for (n = -1; n <= 1; n++) 
            for (m = -1; m <= 1; m++) {
                /* 動きベクトルの更新 */
                if ( steps > vec_v_counter) {
                    vecy[yy*MB_X_NUM + xx] += now_y;
                    vecx[yy*MB_X_NUM + xx] += now_x;
                    vec_v_counter++;
                }   
                /* 画像端部の例外処理 */
                if((y + n*sThin + vecy[yy*MB_X_NUM + xx] < 0) || 
                    (x + m*sThin + vecx[yy*MB_X_NUM + xx] < 0) ||
                    (SRC_Y_SIZE < y + n*sThin + (int)vecy[yy*MB_X_NUM + xx] + MB_SIZE) ||
                    (SRC_X_SIZE < x + m*sThin + (int)vecx[yy*MB_X_NUM + xx] + MB_SIZE)) continue;

                /* 誤差の計算 */
	            sum = 0.0;
                bmcount++; //ブロックマッチング回数カウント
                for (j = 0; j < MB_SIZE; j++)
                  for (i = 0; i < MB_SIZE; i++) {
                      sum += fabs(crtmap[(y + j)*SRC_X_SIZE + (x + i)] -
			            premap[(y + n*sThin + (int)vecy[yy*MB_X_NUM+xx]+ j)*SRC_X_SIZE + (x + m*sThin + (int)vecx[yy*MB_X_NUM + xx] + i)]);
                }

	            /* 動きベクトルの更新 */
                if (sum < min) {
                    min = sum;
                    now_y = n*sThin;
                    now_x = m*sThin;
                }
            } /* }}} for(n)*for(m) */
          
          steps++;
          sThin /= 2; // 次の探索の間引き幅は、現在のものを "/2" したものとなる
      } /* }}} while(step < 4) */
    }
  fprintf(stderr,"min=%f \n",min);
  fprintf(stderr,"bmcount=%d \n", bmcount/b_num);
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



