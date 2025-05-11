#include <Adafruit_Fingerprint.h>     //指纹识别库
#include <Keypad.h>                   //4*4按键库
#include <LiquidCrystal_I2C.h>        //lcd库

LiquidCrystal_I2C lcd(0x27,16,2);     //定义lcd对象

int flag = 0;          //标记变量，用于控制模式变化
int wait = 0;
int mode = 0; //0：两种都行，or；1：and
int wrong = 0;


//-----------------4*4按键相关设置-----------------------
String pwd;              //记录按键密码
String key = "1357#";      //用户密码key             

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
                        //define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] =
{
    { 'D','#','0','*'},
    { 'C','9','8','7'},
    { 'B','6','5','4'},
    { 'A','3','2','1'}
};
byte rowPins[ROWS] = { 9, 10, 11, 12 }; //connect to the row pinouts of the keypad
byte colPins[COLS] = { 5, 6, 7, 8 }; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

//----------------指纹传感器相关设置-------------------------------------------
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2, 3);     //硬件串口连接定义，端口号2，3
#else
        // On Leonardo/M0/etc, others with hardware serial, use hardware serial!
        // #0 is green wire, #1 is white
#define mySerial Serial1
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);    //定义指纹对象

//---------------程序初始化函数--------------------------------------------
void setup()
{
    lcd.init();           //初始化液晶显示屏
    lcd.backlight();      //打开液晶屏背光

    Serial.begin(9600);   //设置串口通信频率
    while (!Serial) ;      // For Yun/Leo/Micro/Zero/...
    delay(100);
    Serial.println("\n\nAdafruit finger detect test");  //串口显示提示信息

    // set the data rate for the sensor serial port
    finger.begin(57600);
    delay(5);

    if (finger.verifyPassword())
    {                      //如果发现指纹传感器硬件
        Serial.println("Found fingerprint sensor!");
    }
    else
    {                                             //否则提示无硬件设备
        Serial.println("Did not find fingerprint sensor :(");
        while (1) { delay(1); }
    }

    Serial.println(F("Reading sensor parameters"));  //读取指纹传感器参数
    finger.getParameters();
    Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
    Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
    Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
    Serial.print(F("Security level: ")); Serial.println(finger.security_level);
    Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
    Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
    Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

    finger.getTemplateCount();     //获取已录入指纹状态统计

    if (finger.templateCount == 0)
    {    //如果指纹未录入，提醒录入
        Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
    }
    else
    {
        Serial.println("Waiting for valid finger..."); //串口提示等待有效指纹
        Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
    }
}

void getKeypad()
{
    char customKey = customKeypad.getKey();
    wait++;
    if (wait > 25)
    {                //如果按下'*',清屏,清除原来的输入,并提示clear
        lcd.clear();
        pwd = "";
        //Serial.println("hang up");
        //lcd.setCursor(0, 1);
        //lcd.print("clear");
        //delay(500);
        wait = 0;
        //lcd.clear();
    }
    if (customKey)
    {
        wait = 0;
        Serial.println(customKey);
        pwd = pwd + customKey;                 //将输入按键值记录在变量pwd中
        lcd.clear();                       //清屏显示pwd字符串
        lcd.setCursor(0, 0);
        lcd.print(pwd);

        if (customKey == '*')
        {                //如果按下'*',清屏,清除原来的输入,并提示clear
            lcd.clear();
            pwd = "";
            lcd.setCursor(0, 1);
            lcd.print("Clear");
            delay(500);     //一段时间后清屏
            lcd.clear();
        }
        else
        {
            if (customKey == '#')
            {             //没有按下'*'时，如果按下'#',判断密码是否正确，并给出提示，清空原先输入
                if (pwd == key)
                {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print(pwd);
                    lcd.setCursor(0, 1);
                    lcd.print("Correct");
                    Serial.println("correct");
                    wrong = 0;
                    flag = 1;
                    pwd = "";
                }
                else
                {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print(pwd);
                    lcd.setCursor(0, 1);
                    wrong++;
                    lcd.print("Wrong");
                    Serial.println("wrong");
                    Serial.println("have tried for"); Serial.println(wrong);
                    if (wrong > 2)
                    {
                        lcd.setCursor(5, 1);
                        lcd.print(wrong);
                    }
                    if (wrong > 4)
                    {
                        Serial.println("locked");
                        int i = 12;
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Try again after");
                        lcd.setCursor(0, 1);
                        lcd.print("[");
                        lcd.setCursor(3, 1);
                        lcd.print("]s");
                        while (i > 0)
                        {
                            if (i < 10)
                            {
                                lcd.setCursor(1, 1);
                                lcd.print("0");
                                lcd.setCursor(2, 1);
                                lcd.print(i);
                            }
                            else
                            {
                                lcd.setCursor(1, 1);
                                lcd.print(i);
                            }
                            Serial.println(i);
                            delay(1000);
                            i--;
                        }
                        wrong = 3;
                        lcd.clear();
                    }
                    pwd = "";
                }
            }
        }
    }
}


void loop()                     // run over and over again
{
    while (1)
    {
        getKeypad();          //按键密码检测

        getFingerprintID();   //调用采集指纹函数
        delay(10);            //don't ned to run this at full speed.  ----原来设的是50

        if (flag == 1)
        {
            flag = 0;        //检测标记重置为0
            break;
        }
    }
}

//采集指纹函数
uint8_t getFingerprintID()
{
    uint8_t p = finger.getImage();          //获取指纹图片给p
    switch (p)
    {                            //判断p是否获取成功，给出不同的提示
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            //Serial.println("No finger detected");
            return p;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            return p;
        default:
            Serial.println("Unknown error");
            return p;
    }

    // OK success!                                    

    p = finger.image2Tz();                          //将刚才获取的指纹图片进行转换，还是给变量p
    switch (p)
    {                                    //根据p的结果进行判断给出不同的提示
        case FINGERPRINT_OK:
            Serial.println("Image converted");
            break;
        case FINGERPRINT_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FINGERPRINT_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
        case FINGERPRINT_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
        default:
            Serial.println("Unknown error");
            return p;
    }

    // OK converted!
    p = finger.fingerSearch();                      //将转换好之后的指纹图片进行比对搜索，还是给变量p
    if (p == FINGERPRINT_OK)
    {                      //根据p的结果进行判断给出不同的提示,如找到，没找到等，后面都有返回，为什么这边没有？
        Serial.println("Found a print match!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {          //----在这里可以给出lcd未找到的提示
        Serial.println("Did not find a match");
        lcd.clear();                             //在lcd上进行显示提醒指纹匹配
        lcd.setCursor(0, 0);
        lcd.print("FingerPrint#");
        lcd.setCursor(0, 1);
        lcd.print("Wrong");
        wrong++;

        if (wrong > 2)
        {
            lcd.setCursor(5, 1);
            lcd.print(wrong);
        }
        if (wrong > 4)
        {
            Serial.println("locked");
            int i = 12;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Try again after");
            lcd.setCursor(0, 1);
            lcd.print("[");
            lcd.setCursor(3, 1);
            lcd.print("]s");
            while (i > 0)
            {
                if (i < 10)
                {
                    lcd.setCursor(1, 1);
                    lcd.print("0");
                    lcd.setCursor(2, 1);
                    lcd.print(i);
                }
                else
                {
                    lcd.setCursor(1, 1);
                    lcd.print(i);
                }
                Serial.println(i);
                delay(1000);
                i--;
            }
            wrong = 3;
            lcd.clear();
        }

        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    // found a match!  找到匹配指纹      这里应该只有当找到匹配的指纹，才能做到，否则之前的都直接返回了，这里的代码执行不到
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);          //串口给出匹配指纹的相关信息

    lcd.clear();                             //在lcd上进行显示提醒指纹匹配
    lcd.setCursor(0, 0);
    lcd.print("FingerPrint#");
    lcd.setCursor(12, 0);
    lcd.print(finger.fingerID);
    lcd.setCursor(0, 1);
    lcd.print("Correct");
    wrong = 0;
    delay(1000);     //一段时间后清屏
    lcd.clear();
    flag = 1;          //标记置1
    return finger.fingerID;
}

#define mySerial Serial1
