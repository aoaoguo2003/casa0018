# Final Project



Hints on what should go into this page (note each three of these sections are weighted evenly):

### Definition of problem being solved 
(project overview, research question, data available, outcomes anticipated, application design) 

### Documentation of experiments and results 
(model training results, description of training runs, model architecture choices, visual record of experiments) 

### Critical reflection and learning from experiments 
(observations from experiments, factors incluencing results, potential improvements, weaknesses, feedback from reviews) 



### Note: 
*You can choose what tools you use to write up and document your project - your final submission will be a pdf document being uploaded via Moodle, however we would also expect to see a link through to your GitHub repository where you data, results etc. are documented.*

## Definition of Problem

This project aims to develop a real-time gesture recognition system using a 9-axis IMU sensor on the Arduino Nano 33 BLE Sense. The system classifies user hand movements into rock, paper, and scissors gestures, and integrates this with a simple interactive game.

The key challenge is to achieve robust gesture recognition under real-world conditions, including variations in device orientation and non-standard movements. The system also incorporates an "unknown" class to reduce false positives.

The final system performs real-time inference on-device and provides feedback via an RGB LED interface.

## Experiments and Results

### Version 1: Baseline Model
A three-class model (rock, paper, scissors) was initially trained. While validation accuracy was high, on-device performance was inconsistent, especially under varying orientations.

### Version 2: Adding Unknown Class
An "unknown" class was introduced to capture non-standard movements and reduce false positives. This improved robustness but reduced overall validation accuracy due to increased task complexity.

### Version 3: Orientation Augmentation
Additional data was collected under different device orientations. This improved generalisation and reduced sensitivity to orientation.

### Version 4: Targeted Data Improvement
Additional samples for paper and rock were collected under varied orientations. This reduced confusion between these classes and improved overall accuracy from 75% to 87.5%.

## Reflection

The project highlighted the importance of data quality and distribution over model complexity. A key challenge was the sensitivity of IMU data to device orientation, which significantly affected classification performance.

Introducing an "unknown" class improved robustness by reducing false positives, although it made the classification task more difficult and slightly reduced overall accuracy.

The iterative process demonstrated that improving embedded ML systems requires careful analysis of real-world performance, rather than relying solely on validation metrics.