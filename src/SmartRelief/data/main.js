/**
 * @author Marco Hirsbrunner
 * @version 1
 * @license MIT
 */

var webSocketOpen = false;
var connection;

$(document).ready(() => {
    initializeWebSocket();
    fillMainNav();
    createNavs();
    initalizeSidebarToggles();
    setInterval(() => {
        console.log("requesting update");
        connection.send("u");
    }, 3000)
})

function closeAccessPoint() {
    alert("SmartRelief Netzwerk geschlossen, um es wieder herzustellen starten Sie bitte das Relief neu");
    connection.send("x");
}

/**
 * Turns off all LED (Sends the message to the ESP8266) 
 * Fires when clearLED is pressed in main Navigation
 */
function clearLED() {

    var sendString = "0,0,0,";
    for (var i = 0; i < groups.length; i++) {
        for (var j = 0; j < groups[i][2].length; j++) {
            sendString += groups[i][2][j].toString() + ",";
        }
    }
    counter.innerHTML = "0";
    send(sendString);
    console.log("turn off all led");
}

function activateLedGroup(index) {
    send(groups[index][1] + "," + groups[index][2]);
}

function toggleLED(led) {
    if (document.getElementById(`ledInput-${led}`).checked === true) {
        send(getColors(led) + ',' + led);
    } else {
        send(`0,0,0,${led}`);
    }
}

/**
 * returns the Colors for requested LED
 * 
 * @param {any} number LED Number
 * @returns Color values for the LED
 */
function getColors(number) {
    for (var i = 0; i < groups.length; i++) {
        for (var j = 0; j < groups[i][2].length; j++) {
            if (number == groups[i][2][j]) {
                return groups[i][1];
            }
        }
    }
    console.log("Couldnt Get Color Value, returned default (255,255,255");
    return [255, 255, 255];
}

/**
 * Controls what happens after click on Element (Look bindClick() and setEventListeners())
 * Fires when LED Control part is clicked (Switch or Text next by it)
 * 
 * @param {any} number valid ID of a LED Control Element
 */
function led(number) {
    var numString = number.toString();
    if (document.getElementById(numString).checked === true) {
        send(getColors(number) + "," + numString);
    } else {
        send("0,0,0," + numString);
    }
}

function initalizeSidebarToggles() {

    $('.connectSidebarCollapse').on('click', function() {
        console.log("switching connection sidebar");
        $('#connectSidebar').toggleClass('active');
        $('#led-wrapper').toggleClass('active');
    });
}

function toggleNav(index) {
    console.log("toggeling navigation " + index);
    if ($(`#ledPointsWrapper-${index}`).hasClass('active')) {
        $("#ledGroupNamesList").addClass('active');
        $(`#ledPointsWrapper-${index}`).removeClass('active');
    } else {
        $(`#ledPointsWrapper-${index}`).addClass('active');
        $("#ledGroupNamesList").removeClass('active');
    }

}

/*-------------------------------------------------------------------------------------------------------------------
 *------------------------------------------<Web Socket>--------------------------------------------------------------
 *------------------------------------------------------------------------------------------------------------------*/

/**
 *  Initialize the WebSocket and set WebSocket Handerls
 * 
 */
function initializeWebSocket() {
    try {
        connection = new WebSocket('ws://' + location.host + ':81', ['arduino']);
        if (location.host != "") {
            webSocketOpen = true;
            console.log("WebSocket connected");
            connection.onclose = function() {
                alert("WebSocket Verbindung geschlossen");
                webSocketOpen = false;
            };
            connection.onerror = function() {
                alert("Es geschah ein Fehler mit der Verbindung bitte versuchen Sie das Relief neu zu starten und anschliessend die Verbindugn neu aufzubauen.");
                webSocketOpen = false;
            };
            connection.onmessage = function(e) {
                handleWebSocketData(e.data);
            };
        } else { console.log("WebSocket not connected"); }
    } catch (e) {
        console.log(e);
    }
}


/**
 * Handels WebSocketData
 * Only refreshes counter and turns on/off switches
 * 
 * @param {any} data JSON Object as String with Information about LED States
 */
function handleWebSocketData(data) {
    console.log(`recieved data from websocket: ${data} `);
    var led = JSON.parse(data);
    var count = 0;
    for (let i = 0; i < Object.keys(points).length; i++) {
        let ledNumber = Object.keys(points)[i];
        if (led[ledNumber] == "1") {
            try {
                document.getElementById(`ledInput-${ledNumber}`).checked = true;
                count++;
            } catch (e) {
                // console.warn(e);
            }
        } else {
            try {
                document.getElementById(`ledInput-${ledNumber}`).checked = false;
            } catch (e) {
                // console.warn(e);
            }
        }
    }
    document.getElementById("counter").innerHTML = count.toString();
}

/**
 * Sends Data to Websocket if available
 * 
 * @param {any} string Data to send 
 */
function send(string) {
    if (webSocketOpen) {
        console.log(`sending: ${string}`);
        connection.send(string);
    } else {
        console.log("WebSocket not open, couldn't send " + string);
    }
}

/*-------------------------------------------------------------------------------------------------------------------
 *------------------------------------------<Generators>--------------------------------------------------------------
 *------------------------------------------------------------------------------------------------------------------*/

function createNavs() {
    for (var i = 0; i < groups.length; i++) { //foreach entry in groups (look variables.js)
        var nav = createNav(i);
        document.getElementById("ledGroupWrapper")
            .appendChild(nav);
    }
}

/**
 * Fills the main Navigations with controls to open secondary Navigations
 * 
 */
function fillMainNav() {
    groups.forEach((item, i) => {
        var container = document.createElement("div");
        container.setAttribute("class", "ledGroupContainer");

        var previewIcon = document.createElement("img");
        previewIcon.setAttribute("src", "preview_icon.png");
        previewIcon.setAttribute("alt", `${item[0]} Vorschau`);
        previewIcon.setAttribute("onclick", `activateLedGroup(${i})`)


        var groupText = document.createElement("span");
        groupText.setAttribute("onclick", `toggleNav(${i})`);
        groupText.setAttribute("class", "ledGroupText");
        groupText.appendChild(
            document.createTextNode(item[0])
        );

        container.appendChild(groupText);
        container.appendChild(previewIcon);
        document.getElementById("ledGroupListContainer")
            .appendChild(container);
    });
}

/**
 * Orders the Array alphabetically
 * 
 * @param {Array} items Array to be ordered, arrays are found in variables.js
 * @returns Ordered Array
 */
function sortNavArray(items) {
    var length = items.length;
    for (var i = 0; i < length; i++) { //Number of passes
        for (var j = 0; j < (length - i - 1); j++) {
            //Compare the adjacent positions
            if (points[items[j]].toLowerCase() > points[items[j + 1]].toLowerCase()) {
                var tmp = items[j]; //Temporary variable to hold the current number
                items[j] = items[j + 1]; //Replace current number with adjacent number
                items[j + 1] = tmp; //Replace adjacent number with current number
            }
        }
    }
    return items;
}


/**
 * Creates a secondary Navigation
 * 
 * @param {Array} group group that shall be converted into a Navigation, groups found in variables.js
 * @returns Finished Navigation, ready to be attached
 */
function createNav(groupIndex) {
    group = groups[groupIndex];
    group[2] = sortNavArray(group[2]);
    var wrap = document.createElement("div");
    wrap.setAttribute("class", "ledPointsWrapper");
    wrap.setAttribute("id", `ledPointsWrapper-${groupIndex}`)

    var toggleButton = document.createElement("button");
    toggleButton.innerText = "<< Zurück"
    toggleButton.setAttribute('class', 'btn btn-secondary btn-block p-1')
    toggleButton.setAttribute("onclick", `toggleNav(${groupIndex})`);
    
    wrap.appendChild(toggleButton);

    for (var i = 0; i < group[2].length; i++) {
        var div = document.createElement("label");
        div.setAttribute("class", "ledInputWrapper");
        wrap.appendChild(div);

        var spanLabel = document.createElement("div");
        spanLabel.setAttribute("class", "ledText");
        div.appendChild(spanLabel);

        var labelWrapper = document.createElement("div");
        labelWrapper.setAttribute("class", "ledSwitch");
        div.appendChild(labelWrapper);

        var label = document.createElement("label");
        label.setAttribute("class", "switch");
        labelWrapper.appendChild(label);


        var text = document.createTextNode(points[group[2][i]]);
        spanLabel.appendChild(text);

        var input = document.createElement("input");
        input.setAttribute("type", "checkbox");
        input.setAttribute("id", `ledInput-${group[2][i]}`);
        input.setAttribute("class", "switchInput");
        input.setAttribute("onclick", `toggleLED(${group[2][i]})`);
        label.appendChild(input);

        var slider = document.createElement("span");
        slider.setAttribute("class", "slider round");
        label.appendChild(slider);

    }

    return wrap;
}