# What is it ?

 * Vanilla : changing document bakground style with normal javacript (CSS is GPU accelerated, should be fast)
 * Canvas : a canvas fills all the page, we fill all the canvas with a color
 * WebGL : we change the background of a renderer with Three.js

# How to use

## Serve HTML page

 Launch a webserver in the current directory: `python3 -m http.server` (this is needed to load the relative HTML/JS files).

 Go to http://localhost:8000.


## Websockets communication

We use websockets to send logs when we change state (up or down), so we can sync this with TurboHID logs :)

Logs will be written in the logs directory (/InteractiveLatencyMeasure/code/flashing_app/logs), named with the application that produced them and the time of creation.

Websocket communication uses Python3 (preferably python3.5 and more)

You need to install websockets : `pip install websockets`

To run : `python websocket_server.py`



 Enjoy :)