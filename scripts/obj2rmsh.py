#!/usr/bin/env python3

import os, sys
from pathlib import Path
from error.error import error
from parseobj.parseobj import parseObj
from math import inf
from struct import pack


class Rmsh:
    def __init__(self):
        self.magic = 'RMSH'
        self.version = 0
        self.mins = [inf, inf, inf]
        self.maxs = [-inf, -inf, -inf]
        self.vertices = []
        self.vertices_length = 0
        self.vertexNormals = []
        self.vertexNormals_length = 0
        self.vertexTextureCoordinates = []
        self.vertexTextureCoordinates_length = 0
    def __str__(self):
        return f'''Rmsh {{
magic: "{self.magic}"
version: {self.version}
mins: {self.mins}
maxs: {self.maxs}
vertices[{len(self.vertices)}]: {self.vertices}
vertices_length: {self.vertices_length}
vertexNormals[{len(self.vertexNormals)}]: {self.vertexNormals}
vertexNormals_length: {self.vertexNormals_length}
vertexTextureCoordinates[{len(self.vertexTextureCoordinates)}]: {self.vertexTextureCoordinates}
vertexTextureCoordinates_length: {self.vertexTextureCoordinates_length}
}}'''


if len(sys.argv) != 2:
    error(f'usage: {sys.argv[0]} file.obj')

obj_file_name = Path(sys.argv[1])
if obj_file_name.suffix != '.obj':
    error('This script only accepts Wavefront .obj files, and those files must end with the .obj file extension.')

rmsh_file_name = obj_file_name.with_suffix('.rmsh')


## Read .obj
obj = parseObj(obj_file_name)


## Translate to rmsh
rmsh = Rmsh()
for face in obj.faces:
    for face_element in reversed(face):
        vertex = obj.vertices[face_element.vertex_index]
        vertex_normal = obj.vertex_normals[face_element.vertex_normal_index]
        texture_coordinate = obj.texture_coordinates[face_element.texture_coordinate_index]
        rmsh.mins = list(map(min, rmsh.mins, vertex))
        rmsh.maxs = list(map(max, rmsh.maxs, vertex))
        rmsh.vertices += [vertex]
        rmsh.vertexNormals += [vertex_normal]
        rmsh.vertexTextureCoordinates += [[texture_coordinate[0], -texture_coordinate[1]]]
rmsh.vertices_length = len(rmsh.vertices)
rmsh.vertexNormals_length = len(rmsh.vertexNormals)
rmsh.vertexTextureCoordinates_length = len(rmsh.vertexTextureCoordinates)


print(obj)
print(rmsh)


## Serialize rmsh
endianness = '<'
buf = bytes()
def serialize(f, value):
    global buf
    buf += pack(f'<{f}', value)
for char in rmsh.magic:
    serialize('c', char.encode('ascii'))
serialize('I', rmsh.version)
for f in rmsh.mins:
    serialize('f', f)
for f in rmsh.maxs:
    serialize('f', f)
serialize('I', 3 * rmsh.vertices_length)  # 3 = number of floats in a vertex.
serialize('I', 3 * rmsh.vertexNormals_length)  # 3 = number of floats in a vertex normal.
serialize('I', 2 * rmsh.vertexTextureCoordinates_length)  # 2 = number of floats in a vertex texture coordinate.
for v in rmsh.vertices:
    for f in v:
        serialize('f', f)
for v in rmsh.vertexNormals:
    for f in v:
        serialize('f', f)
for v in rmsh.vertexTextureCoordinates:
    for f in v:
        serialize('f', f)


## Write .rmsh
with open(rmsh_file_name, 'wb') as rmsh_file:
    rmsh_file.write(buf)
