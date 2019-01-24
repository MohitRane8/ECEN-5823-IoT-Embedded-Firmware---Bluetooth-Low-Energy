Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

**1. How much current does a single LED draw when the output drive is set to "Strong" with the original code?**

A single LED in "Strong" drive is observed to draw 0.48 mA. When the LED is off, the current drawn is 4.42 mA and when the LED is on, the current drawn by the board is 4.90 mA.

**2. After commenting out the standard output drive and uncommenting "Weak" drive, how much current does a single LED draw?**

A single LED in "Weak" drive is observed to draw 0.48 mA. When the LED is off, the current drawn is 4.58 mA and when the LED is on, the current drawn by the board is 5.06 mA.

**3. Is there a meaningful difference in current between the answers for question 1 and 2? Please explain your answer, 
referencing the [Mainboard Schematic](https://www.silabs.com/documents/public/schematic-files/WSTK-Main-BRD4001A-A01-schematic.pdf) and [AEM Accuracy](https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf) section of the user's guide where appropriate.**

There is no meaningful difference. The AEM is accurate within 0.1 mA for currents above 250 μA, so there are no accuracy issues here.
GPIO weak 1mA and alternate function weak 1mA.
GPIO weak 1mA and alternate function strong 10mA.
No difference can be seen because the current drawn by the LED in the gpioDriveStrengthWeakAlternateWeak state is 0.48 mA which is less than 1 mA (Weak state).
**The LED will only draw that much amount of current that it requires to light up.** So even when the LED is in gpioDriveStrengthWeakAlternateStrong state, it will still draw 0.48 mA.

**4. Using the Energy Profiler with "weak" drive LEDs, what is the average current and energy measured with only LED1 turning on in the main loop?**

The average current drawn is 4.90 mA and energy measured is 11.84 μWh with only LED1 turning on in the main loop.

**5. Using the Energy Profiler with "weak" drive LEDs, what is the average current and energy measured with both LED1 and LED0 turning on in the main loop?**

The average current drawn is 5.08 mA and energy measured is 12.39 μWh with both LED0 and LED1 turning on in the main loop.
