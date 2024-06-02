#include "Adafruit_PixelDust.h"
#include "LedControl.h"

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"

#include "MPU6050_6Axis_MotionApps20.h"
//#include "MPU6050.h" // not necessary if using MotionApps include file

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

/**************************************************************************/

// MAX7219
#define MAX7219_DOUT      3
#define MAX7219_CS        1
#define MAX7219_CLK       2
#define MAX7219_NUM       1 // numDevices

LedControl lc = LedControl(MAX7219_DOUT, MAX7219_CLK, MAX7219_CS, MAX7219_NUM);

// PixelDust
#define N_GRAINS (3 * 3) ///< Number of grains of sand on 64x64 matrix
#define WIDTH       8 // Display width in pixels
#define HEIGHT      8 // Display height in pixels
#define EXPANSION     1
int nGrains = N_GRAINS; // Runtime grain count (adapts to res)

// Sand object, last 2 args are accelerometer scaling and grain elasticity
Adafruit_PixelDust sand(WIDTH, HEIGHT, N_GRAINS, 32, 32);

// MPU6050

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;
//MPU6050 mpu(0x69); // <-- use for AD0 high

// uncomment "OUTPUT_READABLE_YAWPITCHROLL" if you want to see the yaw/
// pitch/roll angles (in degrees) calculated from the quaternions coming
// from the FIFO. Note this also requires gravity vector calculations.
// Also note that yaw/pitch/roll angles suffer from gimbal lock (for
// more info, see: http://en.wikipedia.org/wiki/Gimbal_lock)
#define OUTPUT_READABLE_YAWPITCHROLL

// #define INTERRUPT_PIN 13  // use pin 2 on Arduino Uno & most boards

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

/**************************************************************************/

void setup_MAX7219(void) {
  byte smiley[] =                               // fill array for opening smiley
  {
    B00111100,
    B01000010,
    B10010101,
    B10100001,
    B10100001,
    B10010101,
    B01000010,
    B00111100
  };

  // put your setup code here, to run once:
  // the MAX72XX is in power-saving mode on startup,
  // we have to do a wakeup call
  lc.shutdown (0,false);
 
  // set the brightness to a medium values
  lc.setIntensity (0,0);                       // 0 = low; 8 = high

  for (int i = 0; i < 8; i++)  
  {
    lc.setRow (0,i,smiley[i]);
  }
  delay(1000);
}

void setup_PixelDust(void) {
  // @brief  Allocates additional memory required by the
  //         Adafruit_PixelDust object before placing elements or
  //         calling iterate().
  if (!sand.begin()) {
    puts("PixelDust init failed");
    Serial.println("PixelDust init failed");
    return;
  }

  // Draw an obstacle for sand to move around
  // for(uint8_t y=0; y<8; y++) {
  //   for(uint8_t x=0; x<8; x++) {
  //     sand.setPixel(x, y);             // Set pixel in the simulation
  //   }
  // }
  sand.randomize(); // Initialize random sand positions
  Serial.println("PixelDust init success");
}

void setup_MPU6050(void) {
  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
    Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif

  // initialize serial communication
  // (115200 chosen because it is required for Teapot Demo output, but it's
  // really up to you depending on your project)
  // Serial.begin(115200);
  // while (!Serial); // wait for Leonardo enumeration, others continue immediately

  // delay(1000);

  // NOTE: 8MHz or slower host processors, like the Teensy @ 3.3V or Arduino
  // Pro Mini running at 3.3V, cannot handle this baud rate reliably due to
  // the baud timing being too misaligned with processor ticks. You must use
  // 38400 or slower in these cases, or use some kind of external separate
  // crystal solution for the UART timer.

  // initialize device
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();
  // pinMode(INTERRUPT_PIN, INPUT);

  // verify connection
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  // wait for ready
  Serial.println(F("\nSend any character to begin DMP programming and demo: "));
  while (Serial.available() && Serial.read()); // empty buffer
  // while (!Serial.available());                 // wait for data
  // while (Serial.available() && Serial.read()); // empty buffer again

  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
    // Calibration Time: generate offsets and calibrate our MPU6050
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    mpu.PrintActiveOffsets();
    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    // // enable Arduino interrupt detection
    // Serial.print(F("Enabling interrupt detection (Arduino external interrupt "));
    // Serial.print(digitalPinToInterrupt(INTERRUPT_PIN));
    // Serial.println(F(")..."));
    // attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
    // mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("start");

  setup_MAX7219();
  setup_PixelDust();
  setup_MPU6050();

  lc.clearDisplay(0);

  Serial.println("done");
}

void loop() {
  // if programming failed, don't try to do anything
  if (!dmpReady) {
    Serial.println("MPU6050 not work");
    return;
  }

    // read a packet from FIFO
    if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { // Get the Latest packet 
        #ifdef OUTPUT_READABLE_YAWPITCHROLL
            // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
            Serial.print("ypr\t");
            Serial.print(ypr[0] * 180/M_PI);
            Serial.print("\t");
            Serial.print(ypr[1] * 180/M_PI);
            Serial.print("\t");
            Serial.println(ypr[2] * 180/M_PI);
        #endif
    }
    
  // Erase old grain positions in pixelBuf[]
  uint8_t     i;
  dimension_t x, y;
  for(i=0; i<N_GRAINS; i++) {
    sand.getPosition(i, &x, &y);
    lc.setLed (0, x*EXPANSION, y*EXPANSION, false); // erase previous ball position
  }

  // Read accelerometer...
  // accel.read();
  // Run one frame of the simulation
  // X & Y axes are flipped around here to match physical mounting
  int16_t ax = (ypr[2] * 180/M_PI);
  int16_t ay = -(ypr[1] * 180/M_PI);
  int16_t az = (ypr[0] * 180/M_PI);
  sand.iterate(ax, ay, az);

  // Draw new grain positions in pixelBuf[]
  for(i=0; i<N_GRAINS; i++) {
    sand.getPosition(i, &x, &y);
    lc.setLed (0, x*EXPANSION, y*EXPANSION, true); // light up new ball position
  }
  delay(10);
}
