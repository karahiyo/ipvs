/* defines.h */

#define SRC_NAME     "seq/bus1"

#define SRC_X_SIZE         352   /* source image horizontal size */
#define SRC_Y_SIZE         288   /* source image vertical size */

#define MB_SIZE             16   /* macro block size */
#define MB_X_NUM            22   /* TB number (SRC_X_SIZE/TB_X_SIZE) */
#define MB_Y_NUM            18   /* TB number (SRC_Y_SIZE/TB_Y_SIZE)  */

#define SW_SIZE              7   /* search window size */

#define HEAD_FRAME           2   /* head frame number */
#define TAIL_FRAME          23   /* tail frame number */
#define SKIP_FRAME           3   /* tail frame number */

/* etc */
#ifndef DBL_MAX
#define DBL_MAX  1.7976931348623157E+308
#endif

#define ROUND(x)       (((x)>= 0)?(int)(x+0.5):(int)(x-0.5))
