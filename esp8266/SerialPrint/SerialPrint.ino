/*
本程序为 模拟电子秤输出
//*/

/**
*                             _ooOoo_
*                            o8888888o
*                            88" . "88
*                            (| -_- |)
*                            O\  =  /O
*                         ____/`---'\____
*                       .'  \\|     |//  `.
*                      /  \\|||  :  |||//  \
*                     /  _||||| -:- |||||-  \
*                     |   | \\\  -  /// |   |
*                     | \_|  ''\---/''  |   |
*                     \  .-\__  `-`  ___/-. /
*                   ___`. .'  /--.--\  `. . __
*                ."" '<  `.___\_<|>_/___.'  >'"".
*               | | :  `- \`.;`\ _ /`;.`/ - ` : | |
*               \  \ `-.   \_ __\ /__ _/   .-` /  /
*          ======`-.____`-.___\_____/___.-`____.-'======
*                             `=---='
*          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*                     佛祖保佑        永无BUG
//*/

const byte pin2 = 2;   // 重量归零
const byte pin4 = 4;   // 重量增加
const byte pin5 = 5;   // 重量减少
const byte pin13 = 13; // 增加时亮灯
const byte pin15 = 15; // 减少时亮灯

byte pin2_state = 0;
byte pin4_state = 0;
byte pin5_state = 0;

float weightF = 0.00;

void setup()
{
  // put your setup code here, to run once:
  // 设置串口波特率为9600
  Serial.begin(9600);

  // 高电平=3.3VDC 低电平=0VDC
  // 以下引脚低电平触发,默认全部设为高电平
  // 以下引脚低电平触发,默认全部设为高电平
  // 以下引脚低电平触发,默认全部设为高电平
  pinMode(pin2, INPUT);        // 设置为输入模式,此引脚默认就是高电平,不需要用INPUT_PULLUP人为的"上拉"
  pinMode(pin4, INPUT_PULLUP); // 设置为输入模式,电压默认为低电平,使用内部"上拉"电阻,人为设置成高电平.
  pinMode(pin5, INPUT_PULLUP); // 设置为输入模式,电压默认为低电平,使用内部"上拉"电阻,人为设置成高电平.
  pinMode(pin13, OUTPUT);
  pinMode(pin15, OUTPUT);
}

void loop()
{
  // put your main code here, to run repeatedly:

  // 循环显示当前的重量
  printWeightF(weightF);

  pin2_state = digitalRead(pin2); // 循环获取GPIO2脚的电平状态.HIGH OR LOW 对应为 1 或 0
  pin4_state = digitalRead(pin4); // 循环获取GPIO4脚的电平状态.HIGH OR LOW 对应为 1 或 0
  pin5_state = digitalRead(pin5); // 循环获取GPIO5脚的电平状态.HIGH OR LOW 对应为 1 或 0

  // 归零
  //  pin2(GPIO2)电压为低电平(0v)时,重量数字=0.00
  if (pin2_state == LOW)
  {
    weightF = 0.00;
    printWeightF(weightF);
  }

  // 增加
  //  pin5(GPIO5)电压为低电平(0v)时,重量数字增加0.01
  if (pin5_state == LOW)
  {
    //
    weightF += 0.01;
    printWeightF(weightF);
    digitalWrite(pin13, HIGH);
  }
  else
  {
    digitalWrite(pin13, LOW);
  }

  // 减少
  //  pin4(GPIO4)电压为低电平(0v)并且重量数字大于0,重量数字减少0.1
  if (pin4_state == LOW)
  {
    weightF -= 0.01;

    if (weightF <= 0)
    {
      weightF = 0.00;
    }
    printWeightF(weightF);
    digitalWrite(pin15, HIGH);
  }
  else
  {
    digitalWrite(pin15, LOW);
  }

  // 延时100毫秒,输出下一个状态的值
  delay(100);
}

// 单精度数字(float)转字符串输出
void printWeightF(float number)
{
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
  for (int i = len - 1; i >= 0; i--)
  {
    // 逐个字符输出至串口
    Serial.print(str[i]);
  }
}
