#include "Graphics.h"
#include "Renderer.h"
#include "CgfCreator.h"
#include <string>
#include <sstream>

using namespace std;

int main([[maybe_unused]] int argc, [[maybe_unused]] char **args){
    Graphics graphics;
    graphics.init();

    freopen("out.txt", "w", stdout);

    // TIL
    /*Renderer renderer(&graphics, FileType::TIL);
    if(renderer.loadData("Z:\\Gonzalo\\Proyectos\\2023-hugo\\HugoGoldFlashparty\\BigFile\\KeyData\\gfx\\STIKKEIN.TIL") == -1) return -1;
    if(renderer.loadPalette("Z:\\Gonzalo\\Proyectos\\2023-hugo\\HugoGoldFlashparty\\BigFile\\KeyData\\gfx\\STIKKEIN.TIL", false) == -1) return -1;*/

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

    // CGF+PAL
    Renderer renderer(&graphics, FileType::CGF);
    //if(renderer.loadData("Z:\\Gonzalo\\Proyectos\\2023-hugo\\HugoGoldFlashparty\\BigFile\\MenuData\\Arrows.cgf") == -1) return -1;
    if(renderer.loadData("Z:\\Gonzalo\\Proyectos\\2023-hugo\\HugoGoldFlashparty\\BigFile\\RopeOutroData\\gfx\\tele.cgf") == -1) return -1;
    if(renderer.loadPalette("Z:\\Gonzalo\\Proyectos\\2023-hugo\\HugoGoldFlashparty\\BigFile\\RopeOutroData\\gfx\\CASEDIE.til", false) == -1) return -1;
    //if(renderer.loadCgfPallete("Z:\\Gonzalo\\Proyectos\\2023-hugo\\HugoGoldFlashparty\\BigFile\\BoltData\\gfx\\PILE.cgf") == -1) return -1;

    // LZP
    /*Renderer renderer(&graphics, FileType::LZP);
    if(renderer.loadData("Z:\\Gonzalo\\Juegos\\Hugo\\A jugar con Hugo - Edicion Platino (v1.0)\\BigFile\\DolmenData\\Graphics\\Tunnels\\rope-up+wall.lzp") == -1) return -1;
    if(renderer.loadPalette("Z:\\Gonzalo\\Juegos\\Hugo\\A jugar con Hugo - Edicion Platino (v1.0)\\BigFile\\DolmenData\\Graphics\\Tunnels\\rope-up+wall.lzp", false) == -1) return -1;*/

    renderer.init();

    bool quit = false;
    int currentFrame = 0;
    int totalFrames = renderer.getTotalFrames();

    CgfCreator creator(&graphics);
    creator.loadFrame("0_15_18_46.png", 0, 15, 18, 46);
    creator.loadFrame("1_15_18_46.png", 1, 15, 18, 46);
    creator.loadFrame("2_15_18_46.png", 2, 15, 18, 46);
    creator.loadFrame("3_15_18_46.png", 3, 15, 18, 46);
    creator.loadFrame("4_15_18_46.png", 4, 15, 18, 46);
    creator.loadFrame("5_15_18_46.png", 5, 15, 18, 46);
    if(creator.save("tele.cgf", 256, 0) != 0) return -1;

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
                if(event.key.keysym.sym == SDLK_SPACE)
                    renderer.save(currentFrame);
            }
        }

        graphics.clear();
        renderer.render(currentFrame);

        stringstream title;
        title << "Frame " << currentFrame << "/" << totalFrames;
        graphics.render(title.str());
    }
    graphics.deinit();
    return EXIT_SUCCESS;
}

