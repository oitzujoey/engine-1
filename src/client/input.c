
#include "input.h"
#include "../common/common.h"
#include "../common/log.h"
#include "../common/cfg2.h"
#include "../common/str2.h"
#include "../common/memory.h"

keybinds_t g_keybinds;

int input_init(void) {
	int error = ERR_CRITICAL;
	
	g_keybinds.keys = NULL; //malloc(sizeof(keybinds_t));
	g_keybinds.length = 0;
	
	const int numJoysticks = SDL_NumJoysticks();
	info("Found %i joysticks.", numJoysticks);
	
	for (int i = 0; i < numJoysticks; i++) {
		if (SDL_IsGameController(i)) {
			SDL_GameControllerOpen(i);
			info("Opening joystick %i as controller.", i);
		}
		else {
			SDL_JoystickOpen(i);
			info("Opening joystick %i as joystick.", i);
		}
	}

	error = ERR_OK;
	return error;
}

int input_execKeyBind(SDL_Event sdlEvent, buttonType_t buttonType, bool keyDown, lua_State *luaState) {
	int error = ERR_CRITICAL;
	if (buttonType == buttonType_keyboard) printf("Keyboard %i\n", sdlEvent.key.keysym.sym);
	if (buttonType == buttonType_mouse) printf("Mouse %i\n", sdlEvent.button.button);
	if (buttonType == buttonType_joystick)
		printf("Joystick %i %i\n", sdlEvent.jbutton.button, sdlEvent.jbutton.which);
	if (buttonType == buttonType_controller)
		printf("Controller %i %i\n", sdlEvent.cbutton.button, sdlEvent.cbutton.which);
	
	// Search bindings for keycode. I don't think there is a convenient way to do binding without a search.
	for (int i = 0; i < g_keybinds.length; i++) {
		if (buttonType != g_keybinds.keys[i].buttonType) {
			continue;
		}
		
		switch (buttonType) {
		case buttonType_keyboard:
			if (g_keybinds.keys[i].key.keycode == sdlEvent.key.keysym.sym) {
				// Found the key. Now run the attached script.
				error = cfg2_execString(keyDown ? g_keybinds.keys[i].keyDownCommand : g_keybinds.keys[i].keyUpCommand, luaState, "bind");
				if (error) {
					goto cleanup_l;
				}
			}
			break;
		case buttonType_mouse:
			if (g_keybinds.keys[i].key.mouseButton == sdlEvent.button.button) {
				// Found the key. Now run the attached script.
				error = cfg2_execString(keyDown ? g_keybinds.keys[i].keyDownCommand : g_keybinds.keys[i].keyUpCommand, luaState, "bind");
				if (error) {
					goto cleanup_l;
				}
			}
			break;
		case buttonType_joystick:
			if ((g_keybinds.keys[i].key.joystickButton == sdlEvent.jbutton.button) && (g_keybinds.keys[i].which == sdlEvent.jbutton.which)) {
				// Found the key. Now run the attached script.
				error = cfg2_execString(keyDown ? g_keybinds.keys[i].keyDownCommand : g_keybinds.keys[i].keyUpCommand, luaState, "bind");
				if (error) {
					goto cleanup_l;
				}
			}
			break;
		case buttonType_controller:
			if ((g_keybinds.keys[i].key.controllerButton == sdlEvent.cbutton.button) && (g_keybinds.keys[i].which == sdlEvent.cbutton.which)) {
				// Found the key. Now run the attached script.
				error = cfg2_execString(keyDown ? g_keybinds.keys[i].keyDownCommand : g_keybinds.keys[i].keyUpCommand, luaState, "bind");
				if (error) {
					goto cleanup_l;
				}
			}
			break;
		default:
			critical_error("Illegal button type %i. Can't happen.", buttonType);
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
	}
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int input_processSdlEvents(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	/*
	If I do demos, I may use SDL_PushEvent for faking the input.
	*/
	
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		
		case SDL_FIRSTEVENT:
			error("Unhandled event type SDL_FIRSTEVENT", "");
			break;
		
		// Application events
		case SDL_QUIT:
			// TODO? This won't work on Mac OS.
			g_cfg2.quit = true;
			break;
		
		// Android, iOS and WinRT events; see Remarks for details
		case SDL_APP_TERMINATING:
			// TODO? Figure out what this does.
			g_cfg2.quit = true;
			break;
		case SDL_APP_LOWMEMORY:
			warning("Low memory. Current solution is to exit the game. Contact the developer if you encounter this bug.", "");
			break;
		case SDL_APP_WILLENTERBACKGROUND:
		case SDL_APP_DIDENTERBACKGROUND:
		case SDL_APP_WILLENTERFOREGROUND:
		case SDL_APP_DIDENTERFOREGROUND:
			error("Unhandled event type %i", event.type);
			break;
		
		// Window events
		case SDL_WINDOWEVENT:
		case SDL_SYSWMEVENT:
			error("Unhandled window event type %i", event.type);
			break;
		
		// Keyboard events
		case SDL_KEYDOWN:
			input_execKeyBind(event, buttonType_keyboard, true, luaState);
			break;
		case SDL_KEYUP:
			input_execKeyBind(event, buttonType_keyboard, false, luaState);
			break;
		case SDL_TEXTEDITING:
			error("Unhandled keyboard event type %i", event.type);
			break;
		case SDL_TEXTINPUT:
			break;
		case SDL_KEYMAPCHANGED:
			error("Unhandled keyboard event type %i", event.type);
			break;
		
		// Mouse events
		case SDL_MOUSEMOTION:
			error("Unhandled mouse event type %i", event.type);
			break;
		case SDL_MOUSEBUTTONDOWN:
			input_execKeyBind(event, buttonType_mouse, true, luaState);
			break;
		case SDL_MOUSEBUTTONUP:
			input_execKeyBind(event, buttonType_mouse, false, luaState);
			break;
		case SDL_MOUSEWHEEL:
			error("Unhandled mouse event type %i", event.type);
			break;
		
		// Joystick events
		case SDL_JOYAXISMOTION:
			// if (SDL_IsGameController(event.jdevice.which)) {
			// 	break;
			// }
		case SDL_JOYBALLMOTION:
			// if (SDL_IsGameController(event.jdevice.which)) {
			// 	break;
			// }
		case SDL_JOYHATMOTION:
			// if (SDL_IsGameController(event.jdevice.which)) {
			// 	break;
			// }
			error("Unhandled joystick event type %i", event.type);
			break;
		case SDL_JOYBUTTONDOWN:
			// if (SDL_IsGameController(event.jdevice.which)) {
			// 	break;
			// }
			input_execKeyBind(event, buttonType_joystick, true, luaState);
			break;
		case SDL_JOYBUTTONUP:
			// if (SDL_IsGameController(event.jdevice.which)) {
			// 	break;
			// }
			input_execKeyBind(event, buttonType_joystick, false, luaState);
			break;
		case SDL_JOYDEVICEADDED:
			// if (SDL_IsGameController(event.jdevice.which)) {
			// 	break;
			// }
			SDL_JoystickOpen(event.jdevice.which);
			info("Opening joystick %i as joystick.", event.jdevice.which);
			break;
		case SDL_JOYDEVICEREMOVED:
			// if (SDL_IsGameController(event.jdevice.which)) {
			// 	break;
			// }
			SDL_JoystickClose(SDL_JoystickFromInstanceID(event.jdevice.which));
			info("Closing joystick %i.", event.jdevice.which);
			break;
		
		// Controller events
		case SDL_CONTROLLERAXISMOTION:
			error("Unhandled controller event type %i", event.type);
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			input_execKeyBind(event, buttonType_controller, true, luaState);
			break;
		case SDL_CONTROLLERBUTTONUP:
			input_execKeyBind(event, buttonType_controller, false, luaState);
			break;
		case SDL_CONTROLLERDEVICEADDED:
			SDL_GameControllerOpen(event.cdevice.which);
			info("Opening joystick %i as controller.", event.cdevice.which);
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			SDL_GameControllerClose(SDL_GameControllerFromInstanceID(event.cdevice.which));
			info("Closing controller %i.", event.cdevice.which);
			break;
		case SDL_CONTROLLERDEVICEREMAPPED:
			error("Unhandled controller event type SDL_CONTROLLERDEVICEREMAPPED", "");
			break;
		
		// Touch events
		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_FINGERMOTION:
			error("Unhandled touch event type %i", event.type);
			break;
		
		// Gesture events
		case SDL_DOLLARGESTURE:
		case SDL_DOLLARRECORD:
		case SDL_MULTIGESTURE:
			error("Unhandled gesture event type %i", event.type);
			break;
		
		// Clipboard events
		case SDL_CLIPBOARDUPDATE:
			error("Unhandled clipboard event type %i", event.type);
			break;
		
		// Drag and drop events
		case SDL_DROPFILE:
		case SDL_DROPTEXT:
		case SDL_DROPBEGIN:
		case SDL_DROPCOMPLETE:
			error("Unhandled drag & drop event type %i", event.type);
			break;
		
		// Audio hotplug events
		case SDL_AUDIODEVICEADDED:
		case SDL_AUDIODEVICEREMOVED:
			error("Unhandled audio hotplug event type %i", event.type);
			break;
		
		// Render events
		case SDL_RENDER_TARGETS_RESET:
		case SDL_RENDER_DEVICE_RESET:
			error("Unhandled render event type %i", event.type);
			break;
		
		// These are for your use, and should be allocated with SDL_RegisterEvents()
		case SDL_USEREVENT:
		case SDL_LASTEVENT:
			error("Unhandled user event type %i", event.type);
			break;
		
		default:
			error("Unhandled event type %i", event.type);
			// error = ERR_GENERIC;
		}
	}
	
	error = ERR_OK;
	// cleanup_l:
	return error;
}

/*
I know, I know. This breaks the rules. Give me a break for once.
*/
int input_stringToKeybind(const char * const key, keybind_t *keybind) {
	int error = ERR_CRITICAL;

	size_t length = strlen(key);
	int index = 0;
	char *deviceId = NULL;
	size_t deviceId_length = 0;
	char *name = NULL;
	size_t name_length = 0;
	
	// Set key type.
	
	if (index >= length) {
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	switch (key[index]) {
	case 'k':
		keybind->buttonType = buttonType_keyboard;
		break;
	case 'm':
		keybind->buttonType = buttonType_mouse;
		break;
	case 'j':
		keybind->buttonType = buttonType_joystick;
		break;
	case 'c':
		keybind->buttonType = buttonType_controller;
		break;
	default:
		error("Bad key type '%c'. \"%s\".", key[index], key);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	index++;
	
	// Set device ID.
	
	if ((keybind->buttonType == buttonType_joystick) || (keybind->buttonType == buttonType_controller)) {
		if (index >= length) {
			error = ERR_GENERIC;
			goto cleanup_l;
		}
		
		// Find length of ID.
		for (int i = index; isdigit(key[i]) && (i < length); i++) {
			deviceId_length++;
		}
		// Copy ID.
		error = str2_copyLengthMalloc(&deviceId, &key[index], deviceId_length);
		if (error) {
			goto cleanup_l;
		}
		
		// Convert ID string to integer.
		// @TODO: Figure out how to deal with casts from long to something else.
		keybind->which = strtoul(deviceId, NULL, 10);
		
		index++;
	}
	else {
		keybind->which = 0;
	}
	
	// Syntax check.
	
	if (index >= length) {
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	if (key[index] != '_') {
		error("Bad key ID. \"%s\".", key);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	index++;
	
	// Key name. Integers for now. @TODO: Add descriptive names.
	
	if (index >= length) {
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Find length of ID.
	for (int i = index; isdigit(key[i]) && (i < length); i++) {
		name_length++;
	}
	
	// Syntax check. We should be at the end of the string.
	if (index + name_length < length - 1) {
		error("Bad key name. \"%s\".", key);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Copy name.
	error = str2_copyLengthMalloc(&name, &key[index], name_length);
	if (error) {
		goto cleanup_l;
	}
	
	// Convert name string to integer.
	// @TODO: Figure out how to deal with casts from long to something else.
	switch (keybind->buttonType) {
	case buttonType_keyboard:
		keybind->key.keycode = strtoul(name, NULL, 10);
		break;
	case buttonType_mouse:
		keybind->key.mouseButton = strtoul(name, NULL, 10);
		break;
	case buttonType_joystick:
		keybind->key.joystickButton = strtoul(name, NULL, 10);
		break;
	case buttonType_controller:
		keybind->key.controllerButton = strtoul(name, NULL, 10);
		break;
	default:
		error("Bad key type '%c'. \"%s\". Can't happen.", key[index], key);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	// Fill in the rest with random junk.
	
	keybind->keyDownCommand = NULL;
	keybind->keyUpCommand = NULL;
	
	
	error = ERR_OK;
	cleanup_l:

	MEMORY_FREE(&deviceId);
	MEMORY_FREE(&name);
	
	return error;
}

int input_bind(const char * const key, const char * const downCommand, const char * const upCommand) {
	int error = ERR_CRITICAL;
	
	int index = 0;
	bool found = false;
	
	keybind_t keybind;
	
	input_stringToKeybind(key, &keybind);
	
	// Find key index.
	if (g_keybinds.keys != NULL) {
		for (; (index < g_keybinds.length) && !found; index++) {
			if (keybind.buttonType == g_keybinds.keys[index].buttonType) {
				switch (keybind.buttonType) {
				case buttonType_keyboard:
					if (g_keybinds.keys[index].key.keycode == keybind.key.keycode) {
						found = true;
					}
					break;
				case buttonType_mouse:
					if (g_keybinds.keys[index].key.mouseButton == keybind.key.mouseButton) {
						found = true;
					}
					break;
				case buttonType_joystick:
					if ((g_keybinds.keys[index].key.joystickButton == keybind.key.joystickButton) && (g_keybinds.keys[index].which == keybind.which)) {
						found = true;
					}
					break;
				case buttonType_controller:
					if ((g_keybinds.keys[index].key.controllerButton == keybind.key.controllerButton) && (g_keybinds.keys[index].which == keybind.which)) {
						found = true;
					}
					break;
				default:
					critical_error("Illegal button type %i. Can't happen.", keybind.buttonType);
					error = ERR_CRITICAL;
					goto cleanup_l;
				}
			}
		}
	}
	
	// If it doesn't exist, create the key.
	if (!found) {
		g_keybinds.length++;
		g_keybinds.keys = realloc(g_keybinds.keys, g_keybinds.length * sizeof(keybind_t));
		// memset(&g_keybinds.keys[g_keybinds.length - 1], 0, sizeof(keybind_t));
		g_keybinds.keys[g_keybinds.length - 1] = keybind;
	}
	
	// Bind command(s) to keys.
	str2_copyMalloc(&g_keybinds.keys[index].keyDownCommand, downCommand);
	if (upCommand == NULL) {
		str2_copyMalloc(&g_keybinds.keys[index].keyUpCommand, "");
	}
	else {
		str2_copyMalloc(&g_keybinds.keys[index].keyUpCommand, upCommand);
	}

	error = ERR_OK;
	cleanup_l:
	return error;
}
