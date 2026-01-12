# particle_system  
Particle system project from 42 school, the goal is to create a simple yet efficient particle system in a 3D environment  using openGL  
  
Before you start, here are the necessary packages to download:  
sudo apt install g++ libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev \  
libglew-dev libx11-dev ocl-icd-opencl-dev opencl-headers libglm-dev  
  
Controls:  
'H'	: Display commands  
  
Movement & camera:  
'W', 'A', 'S', 'D'	: Move forward / left / back / right  
'Space', 'Left Shift'	: Move up / down  
'Left Ctrl'		: Move faster  
Arrow keys or mouse	: Rotate camera (mouse when captured)  
'C'			: Toggle mouse capture (hides the cursor when active)  
  
Simulation controls:  
'Keypad 0'	: Reset simulation to the cube  
'Keypad 1'	: Reset simulation to the sphere  
'R'		: Toggle trailing mode (~1 second particle paths)  
'G'		: Toggle spaghetti mode (line strip rendering)  
  
Mass commands:  
'M' or ';'	: Toggle mass activity  
'Keypad +' / 'Keypad -'	: Increase / decrease gravitational pull  
'B'	: Toggle slow random rotation of gravitational pull  
'U'	: Increase X rotation angle for particles  
'I'	: Increase Y rotation angle for particles  
'O'	: Increase Z rotation angle for particles  
'J'	: Decrease X rotation angle for particles  
'K'	: Decrease Y rotation angle for particles  
'L'	: Decrease Z rotation angle for particles  
'T'	: Reset rotation angle to default (0, 1, 0)  
'F'	: Toggle mass follow on cursor (follows screen center if mouse control is active)  
'Y'	: Toggle emitter follow on cursor (follows screen center if mouse control is active)  
'P'	: Toggle mass visibility on screen (white sphere)  
'E'	: Toggle emitter on/off  

System:  
'F11'	: Toggle fullscreen  
'Esc'	: Quit  
