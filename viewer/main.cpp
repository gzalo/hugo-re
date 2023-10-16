#include "Graphics.h"
#include "Renderer.h"
#include <string>
#include <sstream>
using namespace std;

int main([[maybe_unused]] int argc, [[maybe_unused]] char **args){
    Graphics graphics;
    graphics.init();

    freopen("out.txt", "w", stdout);

    // TIL
    Renderer renderer(&graphics, FileType::TIL);
    if(renderer.loadData("C:\\Users\\Gzalo\\Desktop\\tmp\\TIL\\OUTRO.TIL") == -1) return -1;
    if(renderer.loadPalette("C:\\Users\\Gzalo\\Desktop\\tmp\\TIL\\OUTRO.TIL", false) == -1) return -1;

    // PBR
    /*Renderer renderer(&graphics, FileType::PBR);
    if(renderer.loadData("C:\\Users\\Gzalo\\Downloads\\quickbms\\4hugo\\lumber\\grenr.pbr") == -1) return -1;
    if(renderer.loadPalette("C:\\Users\\Gzalo\\Downloads\\quickbms\\4hugo\\lumber\\lumpal.pic", false) == -1) return -1;
    graphics.zeroIsTransparent = true;*/

    // CBR
    /*Renderer renderer(&graphics, FileType::CBR);
    if(renderer.loadData("C:\\Users\\Gzalo\\Downloads\\quickbms\\4hugo\\menu\\joygfx.cbr") == -1) return -1;
    if(renderer.loadPalette("C:\\Users\\Gzalo\\Downloads\\quickbms\\4hugo\\menu\\joystx.raw", false) == -1) return -1;
    graphics.zeroIsTransparent = true;*/

    // BRS
    /*Renderer renderer(&graphics, FileType::BRS);
    if(renderer.loadData("C:\\Users\\Gzalo\\Downloads\\quickbms\\4hugo\\ice\\hopv3.brs") == -1) return -1;
    if(renderer.loadPalette("C:\\Users\\Gzalo\\Downloads\\quickbms\\4hugo\\ice\\hugostor.pic", false) == -1) return -1;
    graphics.zeroIsTransparent = true;*/

    renderer.init();

    bool quit = false;
    int currentFrame = 0;
    int totalFrames = renderer.getTotalFrames();

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)){
          if(event.type == SDL_QUIT){
              quit = true;
          }
            if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.sym == SDLK_LEFT)
                    currentFrame = (currentFrame-1+totalFrames)%totalFrames;
                if(event.key.keysym.sym == SDLK_RIGHT)
                    currentFrame = (currentFrame+1)%totalFrames;
            }
        }

        graphics.clear();
        renderer.render(currentFrame);

        stringstream title;
        title << "Frame " << currentFrame;
        graphics.render(title.str());
    }
    graphics.deinit();
    return EXIT_SUCCESS;
}
