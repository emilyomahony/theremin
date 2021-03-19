# UCSB ECE153B Final Project: Theremin

#### Overview
This project is based on the STM32L476 Discovery Board. It uses the audio jack and various other peripherals to drive a [theremin](https://en.wikipedia.org/wiki/Theremin) with discretized notes. 
#### Audio
I<sup>2</sup>C is used to communicate with the board's CS43L22 audio chip. This is necessary for initial configuration and for changing the volume output. The board's SAI function is used in I<sup>2</sup>S mode to transmit the sound data. The sound data consists of sine wave samples, the frequency of which changes depending on user input. The data is sent from memory to the I<sup>2</sup>S buffer using circular DMA, updating on half transfer complete and full transfer complete. 
#### User Input
Two HC-SR04 ultrasonic sensors are used to detect the distance to the player's hands. The board's PWM function is used to drive these sensors and receive their input. The joystick right button and the LCD on the board are used for selecting a major or minor scale to play with. 
#### Setup
Keil uVision5 is used to build and load this project. Using the project files in the `uv_proj` folder will work if you add everything in the `src` folder into a new source group.
#### See Also
- [Website](https://sites.google.com/view/theremin-project)
- [Video demo](https://youtu.be/tGrLuXMKuoQ)
- [Other projects from the class](https://web.ece.ucsb.edu/~yoga/courses/ECE153B/W2021/proposals.html)

#### References
- STM32L4x6 reference manual
- [chiptune music project](https://github.com/deater/vmw-meter/tree/master/stm32L476/chiptune_cs43l22) (super helpful, thanks Vince!!!)

