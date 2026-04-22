# Dataset Summary

## Overview

The dataset used in this project was collected using the Arduino Nano 33 BLE Sense onboard 9-axis IMU sensor. It consists of motion data corresponding to hand gestures for a rock-paper-scissors classification task.

The dataset includes four classes:
- rock
- paper
- scissors
- unknown

The "unknown" class was introduced to capture non-standard movements and improve robustness in real-world usage.

---

## Dataset Size

- **Total number of samples:** 200  
- **Training samples per class:** 40  
- **Number of classes:** 4  
- **Testing samples (unseen data):** 50  

---

## Data Structure

Each sample consists of time-series data collected over a fixed window:

- **Window length:** 2000 ms  
- **Sampling frequency:** 62.5 Hz  
- **Sensor inputs (9-axis IMU):**
  - Accelerometer: accX, accY, accZ  
  - Gyroscope: gyrX, gyrY, gyrZ  
  - Magnetometer: magX, magY, magZ  

---

## Data Collection Strategy

### Gesture Classes (rock, paper, scissors)

For each gesture class, data was collected under multiple conditions to improve generalisation:

- Standard hand position  
- Slight rotations (approximately ±90°)  
- Small variations in movement speed  
- Minor variations in starting pose  

This was done to reduce sensitivity to device orientation and user-specific motion patterns.

---

### Unknown Class

The "unknown" class includes a diverse set of non-target inputs:

- Static (no movement)
- Random hand movements
- Incomplete gestures
- Non-standard or ambiguous motions

This class was designed to reduce false positives and improve system robustness in real-world scenarios.

---

## Training and Testing Split

The dataset was divided into:

- **Training set:** Used for model training and validation within Edge Impulse  
- **Testing set:** Held-out unseen data used to evaluate generalisation performance  

The testing dataset was collected separately to better simulate real-world usage conditions and avoid overfitting.

---

## Observations

- Initial models trained on limited and uniform data showed high validation accuracy but poor real-world performance.
- Adding orientation variations significantly improved robustness.
- Introducing the "unknown" class reduced false positives but made the classification task more complex.

---

## Limitations

- Dataset size is relatively small for a multi-class time-series classification problem.
- The unknown class is inherently diverse, making it harder for the model to learn clear boundaries.
- Sensor-based data is sensitive to device orientation, requiring careful data collection to ensure generalisation.

---

## Potential Improvements

- Increase dataset size, especially for the unknown class  
- Include more extreme orientation variations  
- Collect data from multiple users to improve generalisation  
- Explore feature engineering or model architecture improvements  
