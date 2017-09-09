#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"

#include "mbed.h"
#include "motor.h"

#include "rohm-sensor-hal/rohm_hal.h"
#include "rohm-sensor-hal/I2CCommon.h"
#include "rohm-rpr0521/rpr0521_driver.h"
#include "rohm-rpr0521/rpr0521.h"

#include "sessalet.h"

#define     MAX_SPEED           100      /* motor()  set: 0 to 100   */

typedef enum Human {
	undetected = 0,
	detected
}Human;

typedef enum Splay{
	stop = 0,
	splay
}Splay;

typedef enum CoverMode{
	cover_close = 0,
	cover_open
}CoverMode;

static Splay splay_mode = stop;	//使用
static CoverMode cover_mode = cover_close;
static Human human_mode = undetected;
//TODO イベントフラグを使うように変更する
static FLGPTN flg_sessalet = 0;
static FLGPTN flg_splay = 0;
static FLGPTN flg_seated = 0;

static int timeout_flg = 0;
static int timeout_counter = 0;

static Motor cover_motor(P4_7, P4_6, P4_5);
static Motor spray_motor(P10_12, P10_15, P8_11);

//プッシュボタン（黄色 Spray）
DigitalIn pushbutton(P10_14);
//プッシュボタン（赤色 Flush）
DigitalIn flushbutton(P8_13);

//焦電センサー(人検知)
DigitalIn human_sensor2(P10_13);

//rohm-rpr0521のDEBUG_printf(...)で使うためのシリアル入出力
Serial pc(USBTX, USBRX);

unsigned char get_psalsval(uint16_t *ps_val, uint16_t *als_val) {
  unsigned char error;
  uint16_t data[3];
  error = (unsigned char)rpr0521_read_data(&data[0]);
  *ps_val = data[0];
  *als_val = data[1];
  return error;
}


int seated_check(){
	int result = UNDETECTED_VAL;
	uint16_t als_val;
	unsigned short ps_val;
	unsigned char rc = get_psalsval(&ps_val, &als_val);
	if (rc == 0) {
		syslog(LOG_ERROR, "seated check ok");
		if (ps_val > HUMAN_DETECT) {
			result = DETECTED_VAL;
		}
	}
	return result;
}

int human_check(){
	int result = UNDETECTED_VAL;
	uint16_t als_val;
	unsigned short ps_val;
	unsigned char rc = get_psalsval(&ps_val, &als_val);
	if (rc == 0) {
		syslog(LOG_ERROR, "sensor check ok");
		if (ps_val > HUMAN_DETECT) {
			result = DETECTED_VAL;
		}
	}
	return result;
}

int human_check2(){
	int result = UNDETECTED_VAL;
	int rc = human_sensor2.read();
	if (rc == 1) {
			syslog(LOG_ERROR, " %d[cnt]-> Near", rc);
		result = DETECTED_VAL;
	} else {
			syslog(LOG_ERROR, " %d[cnt]-> Far", rc);
	}
	return result;
}

void flush_check_cyc(intptr_t unused){
	syslog(LOG_ERROR, "--- flash check---");
	 if(flushbutton){
 		syslog(LOG_ERROR, "not push");
	 }else{
	  		syslog(LOG_ERROR, "fpush push");
			act_tsk(FLASH_TASK);
	 }

}

void human_check_cyc(intptr_t unused) {
	int result = human_check2();
	switch(result){
		case DETECTED_VAL:
//TODO				  set_flg(flg_sessalet, DETECTED);
			syslog(LOG_ERROR, "human detected");
			flg_sessalet = DETECTED;
			human_mode = detected;
			break;
		case UNDETECTED_VAL:
//TODO				  set_flg(flg_sessalet, UNDETECTED);
			syslog(LOG_ERROR, "human undetected");
			flg_sessalet = UNDETECTED;
			human_mode = detected;
			break;
		default:
			break;
	}

}

// Flashタスク流水音の開始と指定時間後の停止
void flash_task(intptr_t unused) {
	syslog(LOG_ERROR, "--- flash task---");
	  flashstart();
}

void initialize(){
    I2CCommonBegin();
    rpr0521_wait_until_found();
    rpr0521_initial_setup();
//      dly_tsk(1);
//      sta_cyc(HUMAN_CHECK_CYC);

}

void coverclose(){
	cover_motor.drive(100);
	dly_tsk(1000);
	cover_motor.drive(0);
	dly_tsk(3000);	// フタが閉まるまで待つ（パカパカするの防止）
}

void coveropen(){
	cover_motor.drive(-100);
	dly_tsk(3000);
	cover_motor.drive(0);
	dly_tsk(1000);	// 少し待つ
}

void spraystart(){
	spray_motor.drive(100);
	dly_tsk(3000);
	cover_motor.drive(0);
	dly_tsk(1000);	// 少し待つ
}

void spraystop(){
	spray_motor.drive(100);
	dly_tsk(500);
	spray_motor.drive(0);
	dly_tsk(100);
}

void flashstart(){
	//TODO
}

void sessalet_main_task(intptr_t exinf) {

	char msg[100];
	syslog(LOG_INFO, "start Sessalet!");
	initialize();

	sta_cyc(HUMAN_CHECK_CYC);
	while(1) {

		syslog(LOG_ERROR, "loop");
	    dly_tsk(5000);

		// 人検出か着座検出を待つ。5秒間たったらふたを閉める
		//TODO	  ercd = twai_flg(flg_sessalet, DETECTED|SEAT_ON, TWF_ORW, &flgptn, 5000);
		timeout_flg = 0;
		timeout_counter = 0;
		while(timeout_flg == 0){	//タイムアウトがきたら1に変わっているはず
			tslp_tsk(1000);
			if((flg_sessalet == DETECTED) && (cover_mode == cover_open)){
				int result = seated_check();
				switch(result){
					case DETECTED_VAL:
						syslog(LOG_ERROR, "-- seated");
						flg_seated = SEAT_ON;
						break;
					case UNDETECTED_VAL:
						syslog(LOG_ERROR, "-- not seated");
						flg_seated = SEAT_OFF;
						break;
					default:
						break;
				}
			}
			if((flg_sessalet == DETECTED) && (flg_seated == SEAT_ON)){//着座検出 -> 着座状態へ遷移
				memset(msg, 0x00, sizeof(msg));
				sprintf(msg, "--- act seated task, from main task ---\r\n");
				//			(void)serial_wri_dat(SIO_PORT_BT, msg, strlen(msg));
				syslog(LOG_ERROR, msg, sizeof(msg));
				syslog(LOG_ERROR, "seated mode");
				int splay_flg = 0;
				while(1){	//着座中のループ
					int result = seated_check();
					switch(result){
						case DETECTED_VAL:
							syslog(LOG_ERROR, "-- seated");
							flg_seated = SEAT_ON;
							break;
						case UNDETECTED_VAL:
							syslog(LOG_ERROR, "-- not seated");
							flg_seated = SEAT_OFF;
							break;
						default:
							break;
					}
					if(flg_seated == SEAT_OFF){
						syslog(LOG_ERROR, "UNDETECTED exit seated loop");
						break;	//exit 着座中のループ
					}
					if(pushbutton){
						syslog(LOG_ERROR, "**spray not push");
					}else{	//スプレーボタン
						if(splay_flg == 0){
							syslog(LOG_ERROR, "**splay start");
							//flush
							spraystart();
							splay_flg = 1;
						}else{
							syslog(LOG_ERROR, "**splay stop");
							splay_flg = 0;
							spraystop();
						}
					}
					dly_tsk(500);
				}
//  		  act_tsk(SEATED_TASK);
///  			  slp_tsk();

			}else if(flg_sessalet == DETECTED){	//人検出

				if(cover_mode == cover_close){
					memset(msg, 0x00, sizeof(msg));
					sprintf(msg, "--- cover open ---\r\n");
					syslog(LOG_ERROR, msg, sizeof(msg));
					coveropen();
					cover_mode = cover_open;
					dly_tsk(1000);
	//				act_tsk(SEATED_TASK);
	//   			  slp_tsk();
				}
			}else if(flg_sessalet == UNDETECTED){
				if(cover_mode == cover_open){
					timeout_counter++;
					if(timeout_counter > 5){
						timeout_flg = 1;
					}
				}
			}else if(flg_sessalet == SEAT_OFF){

			}
		}
		if(timeout_flg == 1){
			timeout_flg = 0;
			timeout_counter = 0;
			memset(msg, 0x00, sizeof(msg));
			sprintf(msg, "--- timeout cover close ---\r\n");
			syslog(LOG_ERROR, msg, sizeof(msg));
			coverclose();
			cover_mode = cover_close;
		}
		dly_tsk(500);
	}
}
