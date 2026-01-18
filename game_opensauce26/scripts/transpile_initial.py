#!/usr/bin/env python3

import sys, os, json


def quit():
    os.exit(0)

def abort(message):
    print(message)
    os.exit(1)

argv = sys.argv
if len(argv) != 1:
    print(f'usage: ./transpile_initial.py')
    quit()

with open("../opensauce26/initial.json") as canopy_data_file:
    canopy_data = json.load(canopy_data_file)
canopy_objects = canopy_data["objects"]
canopy_objects_filtered = filter(lambda object: object["name"] == "Airliner", canopy_objects)
engine_objects = list(map(lambda object: object["position"], canopy_objects_filtered))
engine_data = ("g_tokens_position_initial = "
               + (", "
                  .join(map((lambda o:
                             (", "
                              .join(map(str, o))
                              .join("[]"))),
                            engine_objects))
                  .join("[]"))
               + "\n")
with open("game_opensauce26/lua/generated/initial.lua", "w") as engine_data_file:
    engine_data_file.write(engine_data)
