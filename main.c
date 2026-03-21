#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>

#define WIDTH 800
#define HEIGHT 600

#define step 0.5

struct VECTOR3 {
    float x,y,z;
};

struct TRIANGLE {
    struct VECTOR3 a;
    struct VECTOR3 b;
    struct VECTOR3 c;
};

struct VECTOR3 translate(struct VECTOR3* vector,float x,float y,float z){
    struct VECTOR3 translated;
    translated.x = vector->x+x;
    translated.y = vector->y+y;
    translated.z = vector->z+z;
    return translated;
}

struct VECTOR3 scale(struct VECTOR3* vector,float s){
    struct VECTOR3 scaled;
    scaled.x = vector->x*s;
    scaled.y = vector->y*s;
    scaled.z = vector->z*s;
    return scaled;
}

struct CAMERA {
    struct VECTOR3 pos;
    float focal;
};

struct VECTOR_DATA {
    size_t VERTEX_COUNT;
    struct VECTOR3* VERTICES;
};

struct VECTOR_DATA* get_vertices(const char* file_name){
    FILE* file = fopen(file_name,"r");
    if(!file){return NULL;};
    size_t count = 0;
    char line[256];
    while(fgets(line,sizeof(line),file)){
        if(strncmp(line,"v ",2)==0){
            count++;
        }
    }

    struct VECTOR_DATA* vector_data = malloc(sizeof(struct VECTOR_DATA));
    if(!vector_data){return NULL;}
    struct VECTOR3* vertices = malloc(sizeof(struct VECTOR3)*count);
    if(!vertices){free(vector_data);return NULL;}
    fseek(file,0,SEEK_SET);
    struct VECTOR3 v;

    size_t index = 0;
    while(fgets(line,sizeof(line),file)){
        if(strncmp(line,"v ",2)==0){
            if(sscanf(line,"v %f %f %f",&v.x,&v.y,&v.z)==3){
               vertices[index] = v;
               index++; 
            }
        }
    }

    struct VECTOR3 midpoint = {0,0,0};
    for(size_t i=0;i<count;i++){
        struct VECTOR3* v = &vertices[i];
        midpoint.x += v->x;
        midpoint.y += v->y;
        midpoint.z += v->z;
    }

    midpoint.x /= count;
    midpoint.y /= count;
    midpoint.z /= count;

    for(size_t i=0;i<count;i++){
        struct VECTOR3* v = &vertices[i];
        v->x -= midpoint.x;
        v->y -= midpoint.y;
        v->z -= midpoint.z;
    }

    fclose(file);
    vector_data->VERTEX_COUNT = count;
    vector_data->VERTICES = vertices;
    return vector_data;
}

void free_VECTOR_DATA(struct VECTOR_DATA* VD){
    free(VD->VERTICES);
    free(VD);
}

void draw_vertex(uint32_t* framebuffer,struct VECTOR3* offset,struct VECTOR3* origin,struct CAMERA* camera){
    struct VECTOR3 translated = translate(origin,-camera->pos.x+offset->x,-camera->pos.y+offset->y,-camera->pos.z+offset->z);
    if(translated.z>0){
        float scalingFactor = camera->focal / translated.z;
        struct VECTOR3 scaled = scale(&translated,scalingFactor);
        int x = WIDTH/2 + (int)scaled.x;
        int y = HEIGHT/2 - (int)scaled.y;
        if(x>=0&&x<WIDTH&&y>=0&&y<HEIGHT){
            framebuffer[y*WIDTH+x] = 0xFF000000;
        }
    }
}

struct MODEL {
    struct VECTOR3 position;
    struct VECTOR_DATA* VD;
};

struct MODEL* GET_MODEL(const char* file_name,float x,float y,float z){
    struct MODEL* model = malloc(sizeof(struct MODEL));
    if(!model){return NULL;}
    struct VECTOR_DATA* VD =  get_vertices(file_name);
    if(!VD){free(model);return NULL;}
    model->VD = VD;
    model->position.x = x;
    model->position.y = y;
    model->position.z = z;
    return model;
}

void MODEL_DRAW(struct MODEL* model,uint32_t* framebuffer,struct CAMERA* camera){
    struct VECTOR_DATA* VD = model->VD;
    if(VD){
        for(size_t i=0;i<VD->VERTEX_COUNT;i++){
            draw_vertex(framebuffer,&VD->VERTICES[i],&model->position,camera);
        }
    }
}

void MODEL_DESTROY(struct MODEL* model){
    free(model->VD->VERTICES);
    free(model->VD);
    free(model);
}

void MODEL_ROTATE_Y(struct MODEL* model,float angle){
    float rad = angle*3.14/180;
    float matrix[3][3] = {
        {cos(rad),0,sin(rad)},
        {0,1,0},
        {-sin(rad),0,cos(rad)}
    };

    for(size_t i=0;i<model->VD->VERTEX_COUNT;i++){
        struct VECTOR3* v = &model->VD->VERTICES[i];
        float x = v->x;
        float y = v->y;
        float z = v->z;
        v->x = x*matrix[0][0] + y*matrix[0][1] + z*matrix[0][2];
        v->y= x*matrix[1][0] + y*matrix[1][1] + z*matrix[1][2];
        v->z = x*matrix[2][0] + y*matrix[2][1] + z*matrix[2][2];
    }
}

void MODEL_ROTATE_X(struct MODEL* model,float angle){
    float rad = angle*3.14/180;
    float matrix[3][3] = {
        {1,0,0},
        {0,cos(rad),-sin(rad)},
        {0,sin(rad),cos(rad)}
    };

    for(size_t i=0;i<model->VD->VERTEX_COUNT;i++){
        struct VECTOR3* v = &model->VD->VERTICES[i];
        float x = v->x;
        float y = v->y;
        float z = v->z;
        v->x = x*matrix[0][0] + y*matrix[0][1] + z*matrix[0][2];
        v->y= x*matrix[1][0] + y*matrix[1][1] + z*matrix[1][2];
        v->z = x*matrix[2][0] + y*matrix[2][1] + z*matrix[2][2];
    }
}


void HANDLE_KEYBOARD(const bool* keystate,struct CAMERA* camera,struct MODEL* model){
    if(keystate[SDL_SCANCODE_W]){camera->pos.z += step;}
    if(keystate[SDL_SCANCODE_S]){camera->pos.z -= step;}
    if(keystate[SDL_SCANCODE_A]){camera->pos.x -= step;}
    if(keystate[SDL_SCANCODE_D]){camera->pos.x += step;}
    if(keystate[SDL_SCANCODE_E]){camera->pos.y += step;}
    if(keystate[SDL_SCANCODE_Q]){camera->pos.y -= step;}
    if(keystate[SDL_SCANCODE_R]){MODEL_ROTATE_Y(model,5);}
    if(keystate[SDL_SCANCODE_T]){MODEL_ROTATE_X(model,5);}
}

int main(int argc,char** argv){
    uint32_t* framebuffer = malloc(WIDTH*HEIGHT*sizeof(uint32_t));

    struct MODEL* model = GET_MODEL("model.obj",0,0,-10);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Software Rasterizer",WIDTH,HEIGHT,0);
    SDL_Surface* screen = SDL_GetWindowSurface(window);
    SDL_Renderer* renderer = SDL_CreateRenderer(window,NULL);

    struct CAMERA user_camera = {{0,0,-10},900};

    SDL_Event event;
    while(1){
        if(SDL_PollEvent(&event)){
            if(event.type==SDL_EVENT_QUIT){
                break;
            }
        }

        const bool* keystate = SDL_GetKeyboardState(NULL);
        HANDLE_KEYBOARD(keystate,&user_camera,model);
        memset(framebuffer, 0xFF, WIDTH*HEIGHT*sizeof(uint32_t));

        MODEL_DRAW(model,framebuffer,&user_camera);

        memcpy(screen->pixels,framebuffer,WIDTH * HEIGHT * sizeof(uint32_t));
        SDL_UpdateWindowSurface(window);
        SDL_Delay(16);
    }

    MODEL_DESTROY(model);
    free(framebuffer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
