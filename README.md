# TinySynth
A business card with a digital synthesizer, built with the [ATtiny817](https://www.microchip.com/wwwproducts/en/ATTINY817) microcontroller.

## Features
The business card features my name, contact information, and a digital synthesizer with integrated speakers and keyboard. To use the synthesizer, insert a suitable 3V coin cell battery (CR2016 or CR2032 works), and break off the stylus on the right side of the card. The stylus can then be used to play the keyboard, or to configure the synthesizer with the buttons above the keys.

### Configuration
The synthesizer can be configured using the buttons above the keys. The buttons, from 1 to 8 have the following functions:
  * Oscillator 1 octave: Cycle the octave of oscillator 1 between three settings
  * Oscillator 1 wavefrom: Cycle the waveform of oscillator 1 between sine, square, triangle, and sawtooth
  * Envelope rise time: Toggle slow attack on the envelope
  * Envelope fall time: Toggle slow release on the envelope
  * Oscillator 2 octave / disable: Cycle the octave of oscillator 2 between three settings, or disable oscillator 2
  * Oscillator 2 waveform: Cycle the waveform of oscillator 2 between sine, square, triangle, and sawtooth
  * Oscillator 2 detuning: Cycle the interval of oscillator 2 (above oscillator 1) between unison, third, fifth, and seventh
  * Oscillator 2 synchronize: Enable oscillator 2 reset on each completed period of oscillator 1
    
## Hardware
### Keyboard
The keyboard is designed to be read by the microcontroller with an ADC input. The current keyboard is the third revision of the circuit. 

The first design used a simple voltage divider with an 100 ohm resistor between each key, to generate a unique voltage on each key. The stylus was connected to the ADC input, and was used to sample the voltages. This did not work very well, as a very weak pull-resistor was needed on the stylus, but it was weak enough that just touching it caused disturbance to the voltage.

The second design was more similar to the traditional method used for analog synthesizers. The keys was again connected with 100 ohm resistors, but this time a simple current-mirror circuit was used to drive a constant current through the chain. The stylus was grounded, and when touched to a key it provided a shorter circuit to ground lowering the voltage needed to drive this current. This current was then measured by the ADC. This worked, but was sensitive to changes in supply voltage, and did not give very reliable measurements.

The third and current revision is similar to the second design, but it replaces the constant current driver with a simple resistor. The keyboard forms a simple voltage divider with a fixed top resistor of 10K ohm, and the resistor chain between the keys makes up the bottom resistor. As the sampled voltage is non-linearly related to the bottom resistor, the resistor values betwwen the keys were adjusted to compensate. Some non-linearity was allowed (and is corrected for in the software) to allow use of standard resistor values, and reduce the number of unique BOM items.

### Speakers
The speakers are very thin speakers which I believe is intended to be used in earbuds. They are connected in series, and driven using a speaker-driver amplifying the filtered output signal from the DAC in the microcontroller.

### Battery
The battery holder was designed into the PCB to allow the business card to remain as thin as possible. Some testing was done in advance (by dremeling a 1.6mm PCB to test-fit a battery) but mostly the design was copied from various online inspirations. This design worked really well on the first prototype PCB, so no modification was required, other than to add an indication of the correct polarity of the battery.
