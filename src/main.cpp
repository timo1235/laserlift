
#include <includes.h>

// Configuration
#define STEPS_PER_ROTATION 800
// How far does the spindle move with one turn around - in mm
#define SPINDLE_PITCH 4
// Max speed in mm/s
#define MAX_SPEED_MM_S 10

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/22, /* data=*/21, /* reset=*/U8X8_PIN_NONE);
#define FONT_B8PX u8g2_font_helvB08_tr
#define FONT_B10PX u8g2_font_helvB10_tr
#define FONT_B12PX u8g2_font_helvB12_tr
#define FONT_B14PX u8g2_font_helvB14_tr
#define FONT_B18PX u8g2_font_helvB18_tr
#define FONT_B24PX u8g2_font_helvB24_tr

RotaryEncoder encoder(ENCODER_A, ENCODER_B);

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

uint32_t lastPrint = 0;
int32_t targetPosition = 0;
uint32_t lastEncoderPosition = 0;

void setup() {
    Serial.begin(115200);
    Serial.print("OCS2 ESP32 Firmware version: ");
    Serial.println(FIRMWARE_VERSION);

    engine.init();
    stepper = engine.stepperConnectToPin(STEP_PIN);
    if (stepper) {
        stepper->setDirectionPin(DIR_PIN);
        stepper->setEnablePin(ENABLE_PIN);
        stepper->setAutoEnable(true);

        // Set initial speed and acceleration
        // Calculate the mm per step
        float mmPerStep = (float) SPINDLE_PITCH / STEPS_PER_ROTATION;
        // Convert the desired speed in mm/s to Hz (steps per second)
        float speedInHz = (float) MAX_SPEED_MM_S / mmPerStep;
        stepper->setSpeedInHz(speedInHz);
        stepper->setAcceleration(20000);
    }

    u8g2.begin();
    // Task for display
    xTaskCreatePinnedToCore(
        [](void *parameter) {
        for (;;) {
            u8g2.clearBuffer();
            u8g2.setFont(FONT_B18PX);
            u8g2.drawStr(10, 20, "LaserLift");
            u8g2.setFont(FONT_B8PX);
            u8g2.drawStr(10, 32, "Target Distance(mm):");
            u8g2.setFont(FONT_B24PX);
            u8g2.setCursor(40, 64);
            int32_t currentSteps = stepper->getCurrentPosition();
            int32_t distanceSteps = targetPosition - currentSteps;
            float stepsPerMM = (float) STEPS_PER_ROTATION / SPINDLE_PITCH;   // Ensure float division
            float distanceMM = distanceSteps / stepsPerMM;                   // Calculate distance in mm
            u8g2.print(distanceMM, 1);                                       // Display with one decimal place
            u8g2.sendBuffer();
            vTaskDelay(50);
        }
    },
        "Display Task", 2048, nullptr, DEFAULT_TASK_PRIORITY, NULL, DEFAULT_TASK_CPU);
    // Task for steppers
    xTaskCreatePinnedToCore(
        [](void *parameter) {
        for (;;) {
            // Read encoder position
            long encoderPosition = encoder.getPosition();
            // Get delta for encoder position and add multiplicator for fast movement
            long delta = encoderPosition - lastEncoderPosition;
            if (delta != 0) {
                uint16_t multiplicator = 1;
                long absDelta = abs(delta);   // Calculate the absolute value of delta
                // Adjust multiplicator based on the magnitude of delta
                if (absDelta > 2) multiplicator = map(absDelta, 0, 25, 0, 100);

                targetPosition += delta * multiplicator;
                lastEncoderPosition = encoderPosition;
            }

            // Move the stepper motor
            stepper->moveTo(targetPosition);
            vTaskDelay(50);
        }
    },
        "Stepper Task", 2048, nullptr, DEFAULT_TASK_PRIORITY, NULL, DEFAULT_TASK_CPU);

    // start config manager first - this loads all configuration
    // configManager.setup();
}

void loop() {
    encoder.tick();

    // Serial.println(targetPosition);

    // Print encoder position periodically
    if (millis() - lastPrint > 1000) {
        lastPrint = millis();
        Serial.print("Stepper position: ");
        Serial.print(stepper->getCurrentPosition());
        Serial.print(" | Encoder position: ");
        Serial.print(lastEncoderPosition);
        Serial.print(" | target position: ");
        Serial.println(targetPosition);
    }
}
