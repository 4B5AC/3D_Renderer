#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 600

struct Vector3 {
    float x,y,z;
};

struct Camera {
    struct Vector3 pos;
    float focalLen;
};

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
    while(fgets(line,sizeof(line),file)){
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

void handle_movement(const bool* keystate,struct Camera* cam){
    float step = 0.05f;
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
    }
}

int main(int argc,char* argv[]){
    struct Vector3 vertices[256];
    size_t vertexCount = get_vertices("model.obj",vertices,256);

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
        handle_movement(keystate,&userCamera);
        
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
