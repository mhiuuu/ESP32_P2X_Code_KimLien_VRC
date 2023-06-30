//8-9 DC phía trái, 10-11 DC bên phải. 8,10 tiến - 9,11 lùi

#include <PS2X_lib.h>
/******************************************************************
 * Cài đặt chân cho thư viện :
 * - Trên mạch Motorshield của VIA Makerbot BANHMI, có header 6 chân
 *   được thiết kế để cắm tay cầm PS2.
 * Sơ đồ chân header và sơ đồ GPIO tương ứng:
 *   MOSI | MISO | GND | 3.3V | CS | CLK
 *    12     13    GND   3.3V   15   14
 ******************************************************************/
#include <PS2X_lib.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define PS2_DAT 12 // MISO
#define PS2_CMD 13 // MOSI
#define PS2_SEL 15 // SS
#define PS2_CLK 14 // SLK

/******************************************************************
 * Lựa chọn chế độ cho tay cầm PS2 :
 *   - pressures = đọc giá trị analog từ các nút bấm
 *   - rumble    = bật/tắt chế độ rung
 ******************************************************************/
#define pressures false
#define rumble false

PS2X ps2x; // khởi tạo class PS2x
bool pressed;

void setup()
{
  Serial.begin(115200);
  Serial.print("Ket noi voi tay cam PS2:");

  int error = -1;
  for (int i = 0; i < 10; i++) // thử kết nối với tay cầm ps2 trong 10 lần
  {
    delay(1000); // đợi 1 giây
    // cài đặt chân và các chế độ: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
    Serial.print(".");
  }

  switch (error) // kiểm tra lỗi nếu sau 10 lần không kết nối được
  {
  case 0:
    Serial.println(" Ket noi tay cam PS2 thanh cong");
    break;
  case 1:
    Serial.println(" LOI: Khong tim thay tay cam, hay kiem tra day ket noi vơi tay cam ");
    break;
  case 2:
    Serial.println(" LOI: khong gui duoc lenh");
    break;
  case 3:
    Serial.println(" LOI: Khong vao duoc Pressures mode ");
    break;
  }
  Wire.begin();
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(1600);
  Wire.setClock(400000);

}
int i = 8;

// DC for movement PIN
#define DC_MOVEMENT_1_PIN 8
#define DC_MOVEMENT_2_PIN 9
#define DC_MOVEMENT_3_PIN 10
#define DC_MOVEMENT_4_PIN 11

// DC for Shooter and Ball-Take-in
#define DC_MOVEMENT_5_PIN 12
#define DC_MOVEMENT_6_PIN 13
#define DC_MOVEMENT_7_PIN 14
#define DC_MOVEMENT_8_PIN 15

void loop()
{
  ps2x.read_gamepad(pressed,0); // gọi hàm để đọc tay điều khiển
  pressed = false;

  // các trả về giá trị TRUE (1) khi nút được giữ
  if (ps2x.Button(PSB_START)) // nếu nút Start được giữ, in ra Serial monitor
    Serial.println("Start is being held");
  if (ps2x.Button(PSB_SELECT)) // nếu nút Select được giữ, in ra Serial monitor
    Serial.println("Select is being held");

  DCdes(pressed);
  if(!pressed) resetMotors();
  delay(30);
}
void DCdes(bool &pressed){
  if (ps2x.Button(PSB_PAD_UP)) // Tiến
  {
    pressed = true;
    pwm.setPWM(DC_MOVEMENT_1_PIN,0,4095);
    pwm.setPWM(DC_MOVEMENT_2_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_3_PIN,0,4095);
    pwm.setPWM(DC_MOVEMENT_4_PIN,0,0);
  }
  if (ps2x.Button(PSB_PAD_RIGHT)) // Rẽ Phải
  {
    Serial.print("Right held this hard: \n");
    pressed = true;
    pwm.setPWM(DC_MOVEMENT_1_PIN,0,4000);
    pwm.setPWM(DC_MOVEMENT_2_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_3_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_4_PIN,0,0);
  }
  if (ps2x.Button(PSB_PAD_LEFT)) // Rẽ Trái
  {
    Serial.print("LEFT held this hard: \n");
    pressed = true;
    pwm.setPWM(DC_MOVEMENT_1_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_2_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_3_PIN,0,4000);
    pwm.setPWM(DC_MOVEMENT_4_PIN,0,0);
  }
  if (ps2x.Button(PSB_PAD_DOWN)) //Lùi
  {
    Serial.print("DOWN held this hard: \n");
    pressed = true;
    pwm.setPWM(DC_MOVEMENT_1_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_2_PIN,0,4095);
    pwm.setPWM(DC_MOVEMENT_3_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_4_PIN,0,4095);
  }

  if (ps2x.Button(PSB_RED)) //Shooter
  {
    Serial.print("Circle just pressed: \n");
    pressed = true;
    pwm.setPWM(12,0,3900);
    pwm.setPWM(13,0,0);
    pwm.setPWM(14,0,0);
    pwm.setPWM(15,0,0);
  }
  if (ps2x.Button(PSB_GREEN)) //Ball-take-in
  {
    Serial.print("Triangle pressed \n");
    pressed = true;
    pwm.setPWM(12,0,0);
    pwm.setPWM(13,0,0);
    pwm.setPWM(14,0,4095);
    pwm.setPWM(15,0,0);
  }

  if (ps2x.Button(PSB_PINK)) //Ball-take-in Reverse
  {
    Serial.print("Triangle pressed \n");
    pressed = true;
    pwm.setPWM(12,0,0);
    pwm.setPWM(13,0,0);
    pwm.setPWM(14,0,0);
    pwm.setPWM(15,0,4095);
  }
}

void resetMotors(){
  pwm.setPWM(8,0,0);
  pwm.setPWM(9,0,0);
  pwm.setPWM(10,0,0);
  pwm.setPWM(11,0,0);
  pwm.setPWM(12,0,0);
  pwm.setPWM(13,0,0);
  pwm.setPWM(14,0,0);
  pwm.setPWM(15,0,0);
}
/**
 * @brief Control the servos and other motors of the robot
 * 
 * @param pressed The pointer to check whether
 *                a button is pressed in the loop
 */
