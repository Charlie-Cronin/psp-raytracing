# psp-raytracing
a port of the ray-tracing in one weekend bookseries by Peter Shirley to the psp using the pspsdk

<img width="1016" height="575" alt="cover image from raytracing in one weekend rendered on a psp emulator" src="https://github.com/user-attachments/assets/da8ce587-0f60-49c4-b6ec-744fe3b395dc" />

## how to install
install the EBOOT.PBP file onto a modded psp into the /psp/game directory
or alternatively run the software in a PSP emulator 

## how to use
with the application open you will be greeted by a menu screen like below 
<img width="813" height="458" alt="psp-raytracing main menu" src="https://github.com/user-attachments/assets/a65bf8c4-9f9e-47b2-b86d-84c7bb60ee87" />

you can cycle between the different options by pressing up and down on the d-pad

### Scene Option
allows you to select from pre-made scenes to render, some of these can be seen below in the gallery section

### Resolution
gives the option to render at half or full resolution, half resolution will be less detailed but full resolution will take twice as long to render

### Samples
A higher sample value helps to reduce image noise and increase clarity, however it has a extreme performance cost

### Depth
the depth values controls the amount of ray bounces, improving indirect lighting and reflections, however higher values have a extreme performance cost

### Prevent Sleep
with this setting on, it prevents the PSP from entering sleep mode during the rendering process, meaning the render will be done much quicker and without you having to watch it

## Rendering
as the PSP cannot do real time ray-tracing, it will take a while to do depending on what scene you are rendering and with what settings 
to start rendering from the main menu, press the (X) button on your PSP, this will bring up the rendering screen as seen below
<img width="813" height="458" alt="psp-raytracing in progress image: 6/272 lines done" src="https://github.com/user-attachments/assets/9f02ec9b-c609-439c-a6e9-e38938686d9d" />

this screen gives progress updates as the rendering goes on, telling you exactly how many lines (out of the total vertical resolution) it has done

## Gallery
<img width="1242" height="703" alt="a cornell box with 2 cubes" src="https://github.com/user-attachments/assets/b0578ed7-e815-4fd9-9250-8ad7282709de" />
scene: cornell_box, resolution: full, samples: 95, depth: 5


<img width="1284" height="713" alt="an empty cornell box" src="https://github.com/user-attachments/assets/d616e1f4-ea9a-4ce7-a4f3-ed7131f3af59" />
scene: cornell_box(empty), resolution: full, samples: 95, depth: 5
 

<img width="1056" height="616" alt="a marble textured sphere in a dark environment lit by a single quad light" src="https://github.com/user-attachments/assets/69a7042a-2b01-4904-b1c1-5b4f4689f58f" />
scene: simple_light, resolution: full, samples: 95, depth: 5
 

<img width="1052" height="596" alt="marble sphere on marble ground" src="https://github.com/user-attachments/assets/9d333497-9ef4-4ed0-80cd-a54ab146025c" />
scene: Marble, resolution: full, samples: 40, depth: 15
 

<img width="1016" height="575" alt="RT_weekend scene" src="https://github.com/user-attachments/assets/da8ce587-0f60-49c4-b6ec-744fe3b395dc" />
scene: RT_Weekend, resolution: full, samples: 20, depth: 25
 

<img width="1052" height="596" alt="image" src="https://github.com/user-attachments/assets/fd51bb2b-5a23-474b-85a0-e29da97fb302" />
scene: 3_Spheres, resolution: full, samples: 30, depth: 15
 

