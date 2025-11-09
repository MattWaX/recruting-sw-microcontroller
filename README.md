# Recruting SW Microcontroller Eagle Application project

This is the project with witch I'm applying to enter the Eagle Team (UniTn).
The task consisted in creating a firmware for a nucleo board that would read
the HALL sensor analog and digital pins and send them thru serial, to plot them
on the host pc. 

## How it works

The program flow is primarily controlled by a Finite State Machine with the
following states: 
- **Init**: Startup some structs used by the FSM
- **Wait Request**: Receive commands from the serial to change the output of
  the Listening state until the on-board button is pressed
- **Listening**: The on-board led is on , the cli is off and the nucleo is
  reading analog and digital data from the HALL sensor, the analog data
  before being sent is processed thru 3 different filters chosen thru the cli,
  if the on-board button is pressed the FSM transition to Pause
- **Pause**: Same as Wait Request but the on-board led is blinking with a
  period of 2000ms and a duty cycle of 50%
- **Warning**: The state is entered when the digital pin, in the Listening 
  state, reads 0 for more than 5 seconds uninterrupted, then it spam `WARNING`
  in the serial
- **Error**: If any error occur in any state the FSM will transition to this 
  state. The on-board led will blink with a period of 400ms and a duty cycle of
  50%, the only way to exit the state is by pressing the reset button on the
  nucleo board

![FSM](./img/fsm.png) 

## How it plots


## The hardware

-   |![STM32 Nucleo Board C031C6](./img/pinout_C031C6.png)|
    |:----------------------------:|
    |*STM32 Nucleo Board C031C6*|
- HALL sensor

### Wiring

| Nuclo Board Pin | HALL Sensor |
|---------------- | ----------- |
| PA4             | D0          |
| PB1             | A0          |
| +3v3            | VCC         |
| GND             | GND         |

