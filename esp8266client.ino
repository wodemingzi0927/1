#include <ESP8266WiFi.h>
#include <DHT.h>

#define led_1 4      // 显示WiFi是否连接的小灯 (D2)
#define led_2 5      // 显示环境参数报警状态的小灯 (D1)
#define DHTPIN 2     // DHT11数据引脚连接到GPIO2 (D4)
#define DHTTYPE DHT11   // 使用DHT11传感器

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "xxxx";  
const char* password = "xxxx";  
const char* serverIP = "xxxx"; // 服务器IP地址  
const int serverPort = xxxx;          // 服务器端口号

WiFiClient client; 

void setup() {
  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
  
  // 初始化串口通信
  Serial.begin(9600);
  Serial.println();
  Serial.println("系统启动中...");
  
  // 初始化DHT传感器
  dht.begin();
  
  // 连接WiFi
  Serial.print("正在连接到: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  // 显示WiFi连接过程
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    // LED闪烁表示正在连接
    for(int i = 0; i < 10; i++) {
      digitalWrite(led_1, LOW);
      delay(50);
      digitalWrite(led_1, HIGH);
      delay(50);
    }
  }
  
  // WiFi连接成功
  Serial.println();
  Serial.println("WiFi已连接");
  digitalWrite(led_1, HIGH);  // 点亮LED表示WiFi已连接
  Serial.print("IP地址: ");
  Serial.println(WiFi.localIP());
  
  // 连接服务器
  Serial.print("正在连接到服务器: ");
  Serial.print(serverIP);
  Serial.print(":");
  Serial.println(serverPort);
  
  while (!client.connect(serverIP, serverPort)) {
    Serial.println("连接失败，重试中...");
    // LED闪烁表示正在尝试连接服务器
    for(int i = 0; i < 10; i++) {
      digitalWrite(led_2, LOW);
      delay(50);
      digitalWrite(led_2, HIGH);
      delay(50);
    }
    delay(1000);
  }
  
  // 服务器连接成功
  Serial.println("已连接到服务器");
  // 不再使用led_2指示服务器连接状态，因此不点亮该LED
  digitalWrite(led_2, LOW);
}

void loop() {
  // 读取温湿度数据
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // 检查读取是否成功
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("读取DHT传感器失败!");
    digitalWrite(led_2, HIGH);  // 读取失败时也亮灯报警
    delay(2000);  // 短暂延迟后重试
    return;
  }

  // 打印温湿度数据到串口
  Serial.print("温度: ");
  Serial.print(temperature);
  Serial.print(" °C, 湿度: ");
  Serial.print(humidity);
  Serial.println(" %");

  // 检查服务器连接状态
  if (!client.connected()) {
    Serial.println("服务器连接已断开，正在重新连接...");
    
    // 尝试重新连接服务器
    if (client.connect(serverIP, serverPort)) {
      Serial.println("重新连接服务器成功");
    } else {
      Serial.println("重新连接失败");
      // 服务器连接失败时亮灯报警
      digitalWrite(led_2, HIGH);
      return;  // 连接失败，跳过本次数据发送
    }
  }

  // 检查温湿度是否超出正常范围
  bool isAlarm = false;
  if (temperature > 25.0 || temperature < 15.0) {
    isAlarm = true;
    Serial.println("警告: 温度超出正常范围!");
  }
  if (humidity > 55.0 || humidity < 45.0) {
    isAlarm = true;
    Serial.println("警告: 湿度超出正常范围!");
  }
  
  // 控制报警LED
  digitalWrite(led_2, isAlarm ? HIGH : LOW);

  // 准备数据并发送到服务器
  char str[50];
  sprintf(str, "%s,%.2f,%.2f,%d","esp8266_001" ,temperature, humidity, isAlarm);
  client.println(str);
  Serial.print("已发送数据: ");
  Serial.println(str);
  
  // 等待2分钟后再次读取和发送数据
  delay(120000);
}    