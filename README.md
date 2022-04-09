# AutoWatering
Arduino automatic plant watering

# Useful links

https://thecavepearlproject.org/2020/10/27/hacking-a-capacitive-soil-moisture-sensor-for-frequency-output/
https://www.youtube.com/watch?v=IGP38bz-K48&t=306s


# Problems and solutions


## C++17 and abs problems

C++17 support is needed, but there seems to be a problem enabling C++17 platform-raspberrypi (the framework being used).
The `abs` macro is defined in `Arduino.h` which conflicts with with std::abs used in `std::chrono`. Maybe it can be solved by not enabling C++17 for dependencies, but I couldn't find how to do that with PlatformIO.
So, the easiest way for now is to actually edit Arduino.h and comment ou the `abs` define.
