#!/usr/bin/env python3

import sys, os, tempfile, pathlib, shutil, platform, json

argv = sys.argv

def abort(): return sys.exit(1)

if len(argv) != 4:
    print(f'usage: scripts/makepak.py <config file> <source directory> <destination directory>')
    print(f'example: scripts/makepak.py build.json ./ build/')
    print(f'This script generates the game.zip file.')
    abort()

source_directory = argv[2]
destination_directory = argv[3]

config_file_name = argv[1]
with open(config_file_name) as config_file:
    config = json.load(config_file)


if 'game_directory' not in config:
    print("Must supply key \"game_directory\".")
    abort()
game_directory = f"{source_directory}/{config['game_directory']}"

if 'username' not in config:
    print("Must supply key \"username\".")
    abort()
username = config['username']
if " " in username:
    print("Username may not contain spaces.")
    abort()

if 'password' not in config:
    print("Must supply key \"password\".")
    abort()
password = config['password']
if " " in password:
    print("Password may not contain spaces.")
    abort()


print(f"{argv[0]} {{")
print(f"  game_directory: \"{game_directory}\"")
print(f"  username:       \"{username}\"")
print(f"  password:       \"{password}\"")


with tempfile.TemporaryDirectory() as temp_directory_name:
    temp_directory_path = pathlib.Path(temp_directory_name)
    ## Copy game.
    shutil.copytree(game_directory, temp_directory_path, dirs_exist_ok=True)
    ## Generate identity file.
    with open(temp_directory_path/"identity.cfg", "w") as identity_file:
        identity_file.write(f"""
set username {username}
set password {password}
""")

    ## Zip it all up.
    shutil.make_archive(f"{destination_directory}/game", 'zip', temp_directory_name)

## Tell CMake that the pak needs to be reassembled into the engine.
(pathlib.Path(source_directory)/"src/common/pak.s").touch()

print(f"  Generated \"{destination_directory}/game.zip\".")
print("}")
