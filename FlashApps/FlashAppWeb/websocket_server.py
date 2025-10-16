#!/usr/bin/env python

import asyncio
import websockets
import json
import os
import datetime

LOG_FOLDER = "logs"

async def handler(websocket, path):
    log_handler = None
    application_name = None
    # headers = "{}\t{}\t{}".format("event", "timestamp (ms)", "performance (microseconds)")

    while True:
        message_json = await websocket.recv()
        parsed = json.loads(message_json)

        if "name" in parsed:
            application_name = parsed["name"]
            now = datetime.datetime.now().strftime("%d-%m-%y_%H-%M-%S")
            log_name = "log-{}-{}.log".format(application_name, now)
            log_path = os.path.join(LOG_FOLDER, log_name)

            log_handler = open(log_path, "w")

        if "event" in parsed:
            if "type" in parsed:
                if parsed["type"] == "mousedown":
                    continue
                print("event type : ", parsed["type"])

            log = "{} {}".format(
                parsed["date"] * 1000 * 1000,
                parsed["event"],
            )
            print(log)
            log_handler.write(log + "\n")
            log_handler.flush()

    log_handler.close()


start_server = websockets.serve(handler, '0.0.0.0', 56178)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()