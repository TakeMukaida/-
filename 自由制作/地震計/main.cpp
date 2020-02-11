//地震計
#include "inlcudes.h"

Serial pc(USBTX, USBRX);

AnalogIn  axises[3] = {     //アナログ入力ピン
    AnalogIn(A3),   //x
    AnalogIn(A4),   //y
    AnalogIn(A5)    //z
};

DigitalIn start_sign(USER_BUTTON);  //運転準備ボタンに変えるもよし

DigitalOut LEDs[4] = {
    DigitalOut(D2),     //room_light
    DigitalOut(D4),     //IH
    DigitalOut(D6),     //hot_water
    DigitalOut(LED2)    //hazard
};

int main() {
    pc.baud(9600);  //bits per second
    
    float data[4] = {0};

    short seismic_level = 0;
    
    while(!start_sign);
    
    while(1) {
        
        data[3] = 0;
         
        for(int i = 0; i < 3; i++) {
            data[i] = axises[i] * VCC - OFFSET;
            data[3] += pow((double)data[i], 2);  
//            pc.printf("%d:%5.2f, ", i, data[i]);
        }
        data[3] = sqrt(data[3]) / VPG - SG * 0.1f;  //加速度計算(通常時0m/s^2)
        data[3] *= 100;                             //gal換算
        data[3] = abs(data[3]);                     //絶対値
        
//        pc.printf("a = %05.1f[gal], ", data[3]);
        
        for(int cnt = 0; cnt < 10; cnt++) {         //震度測定
            if((PGA_data[cnt] <= data[3]) && (data[3] < PGA_data[cnt + 1])) {
                seismic_level = cnt;
                break;
            }
        }
 
//        pc.printf("value = %d\n", seismic_level);
          
        if(seismic_level > 6) {         //2Gを超過したため装置の破損が懸念される
//          pc.printf("!");
            LEDs[3] = 1;
        }
        else if(seismic_level > 4) {    //IHをストップ and 給湯もストップ
//          pc.printf("5");
            LEDs[2] = 0;
            LEDs[1] = 0;
            LEDs[0] = 1;
            wait(5);                    //震度５弱観測後、一定時間信号維持
        }
        else if(seismic_level > 2) {    //ライト点灯
//          pc.printf("3");
            LEDs[2] = 1;
            LEDs[1] = 1;
            LEDs[0] = 1;
        }
        else {
//          pc.printf("0");
            LEDs[3] = 0;
            LEDs[2] = 1;
            LEDs[1] = 1;
            LEDs[0] = 0;  
        }
        
//        pc.printf("\n");
    }
}
