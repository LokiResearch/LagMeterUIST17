# Arduino timestamps

* Use the [micros()](https://www.arduino.cc/en/Reference/Micros) function with a resolution of 4 microseconds. **Goes back to zero each 70 minutes**

``` Returns the number of microseconds since the Arduino board began running the current program. This number will overflow (go back to zero), after approximately 70 minutes. On 16 MHz Arduino boards (e.g. Duemilanove and Nano), this function has a resolution of four microseconds (i.e. the value returned is always a multiple of four). On 8 MHz Arduino boards (e.g. the LilyPad), this function has a resolution of eight microseconds. ```

# Data logged by TurboHID callback

```1488280502215013244 leonardo 63 0 0 80 2 67 13 108 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0```

63 is the number of bytes received from the Leonardo<br>
2 following bytes are always 0<br>
80 ('P') means piezo data, 68 ('D') means photo diode data<br>
the 4 subsequent bytes correspond to the Arduino timestamp on 32 bits. 2 67 13 108 corresponds to 37948780 microseconds: (2 << 24) + (67 << 16) + (13 << 8) + 108
