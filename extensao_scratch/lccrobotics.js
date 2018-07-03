(function (ext) {
    // Cleanup function when the extension is unloaded
    var notify_connect = true;
    ext._shutdown = function () {
        socketClose();
        if (retryInterval) {
            clearTimeout(retryInterval);
        }
    };
    function socketClose() {
        if (ext._socket) ext._socket.close();
        ext._socket = null;
        ext._connected = false;
    }
    // Status reporting code
    // Use this to report missing hardware, plugin or unsupported browser
    ext._getStatus = function () {
        return ext._connected ? { status: 2, msg: 'Dispositivo Conectado' } : { status: 1, msg: 'Dispositivo desconectado' };
    };

    ext.ligar_led = function (led) {
        console.log("Ligar led");
        ext.estado_led(led, 'On');
    }
    ext.desligar_led = function (led) {
        console.log("Desligar led");
        ext.estado_led(led, 'Off');
    }
    ext.estado_led = function (led, estado) {
        console.log("Estado do led ", estado);
        sendSocketMessage({ tipo: 'led', led: led, estado: estado });
    }
    ext.dispositivo_conectado = function () {
        if (isSocketOk() && notify_connect) {
            notify_connect = false;
            return true;
        } else {
            return false;
        }


    }

    var initSocket = function () {
        ext._socket = new WebSocket("ws://192.168.4.1:81");
        ext._socket.onopen = function () {
            ext._connected = true;
            notify_connect = true;
            console.log("Conectado :)")
        };

        ext._socket.onmessage = function (message) {
            //var msg = JSON.parse(message.data);
            console.log(message.data)
        };
        ext._socket.onclose = function (e) {
            socketClose();
            ext._connected = false;
            console.log("Desconectado, tentando em 3s")
            retryInterval = setTimeout(initSocket, 3000);
        };
    };
    var retryInterval = setTimeout(initSocket, 1000);

    function sendSocketMessage(obj) {
        if (isSocketOk()) {
            ext._socket.send(JSON.stringify(obj));
        }
    }
    function isSocketOk() {
        return ext._socket && ext._connected;
    }
    // Block and block menu descriptions
    var descriptor = {
        blocks: [
            [' ', 'Ligar %m.leds', 'ligar_led', 'Led 1'],
            [' ', 'Desligar %m.leds', 'desligar_led', 'Led 1'],
            [' ', 'Estado do %m.leds %m.OnOffState', 'estado_led', 'Led 1', 'On'],
            ['h', 'Quando dispositivo conectar', 'dispositivo_conectado'],

        ],
        menus: {
            'OnOffState': ['On', 'Off'],
            'leds': ['Led 1', 'Led Vermelho', 'Led Azul', 'Led Verde']
        }
    };
    ScratchExtensions.register('Elimu LCCR', descriptor, ext);
})({});