function bind(name, onDown, onUp){
  var ws = new WebSocket("ws://"+ location.hostname +":56178/");

  COLORS = {"UP": "#000000", "DOWN": "#FFFFFF"};

  ws.onopen = function(){ws.send(JSON.stringify({"name": name}))};

  // decorators function to send websocket messages when the function is called
  onDown_ws = function(e){
/*    ws.send(JSON.stringify(
        {
          "event": "toolkit",
          "type": e.type,
          "date": Date.now(),
          "pos": [e.pageX, e.pageY]
        })
    );*/
    onDown();
    ws.send(JSON.stringify(
        {
          "event": "repaint",
          "date": Date.now()
        })
    );
  }

  onUp_ws = function(){
    onUp();
 }

  document.onkeydown = onDown_ws;
  document.onkeyup = onUp_ws;

/*  document.onmousedown = onDown_ws;
  document.onmouseup = onUp_ws;*/

  document.touchstart = onDown_ws;
  document.touchend = onUp_ws;

  // for chrome on android...
  document.ontouchstart = onDown_ws;
  document.ontouchend = onUp_ws;
};
