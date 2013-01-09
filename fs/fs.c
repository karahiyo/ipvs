#include "includes.h"

int bmcount;
int bmc_ave;
extern int bmcount;
extern int bmc_ave;

int px, py;

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
      bmcount++; /* ブロックマッチング回数のカウント*/
        for (j = 0; j < MB_SIZE; j++)
          for (i = 0; i < MB_SIZE; i++) {
	        sum += fabs(crtmap[(y + j)*SRC_X_SIZE + (x + i)] -
			  premap[(y + n + j)*SRC_X_SIZE + (x + m + i)]);
	    }
	  /* 動きベクトルの更新 */
          if (sum < min) {
              min = sum;
              vecy[yy*MB_X_NUM + xx] = n;
              vecx[yy*MB_X_NUM + xx] = m;
              py = n;
              px = m;
              //fprintf(stderr,"min=%f   \t",min);
              //fprintf(stderr,"(n,m)=(%d,%d)\n",n,m);
          }
	} /* </for(SW)>*/
      /* ブロックマッチング回数の平均計算*/
      if(bmc_ave != 0) 
        bmc_ave = (bmc_ave + bmcount)/2;
      else
        bmc_ave = bmcount; 
    }
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



