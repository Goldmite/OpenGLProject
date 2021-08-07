#version 450 core
#define PI 3.1415926535
layout (binding = 0, rgba32f) uniform image2D outTexture;
layout (local_size_x = 16, local_size_y = 1, local_size_z = 1) in;
layout (location = 0) uniform float Time;
struct boid {
        float x;
        float y;
        float angle;
        int turndelay;
        int spawndelay;
        int randomAngle;
    };
layout (std430, binding = 3) buffer boidBuffer {
    boid boidArray[];
};

uint hash(uint state) //hash function
{
    state ^= 2747636419u;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    return state;
}
float uintToRange01(uint state)
{
    float res = state / 4294967295f;
    return res;
}

void main(){
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 800;
    bool DelayedSpawn = false; //have spawn delay or all spawn at once
    float moveSpeed = 2;
    float turnAngle = 0.15;

    float redValue = 1.0f; //sin(Time / 2) / 2.0f + 1.0f;
    float greenValue = 1.0f; //sin(Time / 3) / 2.0f + 1.0f;
    float blueValue = 1.0f; //sin(Time) / 2.0f + 1.0f;

    int ID = int(gl_GlobalInvocationID.x); //unique shader ID
    if (ID > boidArray.length()) {
        return;
    }
    boid CurrentBoid = boidArray[ID];
    //uint random = hash(ID.x * 218 + hash(int(CurrentBoid.y * 512))); //very random
    if (CurrentBoid.spawndelay >= ID * int(DelayedSpawn)) {
        CurrentBoid.turndelay += 1;
        //teleportation border
        if (CurrentBoid.y <= 1) { //bot border
            CurrentBoid.y = WINDOW_HEIGHT - 1;
            CurrentBoid.x = WINDOW_WIDTH - CurrentBoid.x;
        }
        else if (CurrentBoid.y >= WINDOW_HEIGHT - 1) { //top border
            CurrentBoid.y = 1;
            CurrentBoid.x = WINDOW_WIDTH - CurrentBoid.x;
        }
        else if (CurrentBoid.x <= 1) { //left border
            CurrentBoid.x = WINDOW_WIDTH - 1;
            CurrentBoid.y = WINDOW_HEIGHT - CurrentBoid.y;
        }
        else if (CurrentBoid.x >= WINDOW_WIDTH - 1) { //right border
            CurrentBoid.x = 1;
            CurrentBoid.y = WINDOW_HEIGHT - CurrentBoid.y;
        }
        //movement logic
        float FOVrange = sqrt(WINDOW_WIDTH * WINDOW_HEIGHT / boidArray.length());
        float otherBoidAngle = 0;
        float avoidAngle = 0;
        float nearbyBoids = 0;
        float PosX = 0, PosY = 0;
        float CohesionIndex = 0.25;
        float AlignmentIndex = 0.50;
        float SeperationIndex = 0.25;
        if (CohesionIndex + AlignmentIndex + SeperationIndex != 0) {
        //detect nearby boids
            for (int checkboid = 0; checkboid <= boidArray.length(); checkboid++) { //loop through every boid
                if (checkboid != ID) { //not through the one that's looping through tho
                    boid otherboid = boidArray[checkboid];
                    float Distance = sqrt((otherboid.x - CurrentBoid.x) * (otherboid.x - CurrentBoid.x) + (otherboid.y - CurrentBoid.y) * (otherboid.y - CurrentBoid.y)); //ty pythagorus
                    if (Distance < FOVrange) { //check if they're inside the detection range
                        if (Distance <= FOVrange / 2 && SeperationIndex != 0) {
                            //calculate avoid index from each boid and get an angle. sum it up and later have an average of the avoid angle
                            float AvoidIndex = ((FOVrange / 2 - Distance) * (50 / FOVrange *  6) + 10) / 100; //for range 50: 10 is 100% avoid, 25 is 1% avoid
                            if (CurrentBoid.x < otherboid.x && CurrentBoid.y != otherboid.y) { //x smaller than center of mass x
                                avoidAngle += (180 + atan((otherboid.y - CurrentBoid.y) / (otherboid.x - CurrentBoid.x))) * (AvoidIndex);
                            }
                            else if (CurrentBoid.y != otherboid.y) { //x bigger than center of mass x
                                avoidAngle += (atan((otherboid.y - CurrentBoid.y) / (otherboid.x - CurrentBoid.x))) * (AvoidIndex);
                            }
                        }
                        else if (CohesionIndex != 0){
                            PosX += otherboid.x; 
                            PosY += otherboid.y;
                        }
                        if (AlignmentIndex != 0) {
                            otherBoidAngle += otherboid.angle;
                        }
                        nearbyBoids++;
                    }
                }
            }
        }
        //change colour depending on density around boid
        redValue = sin((FOVrange / 10.0f) * nearbyBoids / 10.0f + 90) / 2.0f + 1.0f;
        greenValue = sin((FOVrange / 10.0f) * nearbyBoids / 15.0f + 90) / 2.0f + 1.0f;
        blueValue = sin((FOVrange / 10.0f) * nearbyBoids / 20.0f + 90) / 2.0f + 1.0f;
        //all averages of position, alignment angle, and avoidance angle.
        float AvgX = PosX / nearbyBoids;
        float AvgY = PosY / nearbyBoids;
        float AvgAngle = otherBoidAngle / nearbyBoids;
        float AvgAvoidAngle = avoidAngle / nearbyBoids;
        if (nearbyBoids > 0) {
            //cohesion
            if (CurrentBoid.x < AvgX && CurrentBoid.y != AvgY) { //x smaller than center of mass x
                CurrentBoid.angle = atan((AvgY - CurrentBoid.y) / (AvgX - CurrentBoid.x));
            }
            else if (CurrentBoid.y != AvgY) { //x bigger than center of mass x
                CurrentBoid.angle = 180 + atan((AvgY - CurrentBoid.y) / (AvgX - CurrentBoid.x));
            }
            //update to new position after cohesion
            CurrentBoid.x += CohesionIndex * moveSpeed * cos(CurrentBoid.angle);
            CurrentBoid.y += CohesionIndex * moveSpeed * sin(CurrentBoid.angle);
            //seperation
            CurrentBoid.angle = AvgAvoidAngle;
            CurrentBoid.x += SeperationIndex * moveSpeed * cos(CurrentBoid.angle);
            CurrentBoid.y += SeperationIndex * moveSpeed * sin(CurrentBoid.angle);
            //alignment
            CurrentBoid.angle = AvgAngle; //update angle to align correctly 
            CurrentBoid.x += AlignmentIndex * moveSpeed * cos(CurrentBoid.angle);
            CurrentBoid.y += AlignmentIndex * moveSpeed * sin(CurrentBoid.angle);
        }
        else {
            //if odd multiply by 3 and add 1. if even divide by 2
            if (CurrentBoid.turndelay == 10) {
                CurrentBoid.turndelay = 0;
                if (CurrentBoid.randomAngle % 2 == 0) {
                    CurrentBoid.randomAngle = CurrentBoid.randomAngle / 2;
                    CurrentBoid.angle += turnAngle;
                }
                else {
                    CurrentBoid.randomAngle = CurrentBoid.randomAngle * 3 + 1;
                    CurrentBoid.angle -= turnAngle * 2;
                }
                if (CurrentBoid.randomAngle == 2) {
                    CurrentBoid.randomAngle = 27;
                }
            }
            CurrentBoid.x += moveSpeed * cos(CurrentBoid.angle);
            CurrentBoid.y += moveSpeed * sin(CurrentBoid.angle);
        }
    }
    else {
        CurrentBoid.spawndelay += 1;
    }
    imageStore(outTexture, ivec2(CurrentBoid.x, CurrentBoid.y), vec4(redValue, greenValue, blueValue, 1));
    //update array
    boidArray[ID] = CurrentBoid;
}