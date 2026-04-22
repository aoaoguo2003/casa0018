#include <aoaoguo_nano_inferencing.h>
#include <Arduino_LSM9DS1.h>
#include <math.h>

#define CONVERT_G_TO_MS2    9.80665f
#define MAX_ACCEPTED_RANGE  2.0f

// Nano 33 BLE built-in RGB LED pins
#define LEDR 22
#define LEDG 23
#define LEDB 24

static const bool debug_nn = false;
static const float CONFIDENCE_THRESHOLD = 0.50f;

float ei_get_sign(float number) {
    return (number >= 0.0f) ? 1.0f : -1.0f;
}

void allLEDOff() {
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);
}

void allLEDOn() {
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDB, LOW);
}

void showMoveColor(String move) {
    allLEDOff();

    if (move == "rock") {
        digitalWrite(LEDR, LOW);   // red
    }
    else if (move == "paper") {
        digitalWrite(LEDG, LOW);   // green
    }
    else if (move == "scissor" || move == "scissors") {
        digitalWrite(LEDB, LOW);   // blue
    }
}

void blinkResult(int result) {
    int times = 0;

    if (result == -1) {
        times = 1;   // user loses
    }
    else if (result == 0) {
        times = 2;   // draw
    }
    else {
        times = 3;   // user wins
    }

    for (int i = 0; i < times; i++) {
        allLEDOn();
        delay(200);
        allLEDOff();
        delay(250);
    }
}

String randomNanoMove() {
    int r = random(0, 3);
    if (r == 0) return "rock";
    if (r == 1) return "paper";
    return "scissors";
}

int getGameResult(String userMove, String nanoMove) {
    if (userMove == nanoMove) return 0;

    if ((userMove == "rock" && nanoMove == "scissors") ||
        (userMove == "paper" && nanoMove == "rock") ||
        ((userMove == "scissor" || userMove == "scissors") && nanoMove == "paper")) {
        return 1; // user wins
    }

    return -1; // user loses
}

bool isScissorsLabel(String label) {
    return (label == "scissor" || label == "scissors");
}

String normalizeMoveLabel(String label) {
    if (isScissorsLabel(label)) return "scissors";
    return label;
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
    allLEDOff();

    Serial.println("Edge Impulse 9-axis IMU RPS Game");
    Serial.println();

    Serial.print("Model interval (ms): ");
    Serial.println(EI_CLASSIFIER_INTERVAL_MS);
    Serial.print("DSP input frame size: ");
    Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    Serial.print("Raw samples per frame: ");
    Serial.println(EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME);
    Serial.print("Label count: ");
    Serial.println(EI_CLASSIFIER_LABEL_COUNT);

    if (!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");
        while (1);
    }

    randomSeed(analogRead(A0));

    Serial.println("Type 's' to start game.");
}

void loop() {
    if (Serial.available()) {
        char input = Serial.read();

        if (input == 's') {
            Serial.println();
            Serial.println("Get ready...");
            delay(500);

            Serial.println("Go!");

            float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = {0};

            size_t sample_count = 0;
            const size_t axes_per_sample = EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME;

            if (axes_per_sample != 9) {
                Serial.println("Error: this code expects a 9-axis model.");
                Serial.print("But model uses ");
                Serial.print(axes_per_sample);
                Serial.println(" axes per sample.");
                return;
            }

            while (sample_count < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
                int64_t next_tick = (int64_t)micros() + ((int64_t)EI_CLASSIFIER_INTERVAL_MS * 1000);

                float ax, ay, az;
                float gx, gy, gz;
                float mx, my, mz;

                bool acc_ok = IMU.accelerationAvailable();
                bool gyr_ok = IMU.gyroscopeAvailable();
                bool mag_ok = IMU.magneticFieldAvailable();

                if (acc_ok && gyr_ok && mag_ok) {
                    IMU.readAcceleration(ax, ay, az);
                    IMU.readGyroscope(gx, gy, gz);
                    IMU.readMagneticField(mx, my, mz);

                    float acc[3] = {ax, ay, az};

                    for (int i = 0; i < 3; i++) {
                        if (fabs(acc[i]) > MAX_ACCEPTED_RANGE) {
                            acc[i] = ei_get_sign(acc[i]) * MAX_ACCEPTED_RANGE;
                        }
                        acc[i] *= CONVERT_G_TO_MS2;
                    }

                    if (sample_count + 9 <= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
                        buffer[sample_count++] = acc[0];
                        buffer[sample_count++] = acc[1];
                        buffer[sample_count++] = acc[2];
                        buffer[sample_count++] = gx;
                        buffer[sample_count++] = gy;
                        buffer[sample_count++] = gz;
                        buffer[sample_count++] = mx;
                        buffer[sample_count++] = my;
                        buffer[sample_count++] = mz;
                    }
                }

                int64_t wait_time = next_tick - (int64_t)micros();
                if (wait_time > 0) {
                    delayMicroseconds(wait_time);
                }
            }

            Serial.print("Samples collected: ");
            Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE / axes_per_sample);

            signal_t signal;
            int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
            if (err != 0) {
                Serial.print("Signal error: ");
                Serial.println(err);
                Serial.println("Type 's' to test again.");
                return;
            }

            ei_impulse_result_t result = {0};
            err = run_classifier(&signal, &result, debug_nn);
            if (err != EI_IMPULSE_OK) {
                Serial.print("Classifier error: ");
                Serial.println(err);
                Serial.println("Type 's' to test again.");
                return;
            }

            Serial.print("Predictions (DSP: ");
            Serial.print(result.timing.dsp);
            Serial.print(" ms, Classification: ");
            Serial.print(result.timing.classification);
            Serial.println(" ms)");

            size_t best_ix = 0;
            float best_value = result.classification[0].value;

            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                Serial.print(result.classification[ix].label);
                Serial.print(": ");
                Serial.println(result.classification[ix].value, 5);

                if (result.classification[ix].value > best_value) {
                    best_value = result.classification[ix].value;
                    best_ix = ix;
                }
            }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
            Serial.print("Anomaly: ");
            Serial.println(result.anomaly, 5);
#endif

            String userMove = String(result.classification[best_ix].label);
            userMove = normalizeMoveLabel(userMove);

            Serial.println();
            Serial.print("Predicted user move: ");
            Serial.println(userMove);
            Serial.print("Confidence: ");
            Serial.println(best_value, 5);

            if (userMove == "unknown" || best_value < CONFIDENCE_THRESHOLD) {
                Serial.println("Invalid move. Please try again.");
                allLEDOff();
                Serial.println();
                Serial.println("Type 's' to start game.");
                return;
            }

            String nanoMove = randomNanoMove();

            Serial.print("Nano move: ");
            Serial.println(nanoMove);

            // Show Nano move by color
            showMoveColor(nanoMove);
            delay(1000);
            allLEDOff();
            delay(300);

            int gameResult = getGameResult(userMove, nanoMove);

            if (gameResult == 1) {
                Serial.println("Result: You WIN!");
            }
            else if (gameResult == 0) {
                Serial.println("Result: DRAW!");
            }
            else {
                Serial.println("Result: You LOSE!");
            }

            blinkResult(gameResult);
            allLEDOff();

            Serial.println();
            Serial.println("Type 's' to start game.");
        }
    }
}