Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements in the "Default" configuration of the profiler to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See [Shared document](https://docs.google.com/document/d/1Ro9G2Nsr_ZXDhBYJ6YyF9CPivb--6UjhHRmVhDGySag/edit?usp=sharing) for instructions.* 

1. What is the average current per period?
   Answer: 4.55 uA
   <br>Screenshot:  
   ![Avg_current_per_period](https://github.com/CU-ECEN-5823/assignment4-lpm-part2-MohitRane8/blob/master/screenshots/assignment-4/Avg_current_per_period.JPG)  

2. What is the average current when the Si7021 is Load Power Management OFF?
   Answer: 3.94 uA
   <br>Screenshot:  
   ![Avg_current_LPM_Off](https://github.com/CU-ECEN-5823/assignment4-lpm-part2-MohitRane8/blob/master/screenshots/assignment-4/Avg_current_LPM_Off.JPG)  

3. What is the average current when the Si7021 is Load Power Management ON?
   Answer: 26.22 uA
   <br>Screenshot:  
   ![Avg_current_LPM_Off](https://github.com/CU-ECEN-5823/assignment4-lpm-part2-MohitRane8/blob/master/screenshots/assignment-4/Avg_current_LPM_On.JPG)  

4. How long is the Si7021 Load Power Management ON for 1 temperature reading?
   Answer: 80 ms
   <br>Screenshot:  
   ![duration_lpm_on](https://github.com/CU-ECEN-5823/assignment4-lpm-part2-MohitRane8/blob/master/screenshots/assignment-4/duration_lpm_on.JPG)  
   

5. What is the total operating time of your design for assignment 4 in hours assuming a 1000mAh supply?
Ans: Average current = 4.55uA
     Total operating time of my design is 183.15 hrs.

6. How has the power consumption performance of your design changed since the previous assignment?
Ans: The power consumption has reduced. The reason behind this is sleeping in EM3 mode for most of the time.

7. Describe how you have tested your code to ensure you are sleeping in EM1 mode during I2C transfers.
Ans: I am using SleepBlockBegin and SleepBlockEnd to block the operation going into EM2 and further.