#pragma once

# define RUNNING false
# define CAPTURE_MOUSE true
# define IGNORE_MOUSE false
# define KEY_INIT false

# define W_WIDTH 1000
# define W_HEIGHT 800
# define ROTATION_SPEED 1.0f
# define MOVEMENT_SPEED 20.0f

// Trailing config
# define TRAIL_SAMPLES 16
# define TRAIL_INTERVAL 0.07f // ~1 second of history

# define COMMANDS_LIST														\
	"Controls:\n"															\
	"'H': Display commands\n"												\
	"\n"																	\
	"Movement & camera:\n"													\
	"'W', 'A', 'S', 'D': Move forward/left/back/right\n"					\
	"'Space', 'Left Shift': Move up/down\n"									\
	"'Left Ctrl': Move faster\n"											\
	"Arrow keys or mouse (when captured): Rotate camera\n"					\
	"'C': Toggle mouse capture (hides the cursor when active)\n"			\
	"\n"																	\
	"Simulation controls:\n"												\
	"'Keypad 0': Reset simulation to the cube\n"							\
	"'Keypad 1': Reset simulation to the sphere\n"							\
	"'R': Toggle trailing mode (~1 second particle paths)\n"				\
	"'G': Toggle spaghetti mode (line strip rendering)\n"					\
	"\n"																	\
	"Mass commands:\n"														\
	"'M' or ';': Toggle mass activity\n"									\
	"'Keypad +'/'Keypad -': Increase/Decrease gravitational pull\n"			\
	"'B': Toggle slow random rotation of gravitational pull\n"				\
	"'U': Increase X rotation angle for particles\n"						\
	"'I': Increase Y rotation angle for particles\n"						\
	"'O': Increase Z rotation angle for particles\n"						\
	"'J': Decrease X rotation angle for particles\n"						\
	"'K': Decrease Y rotation angle for particles\n"						\
	"'L': Decrease Z rotation angle for particles\n"						\
	"'T': Reset rotation angle to default (0, 1, 0)\n"						\
	"'F': Toggle mass follow on cursor (follows screen center if mouse control is active)\n"	\
	"'P': Toggle mass visibility on screen (white sphere)\n"				\
	"\n"																	\
	"System:\n"															\
	"'F11': Toggle fullscreen\n"											\
	"'Esc': Quit\n"
