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

TaskHandle_t Task1;
TaskHandle_t Task2;


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
}

void IRAM_ATTR stepY() {
  if (readPin(dirY_pin)) {
    cntY++;
  } else {
    cntY--;
  }

  if (cntY > resolution) cntY = resolution;
  if (cntY < 0) cntY = 0;
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

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */
  delay(500);

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task2code,   /* Task function. */
    "Task2",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task2,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */
  delay(500);
}

//task for pins
void Task1code( void * pvParameters ) {
  Serial.print("In/Out task running on core ");
  Serial.println(xPortGetCoreID());

  pinMode(reset_pin, INPUT_PULLDOWN);
  pinMode(check_pin, INPUT_PULLDOWN);

  pinMode(stepX_pin, INPUT_PULLDOWN);
  pinMode(dirX_pin, INPUT_PULLDOWN);
  pinMode(stepY_pin, INPUT_PULLDOWN);
  pinMode(dirY_pin, INPUT_PULLDOWN);

  attachInterrupt(stepX_pin, stepX, RISING);
  attachInterrupt(stepY_pin, stepY, RISING);


  for (;;) {
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

    delay(1);
  }
}

//DAC Task
void Task2code( void * pvParameters ) {
  Serial.print("DAC task running on core ");
  Serial.println(xPortGetCoreID());

  initMCP4822();

  for (;;) {
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
  }
}


void loop() {
 
}
