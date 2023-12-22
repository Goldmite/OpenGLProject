# Programming 2D boids on GPU using OpenGL

> **Inspiration:** https://www.youtube.com/watch?v=bqtqltqcQhw&ab_channel=SebastianLague
### What are boids?
A boid is a "bird-oid object" which essentially simulate the natural movement of a flock of birds or school of fish following 3 simple rules:
- Cohesion: force to form a flock.
- Alignment: force to follow a similar general direction of nearby boids.
- Seperation: keep a distance between each boid to avoid collision.
> **Note:** Boids also have a field of view (FOV) cone, which replicates the vision of a bird, and the rules only apply to what is within tho FOV. I wanted to see the behaviour if I left the FOV unmanaged, therefore these boids see in all directions.


#### That's pretty much it! You now know what boids do.

### What is OpenGL?
OpenGL gives the power to simulate these boids on the GPU rather than the CPU, but why?
Well, because if we have thousands of little objects, they will all follow the previouly listed rules locally. 
In other words, they will all seperately run the code to simulate the movement of a flock. 

And a GPU does exactly that, using shaders (code for graphics). Each boid runs these shaders almost at the same time because the GPU is very very efficient at executing the same process at mass.
> **Guide:** https://learnopengl.com/

## Explaining the shader code
So, there are 3 shaders:
- ### vertex.glsl
```
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
};
```
This one is basic and keeps track of the vertices aka coordinates of a boid since it each one is just one vertice.

in - inputs position and texture coordinates.

out - outputs texture coordinates (what will be displayed.
- ### fragment.glsl
```
out vec4 FragColor;

in vec2 TexCoord;

layout (binding = 0) uniform sampler2D texture;
layout (binding = 1, rgba32f) uniform image2D outTexture;

void main()
{
    ivec2 ID = ivec2(gl_FragCoord.xy);
    float ColorDecay = 0.1;
    vec4 color = texelFetch(texture, ID, 0).rgba;
    imageStore(outTexture, ID, max(color - ColorDecay, 0.0f));
    FragColor = color;
}
```
This one here controls the color of each boid and also slowly decays it which looks like there's a fading tail following each boid.
- ### compute.glsl
Here lies all the logic regarding the movement - applying the 3 rules.
This contains an structure array of boids with the positional variables: x,y and also the direction.

There are multiple adjustable variables which lead to different behaviours and output and the main ones are weights for how much influence each rule has.
```
float CohesionIndex = 1.3, AlignmentIndex = 1.15, SeperationIndex = 0.075;
```

1. Seperation
```
if (SeperationIndex != 0) {
    if (CurrentBoid.x < otherboid.x) { //x smaller than center of mass x 
        if (CurrentBoid.y < otherboid.y) { //y smaller than center of mass y 
            avoidAngle = PI + atan((otherboid.y - CurrentBoid.y) / (otherboid.x - CurrentBoid.x));
        } else if (CurrentBoid.y > otherboid.y) { //y bigger than center of mass y 
            avoidAngle = PI + atan((otherboid.y - CurrentBoid.y) / (otherboid.x - CurrentBoid.x));
        }
    } else if (CurrentBoid.x > otherboid.x) { //x bigger than center of mass x 
        if (CurrentBoid.y > otherboid.y) { //y bigger than center of mass y 
            avoidAngle = atan((otherboid.y - CurrentBoid.y) / (otherboid.x - CurrentBoid.x));
        } else if (CurrentBoid.y < otherboid.y) { //y smaller than center of mass y 
            avoidAngle = atan((otherboid.y - CurrentBoid.y) / (otherboid.x - CurrentBoid.x));
        }
    } else if (CurrentBoid.x == otherboid.x && CurrentBoid.y == otherboid.y) {
        avoidAngle = ID; //randomly angle them if they're too close
    }
    CurrentBoid.x += SeperationIndex * (moveSpeed * AvoidSpeed) * cos(avoidAngle);
    CurrentBoid.y += SeperationIndex * (moveSpeed * AvoidSpeed) * sin(avoidAngle);
}
```
This "if else" nest essentially checks in which quadrant of a circle (where the center is the flock's center) is our current boid located.
Accordingly, following trigonometry rules, the code calculates the angle facing away from the center of mass.

2. Alignment
```
if (AlignmentIndex != 0) { //add up angles to later get average for alignment
    otherBoidAngle += otherboid.angle;
}
<...>
float AvgAngle = otherBoidAngle / nearbyBoids;
//alignment
CurrentBoid.x += AlignmentIndex * moveSpeed * cos(AvgAngle * PI / 180);
CurrentBoid.y += AlignmentIndex * moveSpeed * sin(AvgAngle * PI / 180);
```
While looping through all the boids, the angle of nearby boids is summed up. Later, the average angle is found. 
Since, we know the average angle of movement of nearby boids, we can apply this angle to the current boid directly.


3. Cohesion

```
if (CohesionIndex != 0) { //add up positions to later get average for cohesion
    PosX += otherboid.x;
    PosY += otherboid.y;
}
<...>
float AvgX = PosX / nearbyBoids;
float AvgY = PosY / nearbyBoids;
float CohesionAngle = 0;
//cohesion
if (CurrentBoid.x < AvgX && CurrentBoid.x != AvgX) { //x smaller than center of mass x
    CohesionAngle = atan((AvgY - CurrentBoid.y) / (AvgX - CurrentBoid.x));
}
else if (CurrentBoid.x != AvgX) { //x bigger than center of mass x
    CohesionAngle = PI + atan((AvgY - CurrentBoid.y) / (AvgX - CurrentBoid.x));
}
//update to new position after cohesion
CurrentBoid.x += CohesionIndex * moveSpeed * cos(CohesionAngle);
CurrentBoid.y += CohesionIndex * moveSpeed * sin(CohesionAngle); 
```
While looping through all the boids, the x and y values of nearby boids are summed up. Later, the average center of mass is found.
Then we find the angle that is directed at the center from the current boid (similar to how seperation worked, except here we don't invert the angle) and apply it to the boids coordinates.

## Conclusion

It's truly amazing how these 3 rules (developed by Craig Reynolds in 1986) lead to such complex yet elegant behaviour similar to birds/fish. 

After trying many variatons of values for each rules' weight I've noticed how it effects the movement clearly, there are some weird behaviours which are caused by trigonometry but it's is just one of many ways of implementing boids so I am quite pleased with the result.
I didn't go in depth of the main.cpp file because it isn't very friendly-looking and the information is in the OpenGL guide.

I can see how this could be optimized for performance in the future, for example implementing a quadtree: https://en.wikipedia.org/wiki/Quadtree


