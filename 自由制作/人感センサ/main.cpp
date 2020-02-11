//人感センサ
#include "mbed.h"
#include "math.h"

#define Vcc 5

AnalogIn s0(A2);    //in
AnalogIn s1(A3);    //out
DigitalIn seismic_signal(D8);   //震度伝達
DigitalOut room_led(D2);        //室内灯
DigitalOut emergency_led(D4);   //非常灯

Serial pc(USBTX, USBRX);

Timer timer;

int main() {
    
    pc.baud(921600);
    
    double initial_data[2] = {0};
    double measure_data[2] = {0};
    
    bool seismic_flag = false;
    int flag[4] = {false};
    
    initial_data[0] = s0;
    initial_data[1] = s1;
    
    int human = 0, log_human = 0;
    float time;
    short cnt_invalid = 0;
    
    //timer.start();
    
    while(1) {
        measure_data[0] = s0;
        measure_data[1] = s1;
        /*
        for(cnt = 0; cnt < 2; cnt++) {
            measure_data[cnt] = Vcc * measure_data[cnt] / 1023;
            measure_data[cnt] = 26.549 * pow(measure_data[cnt], -1.2091);
        }
        */
        if(log_human != human)
            cnt_invalid = 1000;
            
        log_human = human;
        //初期待機
        if(initial_data[0] < (measure_data[0] - 0.1)/* && flag[2] == false && flag[3] == false*/) //in
            flag[0]++;
        else if(flag[0] != 0)
            flag[0] = 0;
        if(initial_data[1] < (measure_data[1] - 0.1)/* && flag[2] == false && flag[3] == false*/) //out
            flag[1]++;
        else if(flag[1] != 0)
            flag[1] = 0;
        
        if(flag[0] > 9 && !(flag[2] == true && flag[3] == false)) {
            flag[0] = 0;
            flag[2] = true;
            timer.reset();
            timer.start();
        }
        if(flag[1] > 9 && !(flag[2] == false && flag[3] == true)) {
            flag[1] = 0;
            flag[3] = true;
            timer.reset();
            timer.start();
        }
        
        
        time = timer.read();
        if(time > 5) {          
            timer.stop();
            timer.reset();
            for(int cnt = 0; cnt < 4; cnt++)
                flag[cnt] = 0;
        }
        
        //通過判定
        if(flag[2] == true && flag[3] == false && (time < 5) && flag[1] > 7) {//in
            human++;
            for(int cnt = 0; cnt < 4; cnt++)
                flag[cnt] = 0;
            timer.stop();
            timer.reset();
        }
        if(flag[2] == false && flag[3] == true && (time < 5) && flag[0] > 7) {//out
            if(human < 1)
                human = 0;
            else
                human--;
            for(int cnt = 0; cnt < 4; cnt++)
                flag[cnt] = 0;
            timer.stop();
            timer.reset();
        }
          
        if(cnt_invalid > 0) {
            for(int cnt = 0; cnt < 4; cnt++)
                flag[cnt] = 0;
            timer.stop();
            timer.reset();
            cnt_invalid--;
        }
        else 
            cnt_invalid = 0;
              
        seismic_flag = seismic_signal;
        
        if(seismic_flag)
            emergency_led = true;
        if(human || seismic_flag)
            room_led = true;
        else {
            room_led = false;
            emergency_led = false;
        }
            
        //timer.stop();
        time = timer.read();
        pc.printf("h:%d, 1:%f, 2:%f, time:%f, f:%02d%02d%02d%02d, ic:%d\n", human, measure_data[0], measure_data[1], time, flag[0], flag[1], flag[2], flag[3], cnt_invalid);
        //timer.start();
    }
}
