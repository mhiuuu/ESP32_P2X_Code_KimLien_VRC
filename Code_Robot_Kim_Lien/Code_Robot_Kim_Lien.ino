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

void setup() {
  Serial.begin(115200);
  Serial.print("Ket noi voi tay cam PS2:");

  int error = -1;
  for (int i = 0; i < 10; i++) { // thử kết nối với tay cầm ps2 trong 10 lần
    delay(1000); // đợi 1 giây
    // cài đặt chân và các chế độ: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
    Serial.print(".");
  }

  switch (error) {// kiểm tra lỗi nếu sau 10 lần không kết nối được
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

void loop() {
  ps2x.read_gamepad(pressed,0); // call the function to read datas from the ps2 console
  pressed = false;

  //return value TRUE(1) when buttons are held
  if (ps2x.Button(PSB_START)) //if "Start" button is being held, log messeage to Serial monitor
    Serial.println("Start is being held");
  if (ps2x.Button(PSB_SELECT)) //if "Select" button is being held, log messeage to Serial monitor
    Serial.println("Select is being held");
  int shooting = 0, intake = 0;
  DCdes(pressed, shooting, intake);
  if(!pressed) resetMotors();
  delay(30);
}
void DCdes(bool &pressed, int &shooting, int &intake) {
  /******************************************************
  * Code to controll the DCs which is used for running and turning
  ********************************************************/
  if (ps2x.Button(PSB_PAD_UP)) { //Up
    Serial.println("Going up");
    pressed = true;
    pwm.setPWM(8,0,4095);
    pwm.setPWM(9,0,0);
    pwm.setPWM(10,0,0);
    pwm.setPWM(11,0,4095);
  }
  if (ps2x.Button(PSB_PAD_RIGHT)) { //Turn right
    Serial.println("Turning right");
    pressed = true;
    pwm.setPWM(8,0,2000);
    pwm.setPWM(9,0,0);
    pwm.setPWM(10,0,0);
    pwm.setPWM(11,0,0);
  }
  if (ps2x.Button(PSB_PAD_LEFT)) { //Turn left
    Serial.println("Turning left");
    pressed = true;
    pwm.setPWM(8,0,0);
    pwm.setPWM(9,0,0);
    pwm.setPWM(10,0,0);
    pwm.setPWM(11,0,2000);
  }
  if (ps2x.Button(PSB_PAD_DOWN)) { //Down
    Serial.println("Going down");
    pressed = true;
    pwm.setPWM(8,0,0);
    pwm.setPWM(9,0,3000);
    pwm.setPWM(10,0,3000);
    pwm.setPWM(11,0,0);
  }


  /*****************************************************
  * This section is for ball intake system and shooting system!
  ******************************************************/
  if(ps2x.ButtonReleased(PSB_RED)) {
    //If the human player press the O button and release it and the intake system hasn't working
    //The state of intake will be change and the board will know that it has to start auto intake
    //The same logic works when the intake system is working and human player press O
    if(intake == 0) intake = 1;
    else intake = 0; Serial.println("Abort auto intake");

    //If the human player want to start intake system only when they hold O, this will work 
    //However this lead us to another problem, if the human player want to change the state to auto, the intake system will run for a sec!!! Focusing on fixing it
  } else if(ps2x.Button(PSB_RED)) {
    Serial.println("Manual intake");
    pressed = true;
    pwm.setPWM(DC_MOVEMENT_5_PIN,0,4095);
    pwm.setPWM(DC_MOVEMENT_6_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_7_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_8_PIN,0,0);
  }
  if(intake == 1) {
    Serial.println("Intake auto");
    pressed = true;
    pwm.setPWM(DC_MOVEMENT_5_PIN,0,4095);
    pwm.setPWM(DC_MOVEMENT_6_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_7_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_8_PIN,0,0);
  }

  if(ps2x.ButtonReleased(PSB_PINK)) {
    if(shooting == 0) shooting = 1;
    else shooting = 0; Serial.println("Abort auto shooting");
  } else if (ps2x.Button(PSB_PINK)) { //Shooting
    Serial.println("Shooting manualy");
    pressed = true;
    pwm.setPWM(DC_MOVEMENT_5_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_6_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_7_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_8_PIN,0,3900);
  }
  if(shooting == 1) {
    Serial.println("Auto shooting");
    pressed = true;
    pwm.setPWM(DC_MOVEMENT_5_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_6_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_7_PIN,0,0);
    pwm.setPWM(DC_MOVEMENT_8_PIN,0,3900);
  }
}

void resetMotors() {
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
