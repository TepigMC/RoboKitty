RoboKitty
=========

Arduino UNO codebase for multiple modules that will come together to make an interactive robotic kitty. <br>

Current Modules/Features

<table>
  <tr>
    <td>Module</td>
    <td>Hardware</td>
    <td>Description</td>
  </tr>
  <tr>
    <td>EightBitTunes</td>
    <td>1 Piezo Buzzer, SD Card Shield, SD Card</td>
    <td>
      Plays 8bit tunes that are loaded as .txt files on to a SD Card, following the ABC Notation format.<br>
      Some sample tunes are provided in txt file format, as well as some java scripts that can help convert manually transposed songs in to proper frequency/duration format.<br>
    </td>
  </tr>
  <tr>
    <td>BatteryMonitor</td>
    <td>10 LEDs, 1 Battery</td>
    <td>
      Displays the remaining charge of the attached battery in increments of 10%, with each of the 10 LEDs corresponding to a single increment range.  All LEDs up to calculated remaining percent will display a solid colour, while the LED representing the current percent blinks.<br>
      Example: At 56% remaining charge, LEDs 1-5 will light up (they represent the range from 0%-40%), and LED 5 will be blinking since 56% lies in the 50%-60% range.
    </td>
  </tr>
</table>
