#include "includes.h"

/*===============================================================
  routine    : FullSearch
  return val : int
  input  par : float *premap
               float *crtmap
	       float *vecy
	       float *vecx
  function   : full search
 ===============================================================*/
int fullSearch(float *premap, float *crtmap, float *vecy, float *vecx)
{
  double sum, min;
  register int i, j, m, n, x, y, xx, yy;
  int bmcount = 0; //ブロックマッチング回数をカウント
  int b_num = MB_X_NUM*MB_Y_NUM;

  for (y = 0, yy = 0; y < SRC_Y_SIZE; y += MB_SIZE, yy++) {
    for (x = 0, xx = 0; x < SRC_X_SIZE; x += MB_SIZE, xx++) {

      /* 探索範囲 */
      min = DBL_MAX;
      for (n = -SW_SIZE; n <= SW_SIZE; n++) 
        for (m = -SW_SIZE; m <= SW_SIZE; m++) {
	  /* 画像端部の処理 */
          if ((y + n < 0) || (x + m < 0) ||
              (SRC_Y_SIZE < y + n + MB_SIZE) ||  
              (SRC_X_SIZE < x + m + MB_SIZE)) continue; 

	  /* 誤差の計算 */
	  sum = 0.0;
      bmcount++;
        for (j = 0; j < MB_SIZE; j++)
          for (i = 0; i < MB_SIZE; i++) {
	        sum += fabs(crtmap[(y + j)*SRC_X_SIZE + (x + i)] -
			  premap[(y + n + j)*SRC_X_SIZE + (x + m + i)]);
            //bmcount++; /* ブロックマッチング回数のカウント*/
	    }

	  /* 動きベクトルの更新 */
          if (sum < min) {
              min = sum;
              vecy[yy*MB_X_NUM + xx] = n;
              vecx[yy*MB_X_NUM + xx] = m;
          }
	}
    }
  }
        
  fprintf(stderr,"min=%f \n",min);
  fprintf(stderr,"bm_avg=%d \n",bmcount/b_num);
  
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



