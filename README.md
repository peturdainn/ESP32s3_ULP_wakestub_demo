# ESP32s3 ULP and wakestub demo
Demo code for ESP32-s3 to use the ULP for edge based GPIO wakeup to start the wakestub

Demonstrates ULP and wakestub working together  
There are two novelties in this demo code (compared to available sample code):  
- The ULP is halted, meaning no significant extra powerconsumption  
- The ULP wakes up the main controller, loading the wakestub, and going back to deepsleep

Works with the devkit boot button as test