#include <SPI.h>

#define DAC1 25
#define DAC2 26
#define reset_pin 13
#define check_pin 27

#define stepX_pin 33
#define dirX_pin 32
#define stepY_pin 16
#define dirY_pin 17

int resolution = 4096;
int zero = resolution / 2;

int cntX = zero;
int cntY = zero;

int cntX_OLD = -1;
int cntY_OLD = -1;

const int GAIN_2 = 0x0;
const int DAC_X = 0;
const int DAC_Y = 1;

bool readPin(byte pin)
{
  if (pin > 31)
  {
    if ((REG_READ(GPIO_IN1_REG) & (1 << pin - 32)) > 0) return true;
    return false;
  } else
  {
    if ((REG_READ(GPIO_IN_REG) & (1 << pin)) > 0) return true;
    return false;
  }

}

void setPin(byte pin)
{
  REG_WRITE (GPIO_OUT_W1TS_REG, (1 << pin));
}

void clearPin(byte pin)
{
  REG_WRITE (GPIO_OUT_W1TC_REG, (1 << pin));
}


void IRAM_ATTR stepX() {
  if (!readPin(dirX_pin)) {
    cntX++;
  } else {
    cntX--;
  }

  if (cntX > resolution) cntX = resolution;
  if (cntX < 0) cntX = 0;

  //setDAC(DAC_X, cntX);
}

void IRAM_ATTR stepY() {
  if (readPin(dirY_pin)) {
    cntY++;
  } else {
    cntY--;
  }

  if (cntY > resolution) cntY = resolution;
  if (cntY < 0) cntY = 0;

  //setDAC(DAC_Y, cntY);
}


void initMCP4822() {
  pinMode(SS, OUTPUT);
  SPI.begin();
  //SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
}


void setDAC(byte channel, unsigned int val) {
  byte lowByte = val & 0xff;
  byte highByte = ((val >> 8) & 0xff) | channel << 7 | GAIN_2 << 5 | 1 << 4;

  clearPin(SS);
  //digitalWrite(SS, LOW);
  SPI.transfer(highByte);
  SPI.transfer(lowByte);
  //digitalWrite(SS, HIGH);
  setPin(SS);
}




void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting..");

  pinMode(reset_pin, INPUT_PULLDOWN);
  pinMode(check_pin, INPUT_PULLDOWN);

  pinMode(stepX_pin, INPUT_PULLDOWN);
  pinMode(dirX_pin, INPUT_PULLDOWN);
  pinMode(stepY_pin, INPUT_PULLDOWN);
  pinMode(dirY_pin, INPUT_PULLDOWN);

  attachInterrupt(stepX_pin, stepX, RISING);
  attachInterrupt(stepY_pin, stepY, RISING);

  Serial.print("MOSI: ");
  Serial.println(MOSI);
  Serial.print("MISO: ");
  Serial.println(MISO);
  Serial.print("SCK: ");
  Serial.println(SCK);
  Serial.print("SS: ");
  Serial.println(SS);

  initMCP4822();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (readPin(reset_pin)) {
    Serial.println("RESET ZERO");
    cntX = zero;
    cntY = zero;
  }

  if (readPin(check_pin)) {
    Serial.print("X: ");
    Serial.println(cntX);
    Serial.print("Y: ");
    Serial.println(cntY);
  }


  if (cntX != cntX_OLD)
  {
    setDAC(DAC_X, cntX);
  }

  if (cntY != cntY_OLD)
  {
    setDAC(DAC_Y, cntY);
  }

  cntX_OLD = cntX;
  cntY_OLD = cntY;


  /*
    for (int i = 0; i < resolution; i++) {
      //setOutput(0, GAIN_2, 1, i);
      //setADC_X(i);
      setDAC(DAC_X, i);
      setDAC(DAC_Y, i);
      //setADC_Y(i);
     // delay(1);
    }
  */

  /*

    if (steppedX) {
    Serial.println(cntX);
    steppedX = false;
    }

    if (steppedY) {
    Serial.println(cntY);
    steppedY = false;
    }
  */
}
