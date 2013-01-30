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
int hexagonSearch(float *premap, float *crtmap, float *vecy, float *vecx)
{
  double sum, min;
  register int i, j, m, n, x, y, xx, yy;

  struct _hexagon_struc {
      int x;
      int y;
  };

  /* LDSP */
  struct _hexagon_struc lhsp[7] = 
            {{0,0},{-2,0},{-1,2},{1,2},{2,0},{1,-2},{-1,-2}};

  /* SDSP */
  struct _hexagon_struc shsp[5] =
            {{0,0},{-1,0},{0,1},{1,0},{0,-1}};

  /* 探索範囲 */
  for (y = 0, yy = 0; y < SRC_Y_SIZE; y += MB_SIZE, yy++)
    for (x = 0, xx = 0; x < SRC_X_SIZE; x += MB_SIZE, xx++) {

      static int now_x, now_y; // 各ステップごとの開始点を記憶
      double round_min; // hexagon-basedサーチの一回の試行の中での最小値の値
      int yet[MB_SIZE][MB_SIZE]; // マクロブロックサイズの配列。各画素位置が探索済かどうかを保持
      int lhspGreed; // lhsp探索終了マーク
      int shspGreed; // shsp探索終了マーク
      int tmp_vecy=0,tmp_vecx=0;
      
      min = DBL_MAX;
      round_min = DBL_MAX;
      now_x = 0, now_y = 0; // 探索開始点(0,0)
      lhspGreed = 1;
      int isblind; // 探索空間が行き止まりかフラグ。0なら行き止まり

      /* マクロブロックサイズの配列をゼロ(=未探索)で初期化 */
      for(n = -SW_SIZE; n <= SW_SIZE; n++)
        for(m = -SW_SIZE; m <= SW_SIZE; m++){
           yet[n + SW_SIZE][m + SW_SIZE] = 0; // 配列の添字が負の値にならないようにSW_SIZEを加算
      }

      
      while(lhspGreed != 0) {
          round_min = min;
          isblind = 0;
          fprintf(stderr,"NOW(%d,%d) -> ",now_y,now_x);

          /* LHSP探索 */
          for (n = 0; n < 7; n++) {
              /* 画像端部の例外処理 */
                if((y + now_y + lhsp[n].y < 0) || 
                    (x + now_x + lhsp[n].x < 0) ||
                    (SRC_Y_SIZE < y + now_y + lhsp[n].y + MB_SIZE) ||
                    (SRC_X_SIZE < x + now_x + lhsp[n].x + MB_SIZE)) continue;

                if((now_y + lhsp[n].y < -7) ||
                    (now_x + lhsp[n].x < -7) ||
                    (now_y + lhsp[n].y > 7) ||
                    (now_x + lhsp[n].x > 7)) { 
                    fprintf(stderr,"\n\t ** out => %d(%d,%d)\n",n,now_y+lhsp[n].y,now_x+lhsp[n].x);
                    continue;
                }

                /* 探索済の点か判定 */
                if (yet[now_y + lhsp[n].y + SW_SIZE][now_x + lhsp[n].x + SW_SIZE]==1)
                    continue;
                else
                    yet[now_y + lhsp[n].y + SW_SIZE][now_x + lhsp[n].x + SW_SIZE] = 1;

                /* 誤差の計算 */
	            sum = 0.0;
                bmcount++; //ブロックマッチング回数カウント
                isblind = 1; // 行き止まりではないフラグ ON
                for (j = 0; j < MB_SIZE; j++)
                  for (i = 0; i < MB_SIZE; i++) {
                      sum += fabs(crtmap[(y + j)*SRC_X_SIZE + (x + i)] -
			            premap[(y + now_y + lhsp[n].y + j)*SRC_X_SIZE + (x + now_x + lhsp[n].x + i)]);
                }

	            /* 一試行の最小値(round_min)と動きベクトルの更新 */
                if (sum < round_min) {
                    round_min = sum;
                    tmp_vecy = lhsp[n].y;
                    tmp_vecx = lhsp[n].x;
                }
            } /* }}} for(n)*for(m) */

           /* 探索開始点の移動 */
           if(isblind == 1) {
                now_y += tmp_vecy;
                now_x += tmp_vecx;
           } else {
                vecy[yy*MB_X_NUM + xx] = now_y;
                vecx[yy*MB_X_NUM + xx] = now_x;
                lhspGreed = 0;
                fprintf(stderr,"::vec(y,x)=(%d,%d)\n",(int)vecy[yy*MB_X_NUM+xx],(int)vecx[yy*MB_X_NUM+xx]);
           }

//            fprintf(stderr,"::vec(y,x)=(%d,%d) \n",vecy[yy*MB_X_NUM+xx],vecx[yy*MB_X_NUM+xx]);


            /* 終了条件の判定 */
            if (round_min < min )
                min = round_min;
            else {
                vecy[yy*MB_X_NUM + xx] = now_y;
                vecx[yy*MB_X_NUM + xx] = now_x;
                lhspGreed = 0;
            fprintf(stderr,"::vec(y,x)=(%d,%d)\n",(int)vecy[yy*MB_X_NUM+xx],(int)vecx[yy*MB_X_NUM+xx]);
            }
    } /* }}} while(lhspGreed) */
    
  
  /* SHSP探索 */
  tmp_vecy=0,tmp_vecx=0;
  fprintf(stderr,"\tSHSP(%d,%d) -> ",(int)vecy[yy*MB_X_NUM + xx],(int)vecx[yy*MB_X_NUM + xx]);
  for(n=0; n<5; n++) {
    isblind = 0; // 行き止まりフラグ OFF
    /* 画像端部の例外処理 */
    if((y + now_y + shsp[n].y < 0) || 
        (x + now_x + shsp[n].x < 0) ||
        (SRC_Y_SIZE < y + now_y + shsp[n].y + MB_SIZE) ||
        (SRC_X_SIZE < x + now_x + shsp[n].x + MB_SIZE)) continue;

    if((now_y + shsp[n].y < -7) ||
        (now_x + shsp[n].x < -7) ||
        (now_y + shsp[n].y > 7) ||
        (now_x + shsp[n].x > 7)) { 
        fprintf(stderr,"\n\t\t ** out => %d(%d,%d)\n",n,now_y+shsp[n].y,now_x+shsp[n].x);
        continue;
    }

    /* 誤差の計算 */
    sum = 0.0;
    bmcount++; //ブロックマッチング回数カウント
    isblind = 1; // 行き止まりフラグ ON
    for (j = 0; j < MB_SIZE; j++)
        for (i = 0; i < MB_SIZE; i++) {
            sum += fabs(crtmap[(y + j)*SRC_X_SIZE + (x + i)] -
                premap[(y + now_y + shsp[n].y + j)*SRC_X_SIZE + (x + now_x + shsp[n].x + i)]);
        }

    /* 一試行の最小値(round_min)と動きベクトルの更新 */
    if (sum < min) {
        min = sum;
        tmp_vecy = shsp[n].y;
        tmp_vecx = shsp[n].x;
    }
  } /* }}} for(shsp) */

    /* 動きベクトルを更新 */
    if(isblind == 1) {
        vecy[yy*MB_X_NUM + xx] += tmp_vecy;
        vecx[yy*MB_X_NUM + xx] += tmp_vecx;
    }
    fprintf(stderr,"\t::vec(y,x)=(%d,%d)\n",(int)vecy[yy*MB_X_NUM+xx],(int)vecx[yy*MB_X_NUM+xx]);
    

  } /* }}} for(SRC_Y_SIZE)*for(SRC_X_SIZE) */
  fprintf(stderr,"\n********************************* \nmin=%f \n",min);
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



