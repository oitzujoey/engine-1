
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

int network_packetAdd_entityList(enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length, const entityList_t *data, const ptrdiff_t data_length) {
	int error = ERR_CRITICAL;
	
	const size_t data_size = sizeof(entity_t *) + 3*sizeof(size_t) + sizeof(ptrdiff_t *);
	const size_t data_byteLength = data_size / sizeof(enet_uint8) * data_length;
	
	ptrdiff_t structIndex = 0;
	
	if (*index + data_byteLength > packet_length) {
		error = ERR_GENERIC;
		error("Packet too small for data. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td", data_byteLength);
		goto cleanup_l;
	}
	
	for (ptrdiff_t i = 0; i < data_length; i++) {
		// Send little endian. Note that enet takes care of endianness in bytes.
		// entities
		for (unsigned int j = 0; j < sizeof(entity_t *); j++) {
			// packet[*index + structIndex++] = ((unsigned long long int) data[i].entities >> 8*j) & 0xFF;
			packet[*index + structIndex++] = 0;
		}
		// entities_length
		for (unsigned int j = 0; j < sizeof(size_t); j++) {
			packet[*index + structIndex++] = ((unsigned long long int) data[i].entities_length >> 8*j) & 0xFF;
		}
		// deletedEntities
		for (unsigned int j = 0; j < sizeof(ptrdiff_t *); j++) {
			// packet[*index + structIndex++] = ((unsigned long long int) data[i].deletedEntities >> 8*j) & 0xFF;
			packet[*index + structIndex++] = 0;
		}
		// deletedEntities_length
		for (unsigned int j = 0; j < sizeof(size_t); j++) {
			packet[*index + structIndex++] = ((unsigned long long int) data[i].deletedEntities_length >> 8*j) & 0xFF;
		}
		// deletedEntities_length_allocated
		for (unsigned int j = 0; j < sizeof(size_t); j++) {
			packet[*index + structIndex++] = ((unsigned long long int) data[i].deletedEntities_length_allocated >> 8*j) & 0xFF;
		}
	}
	
	// Increase the packet byte index.
	*index += data_byteLength;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int network_packetAdd_entity(enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length, const entity_t *data, const ptrdiff_t data_length) {
	int error = ERR_CRITICAL;
	
	const size_t data_size = sizeof(ptrdiff_t) + sizeof(size_t) + sizeof(entity_childType_t) + 7*sizeof(vec_t) + sizeof(bool);
	const size_t data_byteLength = data_size / sizeof(enet_uint8) * data_length;
	
	ptrdiff_t structIndex = 0;
	
	if (*index + data_byteLength > packet_length) {
		error = ERR_GENERIC;
		error("Packet too small for data. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, data_byteLength);
		goto cleanup_l;
	}
	
	for (ptrdiff_t i = 0; i < data_length; i++) {
		// Send little endian. Note that enet takes care of endianness in bytes.
		// children
		for (unsigned int j = 0; j < sizeof(ptrdiff_t *); j++) {
			// packet[*index + structIndex++] = ((unsigned long long int) data[i].children >> 8*j) & 0xFF;
			packet[*index + structIndex++] = 0;
		}
		// children_length
		for (unsigned int j = 0; j < sizeof(size_t); j++) {
			packet[*index + structIndex++] = ((unsigned long long int) data[i].children_length >> 8*j) & 0xFF;
		}
		// childType
		for (unsigned int j = 0; j < sizeof(entity_childType_t); j++) {
			packet[*index + structIndex++] = ((unsigned long long int) data[i].childType >> 8*j) & 0xFF;
		}
		// position
		for (ptrdiff_t j = 0; j < sizeof(vec3_t)/sizeof(vec_t); j++) {
			for (unsigned int k = 0; k < sizeof(vec_t); k++) {
				packet[*index + structIndex++] = (*((unsigned long long int *) &data[i].position[j]) >> 8*k) & 0xFF;
			}
		}
		// orientation.s
		for (unsigned int j = 0; j < sizeof(vec_t); j++) {
			// packet[*index + structIndex++] = ((unsigned long long int) data[i].orientation.s >> 8*j) & 0xFF;
			packet[*index + structIndex++] = (*((unsigned long long int *) &data[i].orientation.s) >> 8*j) & 0xFF;
		}
		// orientation.v
		for (ptrdiff_t j = 0; j < sizeof(vec3_t)/sizeof(vec_t); j++) {
			for (unsigned int k = 0; k < sizeof(vec_t); k++) {
				packet[*index + structIndex++] = (*((unsigned long long int *) &data[i].orientation.v[j]) >> 8*k) & 0xFF;
			}
		}
		// inUse
		for (unsigned int j = 0; j < sizeof(bool); j++) {
			packet[*index + structIndex++] = ((unsigned long long int) data[i].inUse >> 8*j) & 0xFF;
		}
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
		error("Packet too small for data. "COLOR_BLUE"[packet_length] "COLOR_CYAN"%td "COLOR_BLUE"[data_length] "COLOR_CYAN"%td"COLOR_NORMAL, data_byteLength);
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
	
	const size_t data_size = sizeof(ptrdiff_t) + sizeof(size_t) + sizeof(entity_childType_t) + 7*sizeof(vec_t) + sizeof(bool);
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
		// printf("children %lu\n", (unsigned long) data[i].children);
		// printf("children_length %lu\n", (unsigned long) data[i].children_length);
		// printf("childType %lu\n", (unsigned long) data[i].childType);
		// printf("inUse %lu\n", (unsigned long) data[i].inUse);
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
