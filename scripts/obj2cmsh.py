#!/usr/bin/env python3

import os, sys
from pathlib import Path
from error.error import error
from parseobj.parseobj import parseObj
from math import inf
from struct import pack


class Cmsh:
    def __init__(self):
        self.magic = 'CMSH'
        self.version = 0
        self.mins = [inf, inf, inf]
        self.maxs = [-inf, -inf, -inf]
        self.vertices = []
        self.vertices_length = 0
    def __str__(self):
        return f'''Cmsh {{
magic: "{self.magic}"
version: {self.version}
mins: {self.mins}
maxs: {self.maxs}
vertices[{len(self.vertices)}]: {self.vertices}
vertices_length: {self.vertices_length}
}}'''


if len(sys.argv) != 2:
    error(f'usage: {sys.argv[0]} file.obj')

obj_file_name = Path(sys.argv[1])
if obj_file_name.suffix != '.obj':
    error('This script only accepts Wavefront .obj files, and those files must end with the .obj file extension.')

cmsh_file_name = obj_file_name.with_suffix('.cmsh')


## Read .obj
obj = parseObj(obj_file_name)


## Translate to cmsh
cmsh = Cmsh()
for face in obj.faces:
    for face_element in face:
        vertex = obj.vertices[face_element.vertex_index]
        cmsh.mins = list(map(min, cmsh.mins, vertex))
        cmsh.maxs = list(map(max, cmsh.maxs, vertex))
        cmsh.vertices += [vertex]
cmsh.vertices_length = len(cmsh.vertices)


## Serialize cmsh
endianness = '<'
buf = bytes()
def serialize(f, value):
    global buf
    buf += pack(f'<{f}', value)
for char in cmsh.magic:
    serialize('c', char.encode('ascii'))
serialize('I', cmsh.version)
for f in cmsh.mins:
    serialize('f', f)
for f in cmsh.maxs:
    serialize('f', f)
serialize('I', 3 * cmsh.vertices_length)  # 3 = number of floats in a vertex.
for v in cmsh.vertices:
    for f in v:
        serialize('f', f)


## Write .cmsh
with open(cmsh_file_name, 'wb') as cmsh_file:
    cmsh_file.write(buf)
