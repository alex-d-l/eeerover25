# **EEERover Group 25 Project**

---

## Summary

A remotely-controlled rover that can navigate an arena and identify the name and species of lizards that live there.

## Structure

This repo contains the Arduino code and the code for the webpage. The Arduino code can be found in `src/main.cpp` and the code for the webpage is in the `xampp` folder. Specifically, the HTML, CSS and JS files for the webpage can be found in `xampp/htdocs` and are labelled `index.html`, `style.css` and `script.js` respectively.

Note that since the submission of the project report, the main.cpp was improved prior to the demo. In particular, the `infraredRead` function in `main.cpp` now uses a digital pin instead of an analogue pin, and has very similar code to the `readRadio` function. This was done to improve the latency. Code was also added to determine the species from the data from the infrared, radio signals and magnet and display this on the webpage. 

## Movement of Rover and Display of Data

The rover can be controlled using the arrow buttons on the webpage or using the arrow keys on the keyboard. When a button is pressed down, the rover will perform that action for the duration that it is pressed down; when let go, the rover should moving. The STOP button or Spacebar stops the movement of the rover when pressed. 

Data from infrared, radio and ultrasonic signals, as well as information about the polarity of a magnet placed near the respective sensors on the rover are processed and displayed on the webpage. Ultrasonic signals correspond to the 'Name' of a particular lizard; the infrared and radio signals and the magnetic polarity determine the species of a lizard. This is also displayed on the webpage. The data displayed on the webpage is updated every second.
