#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <WiFiUdp.h>

const char *ssid = "ye_X20";
const char *password = "yzh12345";

WiFiServer server(80);

WiFiUDP udp;
const int UDP_PORT = 6454; // Art-Net 默认端口

#define BUTTON_PIN 16 // D0
// WS2812 配置
#define NUM_LEDS 64 // LED 数量
#define DATA_PIN 4  // D2 WS2812 数据引脚
CRGB ledColors[NUM_LEDS];

// Art-Net 配置
const int UNIVERSE = 0;       // Art-Net Universe
const int DMX_CHANNELS = 512; // DMX 通道数

// 初始化Wi-Fi连接
void setupWiFi() {
  Serial.println();
  Serial.print("连接到 ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // 打印本地IP地址
  Serial.println("");
  Serial.println("WiFi已连接");
  Serial.println("IP地址: ");
  Serial.println(WiFi.localIP());
}

void setupServer() {
  // 初始化TCP服务器
  server.begin();
  Serial.println("TCP服务器已启动");
  // 初始化UDP服务器
  udp.begin(UDP_PORT);
  Serial.println("UDP服务器已启动");
}

// 初始化 FastLED
void setupFastLED() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(ledColors, NUM_LEDS);
  FastLED.setBrightness(50); // 设置亮度（0-255）
  FastLED.clear();
  FastLED.show();
}

void handleTCPClient() {
  WiFiClient client = server.accept();
  if (!client) {
    return;
  }
  if (client) {
    Serial.println("新客户端连接");
    // 等待客户端发送数据
    while (client.connected()) {
      if (client.available()) {
        // 读取客户端请求
        String request = client.readStringUntil('\r');
        Serial.println(request);
        client.flush();

        // 发送HTTP响应
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        // client.println("Connection: close"); // 告诉浏览器连接即将关闭
        client.println(); // 空行表示头部结束
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        client.println("<head><title>ESP8266 Web Server</title></head>");
        client.println("<body>");
        client.println("<h1>LED状态</h1>");
        client.println("<table border='1'>");

        // 显示LED状态
        for (int i = 0; i < NUM_LEDS; i++) {
          if (i % 8 == 0) {
            client.println("<tr>");
          }
          client.print("<td style='background-color:rgb(");
          client.print(ledColors[i].r);
          client.print(",");
          client.print(ledColors[i].g);
          client.print(",");
          client.print(ledColors[i].b);
          client.println(");'>&nbsp;</td>");
          if (i % 8 == 7) {
            client.println("</tr>");
          }
        }

        client.println("</table>");
        client.println("</body>");
        client.println("</html>");

        // 添加短暂延迟，确保数据发送完成
        delay(10);
        break;
      }
    }
    // 关闭连接
    client.stop();
    Serial.println("客户端断开连接");
  }
}

// 解析 Art-Net 数据包并更新 LED
void parseArtNetPacket(uint8_t *packet, int packetSize) {
  if (strncmp((char *)packet, "Art-Net", 7) != 0) {
    return;
  }
  // 检查 OpCode
  uint16_t opCode = packet[8] | (packet[9] << 8);
  if (opCode != 0x5000) {
    return; // 不是 DMX 数据包
  }
  // 检查 Universe
  uint16_t universe = packet[14] | (packet[15] << 8);
  if (universe != UNIVERSE) {
    return; // 不是目标 Universe
  }
  // 提取 DMX 数据
  uint16_t dmxLength = packet[16] | (packet[17] << 8);
  uint8_t *dmxData = packet + 18;
  // 更新 WS2812 LED
  for (int i = 0; i < NUM_LEDS; i++) {
    int dmxIndex = i * 3; // 每个 LED 占 3 个 DMX 通道（RGB）
    if (dmxIndex + 2 < dmxLength) {
      ledColors[i].r = dmxData[dmxIndex];
      ledColors[i].g = dmxData[dmxIndex + 1];
      ledColors[i].b = dmxData[dmxIndex + 2];
    }
  }

  FastLED.show(); // 更新 LED 显示
}

// 处理UDP数据包
void handleUDPPacket() {
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    Serial.println("收到UDP数据包");
    uint8_t packet[packetSize];
    udp.read(packet, packetSize);

    // 解析 Art-Net
    parseArtNetPacket(packet, packetSize);

    // 打印
    Serial.print("数据包内容: ");
    for (int i = 0; i < packetSize; i++) {
      Serial.print(packet[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  setupServer();
  setupFastLED();
}

void loop() {
  // handleTCPClient();
  handleUDPPacket();
  delay(50);
  FastLED.show(); // 更新 LED 显示
}
