
/*
特别注意! 特别注意! 特别注意!
请使用 millis()做延时除去抖动
不要用delay(),会抖动的

//* 本程序的用途:模拟真实电子秤表头发送特定格式的数据至电脑主板.(使用串口发送的方式,发送指定格式的数据到电脑主板,)
//*
//* 使用的组件: esp8266开发板1个 , 10K旋转电位器1个 , 0.91寸oled屏幕1个
//*
//* 连线方式: 10k电位器L M R 三个脚, L 脚连接GND , M脚连接 A0 , R脚连接 3.3v (逆时针转动数值变小, 顺时针转动数值变大)
//*           0.91寸oled 四个脚, VCC 连接 3.3v , GND连接 GND, SDA 连接 GPIO4 (SDA) , SCL 连接 GPIO5 (SCL)
//*
//* 思路:
//*       通过旋转电位器改变A0引脚的电压值,经过转换公式输出到串口和oled屏幕
//*
//* 方法如下:
//*          1,重新映射A0口的数据范围 ,由默认的12-1023 改为 0-1200
//*            详解:esp8266的A0引脚的位宽是2的10次方,量程为0-1023.最小值设为12 是因为电位器有误差,便宜货. 
//*
//*          2,使用" weightF = map(analogRead(pin_a0), 12, 1024, 0, 1200) * 0.01;" 把获取到的数值转换成 0.00 - 12.00
//*            详解:模拟的量程为 0.00 - 12.00
//*
//*          3,分别从oled屏幕和串口输出转换后的数据
//*             详解:1, 串口输出:在电脑上,称重软件需要获取当前重量. 
//*                 2,  oled屏幕输出:可直观地显示当前传输的数据
//*/


#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//字体文件
#include "Digital_Readout_Thick_V130pt7b.h"

//如果你的 OLED 显示屏没有连接复位引脚或者使用了其他方式进行复位，可以传递-1表示不使用特定的复位引脚。
Adafruit_SSD1306 OLED(128,64,&Wire,-1);


/* 设置oled屏幕的相关信息 */
#define SDA_PIN 4           // SDA引脚，默认gpio4(D2)
#define SCL_PIN 5           // SCL引脚，默认gpio5(D1)

// 模拟引脚A0
const int pin_a0 = A0;                                                                                   

//重量
float weightF = 0.00;

//当前的时间 -上一次的时间 > 预定义的延时
unsigned long ptime = 0;//上一次的时间(毫秒)
const long delay_time = 200;//预定义的延时(毫秒),每隔200毫秒取样一次.


void setup() {
 //初始化串口通讯波特率为9600
  Serial.begin(9600);
  if (!OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));

    //for (;;) 这是一个无限循环。如果初始化失败，程序将进入这个循环，停止继续执行其他代码。这样可以防止程序在显示屏初始化失败的情况下继续运行可能导致错误的操作。
    for (;;);
  }
  OLED.clearDisplay();

  //引用字体文件
  OLED.setFont(&Digital_Readout_Thick_V130pt7b);

  //文字颜色是白色
  OLED.setTextColor(WHITE);
}

void loop() {
  //当前的时间
   unsigned long ctime = millis();

   //如果 当前的时间 -上一次的时间 > 预定义的延时,则获取A0引脚的电压值并转换
   //此步骤为 软件除抖动
  if (ctime - ptime > delay_time){

    //上一次的时间设为当前的时间,为下次判断使用
    ptime = ctime;

    //数据转换
    weightF = map(analogRead(pin_a0), 12, 1024, 0, 1200) * 0.01;


  }

    OLED.clearDisplay();
    OLED.setCursor(5,50);
    OLED.print(weightF);
    OLED.display();


//串口输出当前的重量
  printWeightF(weightF);

}


//数据格式转换
void printWeightF(float number) {
  /*输出格式为反序输出,输出格式为 xx.yyy000=
  例如重量=12.34
  反序输出为"43.21000="
  这是模拟电子秤表头的输出格式

  以下是串口调试器数据解析设置
  {
    重量回调方式: 只读式

    所占字节: 8

    是否反转: 是

    标识符: =

    标识符位置: 0

    整数起始位置: 4

    整数结束位置: 7

    小数起始位置: 1

    小数结束位置: 2
  }
  //*/

  // 原始字符串拼接,至少3个0,多加几个没问题的,重量解析时又会做一次反向读取
  // 如发送的反序数据为43.21000= ,接收的正序数据为 =00012.43
  String str = "=000" + String(number);

  // 获取原始字符串的长度
  int len = str.length();

  // 开始反向取值并输出
  for (int i = len - 1; i >= 0; i--) {
    // 逐个字符输出至串口
    Serial.print(str[i]);
  }
}
