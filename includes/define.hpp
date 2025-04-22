#pragma once

# define RUNNING false
# define CAPTURE_MOUSE true
# define IGNORE_MOUSE false
# define KEY_INIT false

# define W_WIDTH 1000
# define W_HEIGHT 800
# define ROTATION_SPEED 1.0f
# define MOVEMENT_SPEED 20.0f

# define COMMANDS_LIST														\
	"Controls:\n"															\
	"'H': Display commands\n"												\
	"\n"																	\
	"Camera commands:\n"													\
	"'Z', 'W': Move forward\n"												\
	"'Q', 'A': Move left\n"													\
	"'S': Move back\n"														\
	"'D': Move right\n"														\
	"' ': Move up\n"														\
	"'V': Move down\n"														\
	"'C': Toggle mouse control for camera angle\n"							\
	"(Mouse control will hide the cursor until 'C' is pressed again)\n"		\
	"\n"																	\
	"Simulation controls:\n"												\
	"'0': Reset simulation to the cube\n"									\
	"'1': Reset simulation to the sphere\n"									\
	"\n"																	\
	"Mass commands:\n"														\
	"'M': Toggle mass activity\n"											\
	"'+': Increase gravitational pull\n"									\
	"'-': Decrease gravitational pull (becomes a push after 0 threshold)\n" \
	"'U': Increase X rotation angle for particles\n"						\
	"'I': Increase Y rotation angle for particles\n"						\
	"'O': Increase Z rotation angle for particles\n"						\
	"'J': Decrease X rotation angle for particles\n"						\
	"'K': Decrease Y rotation angle for particles\n"						\
	"'L': Decrease Z rotation angle for particles\n"						\
	"'T': Reset rotation angle to default (0, 1, 0)\n"						\
	"'F': Toggle mass follow on cursor (follows screen center if mouse control is active)\n"	\
	"'P': Toggle mass visibility on screen (white sphere)\n"


