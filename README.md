# SyncroRun

The repository serves to host the code written as part of my bachelor thesis at Department of Mechatronics Engineering, [National University of Sciences and Technology](http://nust.edu.pk), Pakistan. The project developed was a prototype of the US-patent [US 8926475B2](https://patents.google.com/patent/US8926475). 

## Introduction

People using a treadmill find it impractical to view their personal mobile/tablet/e-reader whilst running. The project aimed at developing an eight-bar parallel robotic manipulator that could synchronize the motion of a mobile/tablet/e-reader device mounted on the manipulator with the motion of the face of a person running on a treadmill. The concept revolves around use of a controller to synchronize phase and frequency of a manipulator that traced a fixed figure-8 trajectory with the phase and frequency of user's face (captured through stationary monocular camera), by emulating a sagittal symmetric trajectory of user's face in the coronal plane. The contraption is mounted onto or beside treadmill, and is positioned in front of user at comfortable viewing distance. The system was tested by multiple persons running with different speeds, and successful synchronization was observed to be achieved. It was demonstrated that accuracy of synchronization and time required to synchronize both depended on the speed of treadmill runner, with stable and faster synchronizations being achieved at lower to moderate speeds. 

**Concept Animation**

- [Video](https://youtu.be/csyDfKPXqz8)

**Demonstration Videos**

- [Demo 1](https://www.youtube.com/watch?v=MMvzMPrRD8o)
- [Demo 2](https://www.youtube.com/watch?v=-5uN_vG9Avs)

## Technical Aspects

The custom robotic manipulator was driven by an Arduino microcontroller interfaced with a Qt-based GUI application through serial communication. The Arduino microcontroller overlooked robot motor speed control based on robot sensor values and data from the GUI application. The GUI application was run on a separate computer (or Raspberry Pi), and used the [OpenCV](https://opencv.org/) library to process frames from a stationary camera and make computations regarding user's face motion. 

To initiate process, the user would step on treadmill and look into the camera before starting to run. As soon as the GUI code detected user's face, it would analyze the face trajectory of user whilst running, and serially transmit relevant data to the Arduino microcontroller. The microcontroller would then use this feedback together with robot sensor values to synchronize the position of mounted mobile device with position of face of user. 

### Requirements

The following software and hardware requirements were met when the project was undertaken. 

#### GUI Application

- **Software:** [GCC](https://gcc.gnu.org/), [Qt 5](https://www.qt.io/), [OpenCV 3](https://opencv.org/)
- **Hardware:** Ubuntu 16.04 (x64), Intel Core i7, 8GB RAM

#### Arduino Code

- **Software:** [Arduino IDE](https://www.arduino.cc/en/Main/Software)
- **Hardware:** [Arduino MEGA2560](https://www.arduino.cc/en/Guide/ArduinoMega2560), Arduino USB-to-TTL converter

## Hardware and Dataset

The research was conducted in collaboration with National Taiwan University of Science and Technology, Taiwan, with [Dr. Chyi-Yeu Lin](https://meche.ntust.edu.tw/files/15-1106-49879,c2174-1.php?Lang=en) as the project lead, and is still being carried forward. Therefore, some files like datasets and robot designs can not made public yet. However, please drop me an email if you are interested in any aspect of the project, particularly related to hardware CAD files or datasets generated from experimentation. 
