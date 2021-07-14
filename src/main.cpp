#include <M5EPD.h>

#include <WiFi.h>
#include "time.h"
#include <HTTPClient.h>
#include <regex>
#include <string>  
using namespace std; 

#define FF18 &FreeSans12pt7b

string SSID = ""; // SSIDとキー、最初に自分の環境のを入れておく
string PASS = "";

string VolumioURL = "http://192.168.1.86";    // 自環境のVolumioURL
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

typedef struct {   
    int x;
    int y;
} Pos_t;


//ボタンの位置とサイズ
rect_t btns[3] = {
  {100,650,100,100},
  {220,650,100,100},
  {340,650,100,100}
};

rect_t infoRect={
  100, 220, 350, 350
};

rect_t volRect={
  510, 350, 30, 300
};

int VOL = 50;

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

void BtnDraw(int No,bool B){
    canvas.deleteCanvas();  //既存のdeleteCanvas
    canvas.createCanvas(100,100); // 100x100のCanvas作成

    if(B){    // B が Trueならボタン正常標示、Falseなら反転
        canvas.drawRoundRect( 0,0,100,100, 5, 15);
    } else {
        canvas.fillRoundRect(0, 0, 100, 100, 5,15); // ボタン塗りつぶし反転
    }
    switch(No){
      case 0:
        canvas.drawString("<<", 10, 40);
        break;
      case 1:
        canvas.drawString("Play/", 6, 20);
        canvas.drawString("Pause", 6, 50);
        break;
      case 2:
        canvas.drawString(">>", 10, 40);
        break;
    }


    canvas.pushCanvas(btns[No].x,btns[No].y,UPDATE_MODE_DU4);
}


void VolDraw(){
    canvas.deleteCanvas();  //既存のdeleteCanvas
    canvas.createCanvas(volRect.w,volRect.h); // 100x100のCanvas作成

    canvas.fillRect(0,    volRect.h-VOL ,volRect.w,  VOL ,15);  // VOLUME

    canvas.pushCanvas( volRect.x,volRect.y,UPDATE_MODE_DU4);
}


void EPDinit()
{
  canvas.deleteCanvas(); // setup時には無意味？
    canvas.createCanvas(540, 960);
    
    canvas.setTextSize(3);

    //canvas.drawString("Volumio", 45, 15);
    canvas.drawString("Remote controller", 80, 50);

    canvas.drawRoundRect(infoRect.x, infoRect.y, infoRect.w, infoRect.h, 5, 15);
    //canvas.drawRoundRect(100, 220, 350, 350, 5, 15);
    //canvas.fillRect(300, 200, 100, 100, 15);

    //ボリューム枠
    canvas.drawRect(volRect.x,volRect.y,volRect.w,volRect.h,15);
    //仮ボリューム
    canvas.fillRect(volRect.x,volRect.y+100,volRect.w,volRect.h-100,15);


    canvas.pushCanvas(0,0,UPDATE_MODE_DU4);

  //BtnDrawAll();

    //ボタン描画
    BtnDraw(0,true);
    BtnDraw(1,true);
    BtnDraw(2,true);


}

void setup()
{
    M5.begin();
    M5.EPD.SetRotation(90);
    M5.TP.SetRotation(90);
    M5.EPD.Clear(true);
    M5.RTC.begin();


  canvas.createCanvas(540, 960);
  canvas.setTextDatum(TC_DATUM);
  canvas.setTextSize(3);

  canvas.drawString("... Initializing ...", 270, 640);

  canvas.drawString("Loading font ...", 270, 230);
   // canvas.loadFont("/font.ttf",SD);
    
    //canvas.loadFont("/GenSenRounded-R.ttf",SD);
    canvas.createRender(96, 256); //? 
    canvas.createRender(32, 256);


  canvas.drawString("OK!", 270, 254);
  canvas.pushCanvas(0,0,UPDATE_MODE_DU4);



    p_mode = 0; // stop

    //WiFi
    WiFi_setup();
    
    EPDinit();

}

void AlbumArt(){

          //アルバムアートテスト
        canvas.drawPngUrl("http://192.168.1.86/albumart?cacheid=223&web=Pharrell%20Williams/Girl/extralarge&path=%2Fmnt%2FNAS%2Fpub2%2FCompilations%2FGirl&metadata=false",50,200);
        //canvas.pushCanvas(0,0,UPDATE_MODE_GC16);

}

// in_str中から S_strを探し、その,次の””でくくられたStringを返す
String InfoGet(String in_str, String S_str){
        String ret_str = "";
        int l = in_str.indexOf(S_str);
          //Serial.println(l);
        if(l>0){
          int l2 = in_str.indexOf("\"",l+S_str.length()+3); // +3は、検索対象の後ろの ",:" の分。
          //Serial.println(l2);
          ret_str = String( in_str.substring(l+S_str.length()+3,l2));
          //Serial.println(l2);
          // canvas.println(in_str.substring(l+S_str.length()+3,l2));
          
        }      
        return(ret_str);
}

void MusicInfo(){
    string CmdStr = VolumioURL;
    String res_str;
  
        HTTPClient http;
        CmdStr += "/api/v1/getState";
        http.begin(CmdStr.c_str());
        //http.begin("http://192.168.1.86/api/v1/getState");

        int httpCode = http.GET();
        if (httpCode > 0) {
          res_str = http.getString();
          //以降、データに応じた処理
          Serial.println(res_str);
        } else {
          Serial.println("Error on HTTP request");
        }
        http.end();

         // 既存 canvas の削除
        canvas.deleteCanvas();
        // 新規 canvas の生成 （幅 350 x 高さ 25 [pixel]）
        canvas.createCanvas( infoRect.w, infoRect.h);

/*
        String title_str = "";
        int l = res_str.indexOf("title");
        if(l>0){
          int l2 = res_str.indexOf("\"",l+8);
          title_str= String( res_str.substring(l,l2));
          canvas.println(res_str.substring(l+8,l2));
          
        }      
*/
        String title_str = InfoGet(res_str,"title");
        canvas.println("TITLE:");
        canvas.println();
        canvas.println(title_str);
        canvas.println();

        String album_str = InfoGet(res_str,"album");
        canvas.print("ALBUM:");
        canvas.println(album_str);
        canvas.println();

        String artist_str = InfoGet(res_str,"artist");
        canvas.print("artist:");
        canvas.println(artist_str);
        canvas.println();

        // AlbumArt
        String albumArt_str = InfoGet(res_str,"albumart");
        //canvas.println(albumArt_str);
        //canvas.println();
        CmdStr = VolumioURL;
        CmdStr += albumArt_str.c_str();
        Serial.println(CmdStr.c_str());


    canvas.drawRoundRect(0,0, infoRect.w, infoRect.h, 5, 15); //枠書き直し


    canvas.pushCanvas(infoRect.x, infoRect.y, UPDATE_MODE_DU4);


         // 既存 canvas の削除
        canvas.deleteCanvas();
        // 新規 canvas の生成 （幅 350 x 高さ 25 [pixel]）
        canvas.createCanvas( infoRect.w, infoRect.h);

         canvas.drawPngUrl(CmdStr.c_str(),0,0); // アルバムアートDRAW

        canvas.pushCanvas(infoRect.x, infoRect.y, UPDATE_MODE_GC16);


        //string str1 = regex_replace(res_str, "title", " \r\n");

       // canvas.println(res_str);

     // canvas.drawRoundRect(0, 0, 350, 350, 5, 15);
}

void TouchScan(Pos_t *p){

  if(M5.TP.avaliable()){      // タッチセンサが生きていて
    if(!M5.TP.isFingerUp()){  // 指が触れていたら（isFingerUp 否定）
        M5.TP.update();

        // サンプルより。二重に検出している。（そうしないと無入力でヒットし続けちゃう模様）        
        bool is_update = false;
        int x;
        int y;
        for(int i=0;i<2; i++){
            tp_finger_t FingerItem = M5.TP.readFinger(i);
            if((point[i][0]!=FingerItem.x)||(point[i][1]!=FingerItem.y)){
                is_update = true;
                point[i][0] = FingerItem.x;
                x = FingerItem.x;
                point[i][1] = FingerItem.y;
                y = FingerItem.y;
                //canvas.fillRect(FingerItem.x-50, FingerItem.y-50, 100, 100, 15);
                Serial.printf("Finger ID:%d-->X: %d /  Y: %d  Size: %d\r\n", FingerItem.id, FingerItem.x, FingerItem.y , FingerItem.size);
            }
        }
        if(is_update)          {
              p->x = x;
              p->y = y;
        }
      }
    }
}


void ButtonTest(char* str, int cmd)
{

      // 既存 canvas の削除
    canvas.deleteCanvas();

    // 新規 canvas の生成 （幅 350 x 高さ 25 [pixel]）
    canvas.createCanvas(350, 25);

    HTTPClient http;

    string CmdStr = VolumioURL;

    switch (cmd) {
      case 1:
        CmdStr += "/api/v1/commands?cmd=toggle";
        http.begin(CmdStr.c_str());
        break;
      case 2:
        if(VOL<90){
          VOL += 10;
        }
        VolDraw();
        //CmdStr += "/api/v1/commands?cmd=prev";
        //http.begin(CmdStr.c_str());
        break;
      case 3:
        if(VOL>10){
          VOL -= 10;
        }
        VolDraw();
        //CmdStr += "/api/v1/commands?cmd=next";
        //http.begin(CmdStr.c_str());
        break;
    }
        
    int httpCode = http.GET();
    if (httpCode > 0) {
      String response = http.getString();
      //以降、データに応じた処理
      canvas.drawString(response, 0, 0);
      Serial.println(response);
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();


    // canvas に図形を描く （canvas 座標系において (5, 5) を起点とする幅 30 x 高さ 15 [pixel] の矩形）
    // 引数の末尾は 16 階調の色（濃さ）で最も濃い
    canvas.drawRect(5, 5, 30, 15, 15);

    // canvas を表示 （画面座標系において (185, 10) を起点に）
    // 引数の末尾は update_mode （DU4 は最も高速，GC16 は低速ながら高画質で画像描画向き）
    canvas.pushCanvas(20, 500, UPDATE_MODE_DU4);


    //canvas.pushCanvas(100,300,UPDATE_MODE_DU4);
    delay(500);
}


int TouchBtn(){
    Pos_t Pos;
    Pos.x = 0;
    Pos.y = 0;


    TouchScan(&Pos);

    if( ! Pos.x == 0) {
     Serial.printf("X: %d /  Y: %d \r\n", Pos.x,Pos.y );
      //Btn Check
      if( (Pos.x >btns[0].x ) && (Pos.x < btns[0].x + btns[0].w ) &&
           (Pos.y >btns[0].y ) && (Pos.y < btns[0].y + btns[0].h )   ) 
      {
        //ここに入ってきたらBtn[0]がタッチされたということ

         BtnDraw(0,false);
          //コマンド処理
          Serial.printf("X[0] : ON \r\n");

          //ボタン戻し
         BtnDraw(0,true);
          return(1);
      }

      if( (Pos.x >btns[1].x ) && (Pos.x < btns[1].x + btns[1].w ) &&
           (Pos.y >btns[1].y ) && (Pos.y < btns[1].y + btns[1].h )   ) 
      {
         BtnDraw(1,false);
          Serial.printf("X[1] : ON \r\n");
         BtnDraw(1,true);
          return(2);
      }

      if( (Pos.x >btns[2].x ) && (Pos.x < btns[2].x + btns[2].w ) &&
           (Pos.y >btns[2].y ) && (Pos.y < btns[2].y + btns[2].h )   ) 
      {
           BtnDraw(2,false);
          Serial.printf("X[2] : ON \r\n");
           BtnDraw(2,true);
          return(3);
      }
    }
    return(0);
}

void loop()
{
    int i;

    if( M5.BtnL.wasPressed()) ButtonTest("Prev",2);
    if( M5.BtnP.wasPressed()) ButtonTest("Play/Pause",1);
    if( M5.BtnR.wasPressed()) ButtonTest("Next",3);

    HTTPClient http;

    string CmdStr = VolumioURL;


    i =TouchBtn();
    if(i>0){
          Serial.printf("Btn %d \r\n",i);
        
          switch (i) {
            case 1:
              //http.begin("http://192.168.1.86/api/v1/commands?cmd=prev");
              CmdStr += "/api/v1/commands?cmd=prev";
              http.begin(CmdStr.c_str());
              break;
            case 2:
              CmdStr += "/api/v1/commands?cmd=toggle";
              //http.begin("http://192.168.1.86/api/v1/commands?cmd=toggle");
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
            canvas.drawString(response, 0, 0);
            Serial.println(response);
          } else {
            Serial.println("Error on HTTP request");
          }
          http.end();

          MusicInfo();

    }


    M5.update();
    delay(100);
    
            
}