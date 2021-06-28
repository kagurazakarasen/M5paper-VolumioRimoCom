#include <M5EPD.h>

#include <WiFi.h>
#include "time.h"
#include <HTTPClient.h>
#include <string>  
using namespace std; 

string SSID = ""; // SSIDとキー、最初に自分の環境のを入れておく
string PASS = "";

string VolumioURL = "http://192.168.1.86";
//string VolumioURL = "http://volumio.local";

M5EPD_Canvas canvas(&M5.EPD);

int point[2][2];
int p_mode;

//◇構造体
typedef struct {   
    int x;
    int y;
    int w;
    int h;
} rect_t;

//ボタンの位置とサイズ
rect_t btns[3] = {
  {100,650,100,100},
  {220,650,100,100},
  {340,650,100,100}
};


void WiFi_setup()
{
     //connect to WiFi
     if( SSID.length() == 0){
       Serial.print("SSID Length = 0  -> Default Connect.");
       WiFi.begin();                 // 一度接続が成功したらこちらでOK.
      }else{
        WiFi.begin(SSID.c_str(), PASS.c_str()); // SSIDに文字列があればこちらで接続
      } 

       while (WiFi.status() != WL_CONNECTED) {
           delay(500);
           Serial.print(".");
       }
       Serial.println("CONNECTED");
}

void WiFI_off()
{
     //disconnect WiFi as it's no longer needed
   WiFi.disconnect(true);
   WiFi.mode(WIFI_OFF);
}

void setup()
{
    M5.begin();
    M5.EPD.SetRotation(90);
    M5.TP.SetRotation(90);
    M5.EPD.Clear(true);
    M5.RTC.begin();

    p_mode = 0; // stop

    //WiFi
    WiFi_setup();
    
    canvas.createCanvas(540, 960);
    
    canvas.setTextSize(3);
    //canvas.drawString("Volumio", 45, 15);
    canvas.drawString("Remote controller", 80, 50);

    canvas.drawRoundRect(50, 200, 400, 400, 5, 15);
    //canvas.fillRect(300, 200, 100, 100, 15);

    //ボタン描画
    canvas.drawRoundRect( btns[0].x, btns[0].y, btns[0].w, btns[0].h, 5, 15);
    canvas.drawRoundRect( btns[1].x, btns[1].y, btns[1].w, btns[1].h, 5, 15);
    canvas.drawRoundRect( btns[2].x, btns[2].y, btns[2].w, btns[2].h, 5, 15);

    canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
}

void MusicInfo(){
        HTTPClient http;
        http.begin("http://192.168.1.86/api/v1/getState");

        int httpCode = http.GET();
        if (httpCode > 0) {
          String response = http.getString();
          //以降、データに応じた処理
          //canvas.drawString(response, 10, 200);
          Serial.println(response);
        } else {
          Serial.println("Error on HTTP request");
        }
        http.end();
}

void TouchScan(){

  if(M5.TP.avaliable()){
    if(!M5.TP.isFingerUp()){
        M5.TP.update();
        canvas.fillCanvas(0);
        bool is_update = false;
        for(int i=0;i<2; i++){
            tp_finger_t FingerItem = M5.TP.readFinger(i);
            if((point[i][0]!=FingerItem.x)||(point[i][1]!=FingerItem.y)){
                is_update = true;
                point[i][0] = FingerItem.x;
                point[i][1] = FingerItem.y;
                canvas.fillRect(FingerItem.x-50, FingerItem.y-50, 100, 100, 15);
                Serial.printf("Finger ID:%d-->X: %d*C  Y: %d  Size: %d\r\n", FingerItem.id, FingerItem.x, FingerItem.y , FingerItem.size);
            }
        }


        if(is_update)
          {
              canvas.pushCanvas(0,0,UPDATE_MODE_GC16);
              //canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
          }
        }
    }
}

void AlbumArt(){

          //アルバムアート
        canvas.drawPngUrl("http://192.168.1.86/albumart?cacheid=223&web=Pharrell%20Williams/Girl/extralarge&path=%2Fmnt%2FNAS%2Fpub2%2FCompilations%2FGirl&metadata=false",50,200);
        //canvas.pushCanvas(0,0,UPDATE_MODE_GC16);

}

void ButtonTest(char* str, int cmd)
{
  // cmd = 1:pause/pley 2:rev 3:fwd

  //char ComStr1[ ] = "arduino";
  
    canvas.fillCanvas(0);
    canvas.drawString(str, 100, 100);

    HTTPClient http;

    string CmdStr = VolumioURL;

    switch (cmd) {
      case 1:
        CmdStr += "/api/v1/commands?cmd=toggle";
        //http.begin("http://192.168.1.86/api/v1/commands?cmd=toggle");
        http.begin(CmdStr.c_str());
        /*
        if(p_mode==1){
            http.begin("http://192.168.1.86/api/v1/commands?cmd=pause");
            p_mode=0;
        }else{
            http.begin("http://192.168.1.86/api/v1/commands?cmd=play");      
            p_mode=1;
        }
        */
        break;
      case 2:
        //http.begin("http://192.168.1.86/api/v1/commands?cmd=prev");
        CmdStr += "/api/v1/commands?cmd=prev";
        http.begin(CmdStr.c_str());
        break;
      case 3:
        CmdStr += "/api/v1/commands?cmd=next";
        http.begin(CmdStr.c_str());
        //http.begin("http://192.168.1.86/api/v1/commands?cmd=next");
        break;
    }
        
    int httpCode = http.GET();
    if (httpCode > 0) {
      String response = http.getString();
      //以降、データに応じた処理
      canvas.drawString(response, 0, 200);
      Serial.println(response);
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();


    
    canvas.pushCanvas(100,300,UPDATE_MODE_DU4);
    delay(500);
}


void loop()
{

    if( M5.BtnL.wasPressed()) ButtonTest("Prev",2);
    if( M5.BtnP.wasPressed()) ButtonTest("Play/Pause",1);
    if( M5.BtnR.wasPressed()) ButtonTest("Next",3);

    TouchScan();

    M5.update();
    delay(100);
    
            
}