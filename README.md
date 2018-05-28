# Runner
> Cube Runner, an embedded systems project powered by an ATMEGA1284 and Arduino Uno

## Introduction

Cube Runner is a skill-based runner game where you avoid incoming objects (cubes) by strafing left and right. As time goes on, the objects come at you at a faster rate and with less opportunities to dodge. Your current speed will be displayed through an LED bar.

![Cube Runner image](https://zmyaro.com/games/icr/icr.png)

## Components (Pin-out)

### Inputs

1. Joystick
2. Reset button
3. Start button

### Outputs

1. 8x8 LED Matrix
2. LED bar to show speed level

## Complexities

1. Using an LED matrix powered by an Arduino Uno, communicating with ATMEGA1284 microcontroller through USART
2. Using EEPROM to save the high score
3. Using the LED bar to display current speed in real time
