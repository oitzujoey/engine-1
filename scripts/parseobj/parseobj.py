from error import error

class ObjFaceElement:
    def __init__(self, vertex_index, texture_coordinate_index, vertex_normal_index):
        self.vertex_index = vertex_index
        self.texture_coordinate_index = texture_coordinate_index
        self.vertex_normal_index = vertex_normal_index
    def __str__(self):
        return f'{self.vertex_index}/{self.texture_coordinate_index}/{self.vertex_normal_index}'

def parseFaceElement(string):
    slash_count = string.count('/')
    string = string + '/'*(2 - slash_count)
    tokens = string.split('/')
    tokens = [int(token)-1 if token else None for token in tokens]
    return ObjFaceElement(*tokens)


class Obj:
    def __init__(self):
        self.mtllib = None
        self.usemtl = None
        self.object_name = None
        self.vertices = []
        self.texture_coordinates = []
        self.vertex_normals = []
        self.faces = []
    def __str__(self):
        return f'''Obj {{
mtllib: {self.mtllib}
usemtl: {self.usemtl}
object_name: {self.object_name}
vertices[{len(self.vertices)}]: {self.vertices}
texture_coordinates[{len(self.texture_coordinates)}]: {self.texture_coordinates}
vertex_normals[{len(self.vertex_normals)}]: {self.vertex_normals}
faces[{len(self.faces)}]: [{', '.join(map(lambda f: ', '.join(map(str, f)).join('[]'), self.faces))}]
}}'''


def parseObj(obj_file_name):
    def tofloat(v):
        if isinstance(v, list):
            return list(map(tofloat, v))
        else:
            return float(v)

    obj = Obj()
    with open(obj_file_name, 'r', encoding='ascii') as obj_file:
        for line in obj_file:
            line = line.strip()

            ## Empty
            if not line: continue
            ## Comment
            if line[0] == '#': continue

            tokens = line.split()
            opcode = tokens[0]
            args = tokens[1:]
            arg_string = line[line.find(' '):].strip()
            match opcode:
                case 'mtllib':
                    if obj.mtllib:
                        error('Only .obj files that use a single material are permitted.')
                    obj.mtllib = arg_string
                case 'o':
                    if obj.object_name:
                        error('Only .obj files that have a single named object are permitted.')
                    obj.object_name = arg_string
                case 'v':
                    obj.vertices += [tofloat(args)]
                case 'vt':
                    obj.texture_coordinates += [tofloat(args)]
                case 'vn':
                    obj.vertex_normals += [tofloat(args)]
                case 'usemtl':
                    if obj.usemtl:
                        error('Only .obj files that use a single material are permitted.')
                    obj.usemtl = arg_string
                case 's':
                    if not (len(args) == 1 and (args[0] == 'off' or args[0] == '0')):
                        print(f'Warning: Smoothing groups are not supported.')
                case 'f':
                    if len(args) != 3:
                        error(f'Only three vertices per face are allowed. To fix, import the object into Blender, press Tab to enter Edit mode, press \'a\' to select all faces, press Ctrl-t or "Face->Triangulate Faces", then export as .obj.')
                    obj.faces += [list(map(parseFaceElement, args))]
                case _:
                    print(f'Ignoring "{opcode}"')
    return obj
