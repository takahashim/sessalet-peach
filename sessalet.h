#ifndef _SESSALET_H_
#define _SESSALET_H_

#ifndef SESSALET_MAIN_TASK_PRI
#define  SESSALET_MAIN_TASK_PRI  2
#endif /*  SESSALET_MAIN_TASK_PRI */

#ifndef SESSALET_TASK_STACK_SIZE
#define SESSALET_TASK_STACK_SIZE 1024
#endif  /* SESSALET_TASK_STACK_SIZE */

/*
 * フラグのビット割り当て
 */
#define SEAT_ON       0x01U		// 着座検出
#define SEAT_OFF      0x02U     // 起立検出
#define DETECTED      0x04U     // 人検出
#define UNDETECTED    0x08U     // 人未検出
#define SPLAY_ON      0x10U
#define SPLAY_OFF     0x20U

#define CALIB_FONT (EV3_FONT_SMALL)
#define CALIB_FONT_WIDTH (6/*TODO: magic number*/)
#define CALIB_FONT_HEIGHT (8/*TODO: magic number*/)

#define NOZZLE_DEGREE 180
#define NOZZLE_OFF_DEGREE -180
#define NOZZLE_SPEED 50 //persent
#define SPLAY_DEGREE 90
#define SPLAY_CLOSE_DEGREE -90
#define SPLAY_OPEN_SPEED 50 //persent
#define SPLAY_CLOSE_SPEED 50 //persent
#define FLASH_MOTOR_SPEED 50 //persent
#define FLASH_DEGREE 90
#define FLASH_STOP_MOTOR_SPEED 50 //persent
#define FLASH_STOP_DEGREE -90
#define FLASH_TIME 3   //水を流す時間

#define COVER_OPEN_DEGREE 270
#define COVER_CLOSE_DEGREE -270
#define COVER_OPEN_SPEED 10 //persent
#define COVER_CLOSE_SPEED 10 //persent

#define COVER_CLOSE_TIME 5 // 「着座してない」検出から自動的にふたが閉まるまでの時間

#define HUMAN_DETECT 1000 //超音波センサーで人検出する距離
#define UNDETECTED_VAL 0
#define DETECTED_VAL 1

#define SEATED_MIN 10 //着座とみなす色
#define SEATED_MAX 50 //着座とみなす色

#define FLASH_SOUND NOTE_E4  //流水音

#define SPLAY_TIMER 10  //スプレーが自動的に止まるまでの時間
#define FLASH_TIMER 10

#define SD_RES_FOLDER "/ev3rt/res"

#ifdef __cplusplus
extern "C" {
#endif

extern void sessalet_main_task(intptr_t exinf);
extern void seated_task(intptr_t exinf);
extern void flash_task(intptr_t exinf);
extern void splay_task(intptr_t exinf);
extern void human_check_cyc(intptr_t exinf);
extern void seated_check_task(intptr_t exinf);
extern void flush_check_cyc(intptr_t exinf);

int human_check();
int seated_check();

#ifdef __cplusplus
}
#endif

#endif /* _SESSALET_H_ */
