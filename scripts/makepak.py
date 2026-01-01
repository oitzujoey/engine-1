#!/usr/bin/env python3

import sys, os, tempfile, distutils, pathlib, shutil, platform

argv = sys.argv

def abort(): return sys.exit(1)

if len(argv) != 2:
    print(f'usage: scripts/makepak.py <mod name>')
    print(f'example: scripts/makepak.py game_sandbox')
    print(f'This example will generate the game.zip file for the game_sandbox game.')
    abort()

username = input("Enter username: ")
if " " in username:
    print("Username may not contain spaces.")
    abort()
password = input("Enter password: ")
if " " in password:
    print("Password may not contain spaces.")
    abort()


game_directory = argv[1]


with tempfile.TemporaryDirectory() as temp_directory_name:
    temp_directory_path = pathlib.Path(temp_directory_name)
    ## Copy game.
    distutils.dir_util.copy_tree(game_directory, temp_directory_path)
    ## Generate identity file.
    with open(temp_directory_path/"identity.cfg", "w") as identity_file:
        identity_file.write(f"""
set username {username}
set password {password}
""")

    ## Zip it all up.
    shutil.make_archive("game", 'zip', temp_directory_name)


if platform.system() == 'Linux':
    ## Make a single-file bundle for the client.
    game_name = '_'.join(game_directory.split('_')[1:])
    game_file_name = game_name + "." + platform.machine()
    with open(game_file_name, 'wb') as game_file:
        with open("build/cengine-1", 'rb') as engine_file:
            shutil.copyfileobj(engine_file, game_file)
        with open("game.zip", 'rb') as resources_file:
            shutil.copyfileobj(resources_file, game_file)
    os.chmod(game_file_name, 0o755)
