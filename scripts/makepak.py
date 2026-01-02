#!/usr/bin/env python3

import sys, os, tempfile, pathlib, shutil, platform

argv = sys.argv

def abort(): return sys.exit(1)

if len(argv) != 4:
    print(f'usage: scripts/makepak.py <mod name> <username> <password>')
    print(f'example: scripts/makepak.py game_sandbox fred 1234')
    print(f'This example will generate the game.zip file for the game_sandbox game.')
    abort()

username = argv[2]
if " " in username:
    print("Username may not contain spaces.")
    abort()
password = argv[3]
if " " in password:
    print("Password may not contain spaces.")
    abort()


game_directory = argv[1]


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
    shutil.make_archive("build/game", 'zip', temp_directory_name)
