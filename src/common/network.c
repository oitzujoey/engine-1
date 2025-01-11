
#include "network.h"
#include <string.h>
#include <stdio.h>
#include "common.h"
#include "log.h"

int g_connectionTimeout;


int network_packetAdd_uint32(enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length, const uint32_t *data, const ptrdiff_t data_length) {
	int error = ERR_CRITICAL;
	
	const size_t data_size = 4;
	const size_t data_byteLength = data_size / sizeof(enet_uint8) * data_length;
	
	if (*index + data_byteLength > packet_length) {
		error = ERR_GENERIC;
		error("Packet too small for data. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td", data_byteLength);
		goto cleanup_l;
	}
	
	for (ptrdiff_t i = 0; i < data_length; i++) {
		// Send little endian. Note that enet takes care of endianness in bytes.
		packet[*index + sizeof(uint32_t)*i + 0] = (data[i] >> 0*8) & 0x000000FF;
		packet[*index + sizeof(uint32_t)*i + 1] = (data[i] >> 1*8) & 0x000000FF;
		packet[*index + sizeof(uint32_t)*i + 2] = (data[i] >> 2*8) & 0x000000FF;
		packet[*index + sizeof(uint32_t)*i + 3] = (data[i] >> 3*8) & 0x000000FF;
	}
	
	// Increase the packet byte index.
	*index += data_byteLength;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int network_packetAdd_ptrdiff(enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length, const ptrdiff_t *data, const ptrdiff_t data_length) {
	int error = ERR_CRITICAL;
	
	const size_t data_size = sizeof(ptrdiff_t);
	const size_t data_byteLength = data_size / sizeof(enet_uint8) * data_length;
	
	ptrdiff_t structIndex = 0;
	
	if (*index + data_byteLength > packet_length) {
		error = ERR_GENERIC;
		error("Packet too small for data. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet_length, *index + data_byteLength);
		goto cleanup_l;
	}
	
	for (ptrdiff_t i = 0; i < data_length; i++) {
		// Send little endian. Note that enet takes care of endianness in bytes.
		for (unsigned int j = 0; j < sizeof(ptrdiff_t); j++) {
			packet[*index + structIndex++] = ((unsigned long long int) data[i] >> 8*j) & 0xFF;
		}
	}
	
	// Increase the packet byte index.
	*index += data_byteLength;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}


int network_packetRead_uint32(uint32_t *data, const ptrdiff_t data_length, const enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length) {
	int error = ERR_CRITICAL;
	
	const size_t data_size = sizeof(uint32_t);
	const size_t data_byteLength = data_size / sizeof(enet_uint8) * data_length;
	
	ptrdiff_t structIndex = 0;
	
	if (*index + data_byteLength > packet_length) {
		error = ERR_GENERIC;
		error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td", packet_length, data_byteLength);
		goto cleanup_l;
	}
	
	for (ptrdiff_t i = 0; i < data_length; i++) {
		// Receive little endian. Note that enet takes care of endianness in bytes.
		data[i] = 0;
		for (unsigned int j = 0; j < sizeof(uint32_t); j++) {
			data[i] = (uint32_t) ((unsigned long long int) data[i] | ((unsigned long long int) packet[*index + structIndex++] << 8*j));
		}
	}
	
	// Increase the packet byte index.
	*index += data_byteLength;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int network_packetRead_entityList(entityList_t *data, const ptrdiff_t data_length, const enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length) {
	int error = ERR_CRITICAL;
	
	const size_t data_size = sizeof(entity_t *) + 3*sizeof(size_t) + sizeof(ptrdiff_t *);
	const size_t data_byteLength = data_size / sizeof(enet_uint8) * data_length;
	
	ptrdiff_t structIndex = 0;
	
	if (*index + data_byteLength > packet_length) {
		error = ERR_GENERIC;
		error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td", packet_length, data_byteLength);
		goto cleanup_l;
	}
	
	for (ptrdiff_t i = 0; i < data_length; i++) {
		// Receive little endian. Note that enet takes care of endianness in bytes.
		// entities
		data[i].entities = 0;
		for (unsigned int j = 0; j < sizeof(entity_t *); j++) {
			data[i].entities = (entity_t *) ((unsigned long long int) data[i].entities | ((unsigned long long int) packet[*index + structIndex++] << 8*j));
		}
		// entities_length
		data[i].entities_length = 0;
		for (unsigned int j = 0; j < sizeof(size_t); j++) {
			data[i].entities_length = (size_t) ((unsigned long long int) data[i].entities_length | ((unsigned long long int) packet[*index + structIndex++] << 8*j));
		}
		// deletedEntities
		data[i].deletedEntities = 0;
		for (unsigned int j = 0; j < sizeof(ptrdiff_t *); j++) {
			data[i].deletedEntities = (ptrdiff_t *) ((unsigned long long int) data[i].deletedEntities | ((unsigned long long int) packet[*index + structIndex++] << 8*j));
		}
		// deletedEntities_length
		data[i].deletedEntities_length = 0;
		for (unsigned int j = 0; j < sizeof(size_t); j++) {
			data[i].deletedEntities_length = (size_t) ((unsigned long long int) data[i].deletedEntities_length | ((unsigned long long int) packet[*index + structIndex++] << 8*j));
		}
		// deletedEntities_length_allocated
		data[i].deletedEntities_length_allocated = 0;
		for (unsigned int j = 0; j < sizeof(size_t); j++) {
			data[i].deletedEntities_length_allocated = (size_t) ((unsigned long long int) data[i].deletedEntities_length_allocated | ((unsigned long long int) packet[*index + structIndex++] << 8*j));
		}
		// printf("entities %lu\n", (unsigned long) data[i].entities);
		// printf("entities_length %lu\n", (unsigned long) data[i].entities_length);
		// printf("deletedEntities %lu\n", (unsigned long) data[i].deletedEntities);
		// printf("deletedEntities_length %lu\n", (unsigned long) data[i].deletedEntities_length);
		// printf("deletedEntities_length_allocated %lu\n", (unsigned long) data[i].deletedEntities_length_allocated);
	}
	
	// Increase the packet byte index.
	*index += data_byteLength;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int network_packetRead_entity(entity_t *data, const ptrdiff_t data_length, const enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length) {
	int error = ERR_CRITICAL;
	
	const size_t data_size = sizeof(ptrdiff_t *) + sizeof(size_t) + sizeof(entity_childType_t) + 7*sizeof(vec_t) + 2*sizeof(bool);
	const size_t data_byteLength = data_size / sizeof(enet_uint8) * data_length;
	
	unsigned long long int tempUnsignedLongLongInt;
	
	ptrdiff_t structIndex = 0;
	
	if (*index + data_byteLength > packet_length) {
		error = ERR_GENERIC;
		error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet_length, data_byteLength);
		goto cleanup_l;
	}
	
	for (ptrdiff_t i = 0; i < data_length; i++) {
		// Receive little endian. Note that enet takes care of endianness in bytes.
		// children
		data[i].children = 0;
		for (unsigned int j = 0; j < sizeof(ptrdiff_t *); j++) {
			data[i].children = (ptrdiff_t *) ((unsigned long long int) data[i].children | ((unsigned long long int) packet[*index + structIndex++] << 8*j));
		}
		// children_length
		data[i].children_length = 0;
		for (unsigned int j = 0; j < sizeof(size_t); j++) {
			data[i].children_length = (size_t) ((unsigned long long int) data[i].children_length | ((unsigned long long int) packet[*index + structIndex++] << 8*j));
		}
		// childType
		data[i].childType = 0;
		for (unsigned int j = 0; j < sizeof(entity_childType_t); j++) {
			data[i].childType = (entity_childType_t) ((unsigned long long int) data[i].childType | ((unsigned long long int) packet[*index + structIndex++] << 8*j));
		}
		// position
		for (ptrdiff_t j = 0; j < sizeof(vec3_t)/sizeof(vec_t); j++) {
			tempUnsignedLongLongInt = 0;
			for (unsigned int k = 0; k < sizeof(vec_t); k++) {
				tempUnsignedLongLongInt = tempUnsignedLongLongInt | ((unsigned long long int) packet[*index + structIndex++] << 8*k);
			}
			data[i].position[j] = *((vec_t *) &tempUnsignedLongLongInt);
		}
		// orientation.s
		tempUnsignedLongLongInt = 0;
		for (unsigned int j = 0; j < sizeof(vec_t); j++) {
			tempUnsignedLongLongInt = tempUnsignedLongLongInt | ((unsigned long long int) packet[*index + structIndex++] << 8*j);
		}
		data[i].orientation.s = *((vec_t *) &tempUnsignedLongLongInt);
		// orientation.v
		for (ptrdiff_t j = 0; j < sizeof(vec3_t)/sizeof(vec_t); j++) {
			tempUnsignedLongLongInt = 0;
			for (unsigned int k = 0; k < sizeof(vec_t); k++) {
				tempUnsignedLongLongInt = tempUnsignedLongLongInt | ((unsigned long long int) packet[*index + structIndex++] << 8*k);
			}
			data[i].orientation.v[j] = *((vec_t *) &tempUnsignedLongLongInt);
		}
		// inUse
		data[i].inUse = 0;
		for (unsigned int j = 0; j < sizeof(bool); j++) {
			data[i].inUse = (bool) ((unsigned long long int) data[i].inUse | ((unsigned long long int) packet[*index + structIndex++] << 8*j));
		}
		// shown
		data[i].shown = 0;
		for (unsigned int j = 0; j < sizeof(bool); j++) {
			data[i].shown = (bool) ((unsigned long long int) data[i].shown | ((unsigned long long int) packet[*index + structIndex++] << 8*j));
		}
	
		// printf("children %lu\n", (unsigned long) data[i].children);
		// printf("children_length %lu\n", (unsigned long) data[i].children_length);
		// printf("childType %lu\n", (unsigned long) data[i].childType);
		// printf("inUse %lu\n", (unsigned long) data[i].inUse);
		
#ifdef CLIENT
		if (!data[i].inUse) {
			insane_free(data[i].materials);
			data[i].materials_length = 0;
		}
#endif
	}
	
	// Increase the packet byte index.
	*index += data_byteLength;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int network_packetRead_ptrdiff(ptrdiff_t *data, const ptrdiff_t data_length, const enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length) {
	int error = ERR_CRITICAL;
	
	const size_t data_size = sizeof(ptrdiff_t);
	const size_t data_byteLength = data_size / sizeof(enet_uint8) * data_length;
	
	ptrdiff_t structIndex = 0;
	
	if (*index + data_byteLength > packet_length) {
		error = ERR_GENERIC;
		error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet_length, data_byteLength);
		goto cleanup_l;
	}
	
	for (ptrdiff_t i = 0; i < data_length; i++) {
		// Receive little endian. Note that enet takes care of endianness in bytes.
		// children
		data[i] = 0;
		for (unsigned int j = 0; j < sizeof(ptrdiff_t); j++) {
			data[i] = (ptrdiff_t) ((unsigned long long int) data[i] | ((unsigned long long int) packet[*index + structIndex++] << 8*j));
		}
	}
	
	// Increase the packet byte index.
	*index += data_byteLength;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}


/*
clientState ::= data

data ::= header none
data ::= header boolean
data ::= header integer
data ::= header real
data ::= header string
data ::= header table


header ::= keyType key type
keyType ::= network_lua_type_t
type ::= network_lua_type_t

key ::= keyString_length keyString
key ::= lua_Number
key ::= lua_Integer
keyString_length ::= lua_Unsigned
keyString ::= keyString_length[char]


none ::= ''

boolean ::= int

integer ::= lua_Integer

real ::= lua_Number

string ::= string_length string_length[char]
string_length ::= lua_Unsigned

table ::= table_length table_length[data]
table_length ::= lua_Unsigned
*/
/*
Adds the object at the top of the stack to the specfied position in the packet.
Name has to be provided, but type comes straight from the data.
*/
int network_packetAdd_lua_object(lua_State *luaState, const void *name, network_lua_type_t nameType, ENetPacket *packet, ptrdiff_t *index) {
	int error = ERR_CRITICAL;
	
	int luaType;
	size_t newPacketLength = packet->dataLength;
	bool isInteger = false;
	ptrdiff_t structIndex = 0;
	size_t name_length;
	network_lua_type_t dataType;
	lua_Integer tempLuaInteger;
	lua_Number tempLuaReal;
	int tempLuaBoolean;
	const char *tempLuaString = NULL;
	lua_Unsigned tempLuaString_length;
	ptrdiff_t tableLengthIndex;
	// const char *keyName = NULL;
	lua_Unsigned tableLength = 0;
	// network_lua_type_t keyType;
	
	luaType = lua_type(luaState, -1);
	
	// Check type and increase length.
	switch (luaType) {
	case LUA_TNIL:
		error("Object is nil. Nil values cannot be sent over the network.", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	case LUA_TNUMBER:
		isInteger = lua_isinteger(luaState, -1);
		if (isInteger) {
			dataType = network_lua_type_integer;
			newPacketLength += sizeof(lua_Integer);
		}
		else {
			dataType = network_lua_type_real;
			newPacketLength += sizeof(lua_Number);
		}
		break;
	case LUA_TBOOLEAN:
		dataType = network_lua_type_boolean;
		// Bools are big. What a waste.
		newPacketLength += sizeof(int);
		break;
	case LUA_TSTRING:
		dataType = network_lua_type_string;
		// Length + string.
		newPacketLength += sizeof(lua_Unsigned) + lua_rawlen(luaState, -1) * sizeof(char);
		break;
	case LUA_TTABLE:
		dataType = network_lua_type_table;
		/*
		Can find this now, but I don't feel like doing it twice. I'd rather do
		malloc twice than iterate through the table multiple times.
		What is easy to do here is to account for the table header.
		*/
		newPacketLength += sizeof(lua_Unsigned);
		break;
	case LUA_TFUNCTION:
		error("Object is a function. Functions cannot be sent over the network (yet).", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	case LUA_TUSERDATA:
		error("Object is userdata. Userdata cannot be sent over the network (yet).", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	case LUA_TTHREAD:
		error("Object is a thread. Threads cannot be sent over the network (yet).", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	case LUA_TLIGHTUSERDATA:
		error("Object is light userdata. Light userdata cannot be sent over the network (yet).", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	case LUA_TNONE:
		dataType = network_lua_type_none;
		warning("Object is type \"none\". Is this what you intended to send?", "");
		break;
	default:
		critical_error("Object has an invalid type.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	// Add header length.
	
	switch (nameType) {
	case network_lua_type_integer:
		newPacketLength += sizeof(lua_Integer);
		break;
	case network_lua_type_real:
		newPacketLength += sizeof(lua_Number);
		break;
	case network_lua_type_string:
		// Name. Length + string.
		name_length = strlen((char *) name);
		newPacketLength += sizeof(lua_Unsigned) + name_length * sizeof(char);
		break;
	default:
		critical_error("Illegal type \"%i\" for object name.", nameType);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	// Data type + name type.
	newPacketLength += 2 * sizeof(network_lua_type_t);
	
	// Allocate space for header and data.
	error = enet_packet_resize(packet, newPacketLength);
	if (error < 0) {
		critical_error("May be out of memory.", "");
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	// Create header.
	
	// Name type.
	for (unsigned int i = 0; i < sizeof(network_lua_type_t); i++) {
		packet->data[*index + structIndex++] = ((unsigned long long) nameType >> 8*i) & 0xFF;
	}
	
	switch (nameType) {
	case network_lua_type_integer:
		for (unsigned int i = 0; i < sizeof(lua_Integer); i++) {
			packet->data[*index + structIndex++] = ((unsigned long long) *((lua_Integer *) name) >> 8*i) & 0xFF;
		}
		break;
	case network_lua_type_real:
		for (unsigned int i = 0; i < sizeof(lua_Number); i++) {
			packet->data[*index + structIndex++] = ((unsigned long long) *((lua_Number *) name) >> 8*i) & 0xFF;
		}
		break;
	case network_lua_type_string:
		// Name length.
		for (unsigned int i = 0; i < sizeof(lua_Unsigned); i++) {
			packet->data[*index + structIndex++] = ((unsigned long long) name_length >> 8*i) & 0xFF;
		}
		// Name.
		for (unsigned int i = 0; i < name_length; i++) {
			packet->data[*index + structIndex++] = ((char *) name)[i];
		}
		break;
	default:
		critical_error("Illegal type \"%i\" for object name.", nameType);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	// Type.
	for (unsigned int i = 0; i < sizeof(network_lua_type_t); i++) {
		packet->data[*index + structIndex++] = ((unsigned long long) dataType >> 8*i) & 0xFF;
	}
	
	// Insert data.
	
	switch (dataType) {
	case network_lua_type_integer:
		tempLuaInteger = lua_tointeger(luaState, -1);
		for (unsigned int i = 0; i < sizeof(lua_Integer); i++) {
			packet->data[*index + structIndex++] = ((unsigned long long) tempLuaInteger >> 8*i) & 0xFF;
		}
		break;
	case network_lua_type_real:
		tempLuaReal = lua_tonumber(luaState, -1);
		for (unsigned int i = 0; i < sizeof(lua_Number); i++) {
			packet->data[*index + structIndex++] = ( *((unsigned long long *) &tempLuaReal) >> 8*i) & 0xFF;
		}
		break;
	case network_lua_type_boolean:
		tempLuaBoolean = lua_toboolean(luaState, -1);
		for (unsigned int i = 0; i < sizeof(int); i++) {
			packet->data[*index + structIndex++] = ((unsigned long long) tempLuaBoolean >> 8*i) & 0xFF;
		}
		break;
	case network_lua_type_string:
		tempLuaString = lua_tostring(luaState, -1);
		tempLuaString_length = lua_rawlen(luaState, -1);
		// String length.
		for (unsigned int i = 0; i < sizeof(lua_Unsigned); i++) {
			packet->data[*index + structIndex++] = ((unsigned long long) tempLuaString_length >> 8*i) & 0xFF;
		}
		// String.
		for (unsigned int i = 0; i < tempLuaString_length; i++) {
			packet->data[*index + structIndex++] = tempLuaString[i];
		}
		break;
	case network_lua_type_table:
		// Save current index (start of table length).
		tableLengthIndex = *index + structIndex;
		
		structIndex += sizeof(lua_Unsigned);
		
		// Update index to prepare for recursion.
		*index += structIndex;
		structIndex = 0;
		
		// For each key in the table...
		lua_pushnil(luaState);  // First key.
		while (lua_next(luaState, -2)) {
		
			switch (lua_type(luaState, -2)) {
			case LUA_TNUMBER:
				if (lua_isinteger(luaState, -2)) {
					// Add the value to the packet.
					tempLuaInteger = lua_tointeger(luaState, -2);
					error = network_packetAdd_lua_object(luaState, &tempLuaInteger, network_lua_type_integer, packet, index);
				}
				else {
					// Add the value to the packet.
					tempLuaReal = lua_tonumber(luaState, -2);
					error = network_packetAdd_lua_object(luaState, &tempLuaReal, network_lua_type_real, packet, index);
				}
				break;
			case LUA_TSTRING:
				// Add the value to the packet.
				error = network_packetAdd_lua_object(luaState, lua_tostring(luaState, -2), network_lua_type_string, packet, index);
				break;
			default:
				critical_error("Bad key type. Can't happen.", "");
				error = ERR_CRITICAL;
				goto cleanup_l;
			}
			if (error) {
				error = ERR_GENERIC;
				goto cleanup_l;
			}
			
			// Pop value.
			lua_pop(luaState, 1);
			
			tableLength++;
		}
		
		// Fill in table length.
		for (unsigned int i = 0; i < sizeof(lua_Unsigned); i++) {
			packet->data[tableLengthIndex + structIndex++] = ((unsigned long long) tableLength >> 8*i) & 0xFF;
		}
		structIndex = 0;
		break;
	case network_lua_type_none:
		// Shouldn't have to push anything...
		break;
	default:
		critical_error("\"%s\" has an invalid type.", name);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	// Increase the packet byte index.
	*index += structIndex;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

/*
After running this function, the stack will look like this:
-1  Value
-2  Key
-3  ...
*/
int network_packetRead_lua_object(lua_State *luaState, ENetPacket *packet, ptrdiff_t *index) {
	int error = ERR_CRITICAL;
	
	unsigned long long tempUnsignedLongLong;
	int tempInt;
	lua_Integer tempLuaInteger;
	lua_Number tempLuaNumber;
	lua_Unsigned tempLuaString_length;
	char *tempLuaString = NULL;
	lua_Unsigned tempLuaTable_length;
	
	ptrdiff_t structIndex = 0;
	size_t data_byteLength = 0;
	
	network_lua_type_t keyType;
	lua_Integer keyInteger;
	lua_Number keyReal;
	lua_Unsigned keyString_length;
	char *keyString = NULL;
	network_lua_type_t dataType;
	
	// Make sure there is enough space on the stack.
	// The maximum number of elements that we will need for this call should be 3.
	error = lua_checkstack(luaState, 3);
	if (!error) {
		outOfMemory();
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	// Header //
	
	// Read key type.
	
	data_byteLength += sizeof(network_lua_type_t);
	if (*index + data_byteLength > packet->dataLength) {
		error = ERR_GENERIC;
		error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet->dataLength, data_byteLength);
		goto cleanup_l;
	}
	
	tempUnsignedLongLong = 0;
	for (unsigned long i = 0; i < sizeof(network_lua_type_t); i++) {
		tempUnsignedLongLong = tempUnsignedLongLong | ((unsigned long long) packet->data[*index + structIndex++] << 8*i);
	}
	keyType = *((network_lua_type_t *) &tempUnsignedLongLong);
	
	// Read key. This will be at index -2 at return.
	
	switch (keyType) {
	case network_lua_type_integer:
		data_byteLength += sizeof(lua_Integer);
		if (*index + data_byteLength > packet->dataLength) {
			error = ERR_GENERIC;
			error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet->dataLength, data_byteLength);
			goto cleanup_l;
		}
		
		tempUnsignedLongLong = 0;
		for (unsigned long i = 0; i < sizeof(lua_Integer); i++) {
			tempUnsignedLongLong = tempUnsignedLongLong | ((unsigned long long) packet->data[*index + structIndex++] << 8*i);
		}
		keyInteger = *((lua_Integer *) &tempUnsignedLongLong);
		
		lua_pushinteger(luaState, keyInteger);
		break;
	case network_lua_type_real:
		data_byteLength += sizeof(lua_Number);
		if (*index + data_byteLength > packet->dataLength) {
			error = ERR_GENERIC;
			error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet->dataLength, data_byteLength);
			goto cleanup_l;
		}
		
		tempUnsignedLongLong = 0;
		for (unsigned long i = 0; i < sizeof(lua_Number); i++) {
			tempUnsignedLongLong = tempUnsignedLongLong | ((unsigned long long) packet->data[*index + structIndex++] << 8*i);
		}
		keyReal = *((lua_Number *) &tempUnsignedLongLong);
		
		lua_pushnumber(luaState, keyReal);
		break;
	case network_lua_type_string:
		// Length
		data_byteLength += sizeof(lua_Unsigned);
		if (*index + data_byteLength > packet->dataLength) {
			error = ERR_GENERIC;
			error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet->dataLength, data_byteLength);
			goto cleanup_l;
		}
		
		tempUnsignedLongLong = 0;
		for (unsigned long i = 0; i < sizeof(lua_Unsigned); i++) {
			tempUnsignedLongLong = tempUnsignedLongLong | ((unsigned long long) packet->data[*index + structIndex++] << 8*i);
		}
		keyString_length = *((lua_Unsigned *) &tempUnsignedLongLong);
		
		// String
		data_byteLength += keyString_length * sizeof(char);
		if (*index + data_byteLength > packet->dataLength) {
			error = ERR_GENERIC;
			error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet->dataLength, data_byteLength);
			goto cleanup_l;
		}
		
		keyString = malloc((keyString_length + 1) * sizeof(char));
		if (keyString == NULL) {
			outOfMemory();
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
		
		for (lua_Unsigned i = 0; i < keyString_length; i++) {
			keyString[i] = (char) packet->data[*index + structIndex++];
		}
		keyString[keyString_length] = '\0';
		
		lua_pushstring(luaState, keyString);
		
		insane_free(keyString);
		keyString_length = 0;
		break;
	default:
		error("Bad key type \"%i\".", keyType);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Read object type.
	
	data_byteLength += sizeof(network_lua_type_t);
	if (*index + data_byteLength > packet->dataLength) {
		error = ERR_GENERIC;
		error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet->dataLength, data_byteLength);
		goto cleanup_l;
	}
	
	tempUnsignedLongLong = 0;
	for (unsigned long i = 0; i < sizeof(network_lua_type_t); i++) {
		tempUnsignedLongLong = tempUnsignedLongLong | ((unsigned long long) packet->data[*index + structIndex++] << 8*i);
	}
	dataType = *((network_lua_type_t *) &tempUnsignedLongLong);
	
	// Create and set object. This will be at index -1 at return.
	
	switch (dataType) {
	case network_lua_type_none:
		// We are done here.
		break;
	case network_lua_type_integer:
		data_byteLength += sizeof(lua_Integer);
		if (*index + data_byteLength > packet->dataLength) {
			error = ERR_GENERIC;
			error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet->dataLength, data_byteLength);
			goto cleanup_l;
		}
		
		tempUnsignedLongLong = 0;
		for (unsigned long i = 0; i < sizeof(lua_Integer); i++) {
			tempUnsignedLongLong = tempUnsignedLongLong | ((unsigned long long) packet->data[*index + structIndex++] << 8*i);
		}
		tempLuaInteger = *((lua_Integer *) &tempUnsignedLongLong);
		
		lua_pushinteger(luaState, tempLuaInteger);
		break;
	case network_lua_type_real:
		data_byteLength += sizeof(lua_Number);
		if (*index + data_byteLength > packet->dataLength) {
			error = ERR_GENERIC;
			error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet->dataLength, data_byteLength);
			goto cleanup_l;
		}
		
		tempUnsignedLongLong = 0;
		for (unsigned long i = 0; i < sizeof(lua_Number); i++) {
			tempUnsignedLongLong = tempUnsignedLongLong | ((unsigned long long) packet->data[*index + structIndex++] << 8*i);
		}
		tempLuaNumber = *((lua_Number *) &tempUnsignedLongLong);
		
		lua_pushnumber(luaState, tempLuaNumber);
		break;
	case network_lua_type_boolean:
		data_byteLength += sizeof(int);
		if (*index + data_byteLength > packet->dataLength) {
			error = ERR_GENERIC;
			error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet->dataLength, data_byteLength);
			goto cleanup_l;
		}
		
		tempUnsignedLongLong = 0;
		for (unsigned long i = 0; i < sizeof(int); i++) {
			tempUnsignedLongLong = tempUnsignedLongLong | ((unsigned long long) packet->data[*index + structIndex++] << 8*i);
		}
		tempInt = *((int *) &tempUnsignedLongLong);
		
		lua_pushboolean(luaState, tempInt);
		break;
	case network_lua_type_string:
		// Length
		data_byteLength += sizeof(lua_Unsigned);
		if (*index + data_byteLength > packet->dataLength) {
			error = ERR_GENERIC;
			error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet->dataLength, data_byteLength);
			goto cleanup_l;
		}
		
		tempUnsignedLongLong = 0;
		for (unsigned long i = 0; i < sizeof(lua_Unsigned); i++) {
			tempUnsignedLongLong = tempUnsignedLongLong | ((unsigned long long) packet->data[*index + structIndex++] << 8*i);
		}
		tempLuaString_length = *((lua_Unsigned *) &tempUnsignedLongLong);
		
		// String
		data_byteLength += tempLuaString_length * sizeof(char);
		if (*index + data_byteLength > packet->dataLength) {
			error = ERR_GENERIC;
			error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet->dataLength, data_byteLength);
			goto cleanup_l;
		}
		
		tempLuaString = malloc((tempLuaString_length + 1) * sizeof(char));
		if (tempLuaString == NULL) {
			outOfMemory();
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
		
		for (lua_Unsigned i = 0; i < tempLuaString_length; i++) {
			tempLuaString[i] = (char) packet->data[*index + structIndex++];
		}
		tempLuaString[tempLuaString_length] = '\0';
		
		lua_pushstring(luaState, tempLuaString);
		
		insane_free(tempLuaString);
		tempLuaString_length = 0;
		break;
	case network_lua_type_table:
		// Length
		data_byteLength += sizeof(lua_Unsigned);
		if (*index + data_byteLength > packet->dataLength) {
			error = ERR_GENERIC;
			error("Malformed packet. End of packet reached. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, packet->dataLength, data_byteLength);
			goto cleanup_l;
		}
		
		tempUnsignedLongLong = 0;
		for (unsigned int i = 0; i < sizeof(lua_Unsigned); i++) {
			tempUnsignedLongLong = tempUnsignedLongLong | ((unsigned long long) packet->data[*index + structIndex++] << 8*i);
		}
		tempLuaTable_length = *((lua_Unsigned *) &tempUnsignedLongLong);
		
		// Table
		lua_createtable(luaState, 0, tempLuaTable_length);
		
		// Prepare `*index` for recursion.
		*index += structIndex;
		structIndex = 0;
		for (lua_Unsigned i = 0; i < tempLuaTable_length; i++) {
			// No more length checking because this does it for us.
			error = network_packetRead_lua_object(luaState, packet, index);
			if (error) {
				goto cleanup_l;
			}
			/*
			Stack:
			-1  Data
			-2  Key
			-3  The table that was just created.
			-4  The key of the table that was just created.
			-5  ...
			*/
			lua_settable(luaState, -3);
		}
		/*
		Stack:
		-1  The table that was just created.
		-2  The key of the table that was just created.
		-3  ...
		*/
		break;
	default:
		error("Object has an invalid type.", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	*index += structIndex;
	structIndex = 0;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}



int network_ipv4ToString(char **string, uint32_t ipAddress) {
	int error = ERR_CRITICAL;

	uint8_t byte[4];
	
	*string = realloc(*string, (strlen("255.255.255.255") + 1) * sizeof(char));
	if (*string == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	for (int i = 0; i < 4; i++) {
		byte[i] = (ipAddress >> (i * 8)) & 0x000000FF;
	}
	
	sprintf(*string, "%d.%d.%d.%d", byte[0], byte[1], byte[2], byte[3]);
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int network_callback_connectionTimeout(cfg2_var_t *var, const char *command, lua_State *luaState) {
	
	g_connectionTimeout = var->integer;
	
	if (var->integer < 1) {
		warning("Variable \"%s\" out of range. Setting to 1.", var->name);
		var->integer = 1;
	}
	
	return ERR_OK;
}

uint32_t network_generateChecksum(const uint8_t *data, size_t length) {
	// Adler-32
	const uint32_t prime = 65521;
	uint32_t a = 1, b = 0;
	for (int i = 0; i < length; i++) {
		a += data[i];
		// Could also replace with modulo.
		if (a >= prime) {
			a -= prime;
		}
		b += a;
		if (b >= prime) {
			b -= prime;
		}
	}
	
	return (b << 16) + a;
}
