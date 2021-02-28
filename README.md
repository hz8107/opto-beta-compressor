## Opto-Beta  : Optical compressor with saturator

1. Overview

   <img src="/image/img1.jpg" style="zoom:67%;" />

2. Structure

![](/image/img3.jpg)

3. Description

   - Saturator : tube warm function + tanh (soft overdrive) , 4x oversampling , SSE optimized.

   - LDR(Photoresistor) : led voltage vs resistance ratio lookup table + RC circuit emulated LDR behaviour.

     

4. FAQs

   - Q: Is it a 'zero latency' compressor?
   - A: If you turn 'Sat off' on , the saturator will be bypassed, then it works as 'zero latency' compressor.
   - Q: Where are the 'attack' 'release' parameter?
   - A: In this LDR based compressor , these parameter will determined by input signal.
   - Q: Is it a high quality plugin?
   - A: I'm new to DSP or JUCE , make mistakes is possible. I tried to improve it , test it a lot. 