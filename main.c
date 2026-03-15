#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include <stdio.h>
#include "math.h"

#define WIDTH 800
#define HEIGHT 600

#define VERTEX_COUNT 100000

#define step 3

struct Vector3 {
    float x,y,z;
};


struct Camera {
    struct Vector3 pos;
    float focalLen;
};

struct Matrix3X3 {
    float a,b,c,d,e,f,g,h,i;
};

void rotateY(struct Vector3* vector, float angleDegrees){
    float rad = angleDegrees * (3.14 / 180.0f);

    struct Matrix3X3 matrix = {
        cosf(rad), 0, sinf(rad),
        0,         1, 0,
       -sinf(rad), 0, cosf(rad)
    };

    float x = vector->x;
    float y = vector->y;
    float z = vector->z;

    vector->x = matrix.a * x + matrix.b * y + matrix.c * z;
    vector->y = matrix.d * x + matrix.e * y + matrix.f * z;
    vector->z = matrix.g * x + matrix.h * y + matrix.i * z;
}

void rotateX(struct Vector3* vector, float angle){
    float rad = angle * (3.14 / 180.0f);

    struct Matrix3X3 matrix = {
        1,       0,        0,
        0,  cosf(rad), -sinf(rad),
        0,  sinf(rad),  cosf(rad)
    };

    float x = vector->x;
    float y = vector->y;
    float z = vector->z;

    vector->x = matrix.a * x + matrix.b * y + matrix.c * z;
    vector->y = matrix.d * x + matrix.e * y + matrix.f * z;
    vector->z = matrix.g * x + matrix.h * y + matrix.i * z;
}

struct Vector3 translate(struct Vector3* vector,float x,float y,float z){
    struct Vector3 translated;
    translated.x = vector->x + x;
    translated.y = vector->y + y;
    translated.z = vector->z + z;
    return translated;
};

struct Vector3 scale(struct Vector3* vector,float s){
    struct Vector3 scaled;
    scaled.x = vector->x * s;
    scaled.y = vector->y * s;
    scaled.z = vector->z * s;
    return scaled;
};


void draw_vertex(SDL_Renderer* renderer,struct Vector3* vertex,struct Camera* cam){
    struct Vector3 translated = translate(vertex,-cam->pos.x,-cam->pos.y,-cam->pos.z);
    if(translated.z <= 0){return;}
    float scalingFactor = cam->focalLen / translated.z;
    struct Vector3 scaled = scale(&translated,scalingFactor);

    float x = WIDTH/2+scaled.x;
    float y = HEIGHT/2-scaled.y;
    if(x>0 && x<WIDTH && y>0 && y<HEIGHT){
        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        SDL_RenderPoint(renderer,x,y);
    }
}

int startsWith(const char* str,const char* sequence){
    size_t index = 0;
    while(sequence[index]!='\0'){
        if(str[index]!=sequence[index]){
            return 0;
        }
        index++;
    }

    return 1;
}


size_t get_vertices(const char* fileName,struct Vector3* buffer,size_t max){
    FILE* file = fopen(fileName,"r");
    if(!file){return -1;}

    size_t count = 0;
    char line[256];
    while(fgets(line,sizeof(line),file) && count<max){
        if(startsWith(line,"v ")){
            float x,y,z;
            if(sscanf(line,"v %f %f %f",&x,&y,&z)==3){
                buffer[count].x = x;
                buffer[count].y = y;
                buffer[count].z = z;
                count++;
            }
        }
    }

    return count;
}

void draw_vertices(SDL_Renderer* renderer,struct Vector3* arr,struct Camera* cam,size_t count){
    for(size_t i=0;i<count;i++){
        draw_vertex(renderer,&arr[i],cam);
    }
}

void handle_keyboard(const bool* keystate,struct Camera* cam,size_t vertex_count,struct Vector3* vertices){
    if(keystate[SDL_SCANCODE_A]){
        cam->pos.x -= step;
    }else if(keystate[SDL_SCANCODE_D]){
        cam->pos.x += step;
    }else if(keystate[SDL_SCANCODE_S]){
        cam->pos.z -= step;
    }else if(keystate[SDL_SCANCODE_W]){
        cam->pos.z += step;
    }else if(keystate[SDL_SCANCODE_E]){
        cam->pos.y += step;
    }else if(keystate[SDL_SCANCODE_Q]){
        cam->pos.y -= step;
    }else if(keystate[SDL_SCANCODE_R]){
        for(size_t i=0;i<vertex_count;i++){
            rotateY(&vertices[i],5);
        }
    }else if(keystate[SDL_SCANCODE_T]){
        for(size_t i=0;i<vertex_count;i++){
            rotateX(&vertices[i],5);
        }
    }
}

int main(int argc,char* argv[]){
    struct Vector3 vertices[VERTEX_COUNT];
    size_t vertexCount = get_vertices("model.obj",vertices,VERTEX_COUNT);

    struct Camera userCamera = {{0,0,-10},900.0};

    struct Vector3 vector = {0,0,100};

    if(!SDL_Init(SDL_INIT_VIDEO)){
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("3D Renderer",WIDTH,HEIGHT,0);
    if(window==NULL){return 1;}

    SDL_Renderer* renderer = SDL_CreateRenderer(window,NULL);
    if(renderer==NULL){return 1;}

    SDL_Event event;
    while(1){
        if(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT){
                break;
            }
        }

        const bool* keystate = SDL_GetKeyboardState(NULL);
        handle_keyboard(keystate,&userCamera,vertexCount,vertices);
        
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);

        draw_vertices(renderer,vertices,&userCamera,vertexCount);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
