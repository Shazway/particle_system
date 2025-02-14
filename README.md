# particle_system  
Particle system project from 42 school, the goal is to create a simple yet efficient particle system in a 3D environment  using openGL  
  
Before you start, here are the necessary packages to download:  
sudo apt install g++ libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev \  
libglew-dev libx11-dev ocl-icd-opencl-dev opencl-headers libglm-dev  
  
Controls:  
'H'	: Display commands  
Camera commands:  
'Z', 'W'		: Move forward  
'Q', 'A'		: Move left  
'S'				: Move back  
'D'				: Move right  
' '				: Move up  
'V'				: Move down  
'C'				: Toggle mouse control for camera angle  
(For mouse control it will hide the mouse untill you press C again)  
(Use the mouse to turn around in the simulation like in a first person shooter)
  
Simulation controls:  
'0'	: Reset simulation to the cube  
'1'	: Reset simulation to the sphere  
  
Mass commands:  
'M'	: Toggle mass activity  
'+'	: Increase gravitational pull  
'-'	: Decrease gravitational pull (becomes a push after 0 threshold)  
'U'	: Increase X rotation angle for particles  
'I'	: Increase Y rotation angle for particles  
'O'	: Increase Z rotation angle for particles  
'J'	: Decrease X rotation angle for particles  
'K'	: Decrease Y rotation angle for particles  
'L'	: Decrease Z rotation angle for particles  
'T'	: Reset rotation angle to default x, y, z(0, 1, 0)  
'F'	: Toggle mass follow on cursor (will follow the center of the screen if mouse control is active)  
