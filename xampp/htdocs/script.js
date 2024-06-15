// This code defines what happens when state of html request changes
// XMLHttpRequest object sends HTTP requests to server and receieves responses from it
var consolidatedRequest = new XMLHttpRequest();
var commandRequest = new XMLHttpRequest();

// Fetch the name every second
setInterval(fetchConsolidatedData, 1000);

// Assign event listeners to handle the responses
consolidatedRequest.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
        var response = JSON.parse(this.responseText);
        document.getElementById("name").innerHTML = response.name;
        document.getElementById("infrared").innerHTML = response.infrared;
        document.getElementById("polarity").innerHTML = response.polarity;
        document.getElementById("radio").innerHTML = response.radio;
    }
};

commandRequest.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
        document.getElementById("response").innerHTML = this.responseText;
    }
};


// Command sent to server in form http://<ip>/command?cmd=x
// For example, if speed = 30, command sent is http://<ip>/command?cmd=SPEED30
function sendCommand(command) {
    commandRequest.open("GET", "http://172.20.10.12/command?cmd=" + command, true); // this is the rover module ip
    commandRequest.send();
}


function fetchConsolidatedData() {
    consolidatedRequest.open("GET", "http://172.20.10.12/consolidated", true);
    consolidatedRequest.send();
}



var moving = {
    forward: false,
    backward: false,
    left: false,
    right: false
};

// Movement arrow functions
function moveForward() {
    if(!moving.forward) {
        moving.forward = true;
        sendCommand('F');
    }
}

function moveBackward() {
    if(!moving.backward) {
        moving.backward = true;
        sendCommand('B');
    }
}

function moveLeft() {
    if(!moving.left) {
        moving.left = true;
        sendCommand('L');
    }
}

function moveRight() {
    if(!moving.right) {
        moving.right = true;
        sendCommand('R');
    }
}

function stop() {
    moving.forward = false;
    moving.backward = false;
    moving.left = false;
    moving.right = false;
    sendCommand('S');
}

// Speed sent to server in the form 'SPEEDx' e.g. 'SPEED30' if speed = 30
function speed(speed) {
    sendCommand('SPEED' + speed);
}


// This code is for the speed slider
var slider = document.getElementById("myRange");
var output = document.getElementById("value");

output.innerHTML = slider.value;

slider.oninput = function() {
    output.innerHTML = this.value;
    speed(this.value);
}

// Colour of slider changes as it moves from 0 to 100
slider.addEventListener("mousemove", updateBackground);

function updateBackground(){
    var x = slider.value;
    var color = 'linear-gradient(90deg, grey ' + x + '%, #eee ' + x + '%)';
    slider.style.background = color;
}

function keyDown(event) {
    switch(event.key) {
        case 'ArrowUp':
            event.preventDefault();
            moveForward();
            break;
        case 'ArrowDown':
            event.preventDefault();
            moveBackward();
            break;
        case 'ArrowLeft':
            event.preventDefault();
            moveLeft();
            break;
        case 'ArrowRight':
            event.preventDefault();
            moveRight();
            break;
        case ' ':
            event.preventDefault();
            stop();
            break;
    }
}

function keyUp(event) {
    switch(event.key) {
        case 'ArrowUp':
        case 'ArrowDown':
        case 'ArrowLeft':
        case 'ArrowRight':
            event.preventDefault();
            stop();
            break;
    }
}

document.addEventListener('keydown', keyDown);
document.addEventListener('keyup', keyUp);