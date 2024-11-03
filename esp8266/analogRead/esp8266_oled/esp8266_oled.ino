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


#include <U8g2lib.h>

/* 设置oled屏幕的相关信息 */
//const int I2C_ADDR = 0x3c;  // oled屏幕的I2c地址
#define SDA_PIN 4           // SDA引脚，默认gpio4(D2)
#define SCL_PIN 5           // SCL引脚，默认gpio5(D1)

/* 新建一个oled屏幕对象，需要输入IIC地址，SDA和SCL引脚号 */
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/5, /* data=*/4);  // ESP32 Thing, HW I2C with pin remapping

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

//u8g2初始化
  u8g2.begin();

//允许u8g2输出utf8编码
  u8g2.enableUTF8Print();
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

//u8g2 设置字体, 0.91寸屏幕 字号最大选28号字体,否则输出不完整.
  u8g2.setFont(u8g2_font_logisoso28_tn);

//u8g2 设置文字的显示方向,默认=0,从左到右
  u8g2.setFontDirection(0);

//U8G2 库采用了一种页面缓冲的机制，这个函数是开启新页面绘制循环的第一步
  u8g2.firstPage();
  do {

//u8g2 设置第一个文字的坐标
    u8g2.setCursor(2, 45);

//u8g2 绘制文字    
    u8g2.print(String(weightF));
  } 
  
//u8g2 用于在绘制多页内容时，循环遍历并处理每一页，直到所有页面都被处理完毕  
  while (u8g2.nextPage());

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
