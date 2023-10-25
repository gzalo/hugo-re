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
    if(renderer.loadData("Z:\\Gonzalo\\Proyectos\\2023-hugo\\HugoGoldFlashparty\\BigFile\\MenuData\\Arrows.cgf") == -1) return -1;
    if(renderer.loadPalPalette("Z:\\Gonzalo\\Proyectos\\2023-hugo\\HugoGoldFlashparty\\BigFile\\MenuData\\ok_mainpal.pal", true) == -1) return -1;

    renderer.init();

    bool quit = false;
    int currentFrame = 0;
    int totalFrames = renderer.getTotalFrames();

    //CgfCreator creator(&graphics);
    /*creator.loadFrame("0_46_101_38.png", 0, 46, 101, 38);
    creator.loadFrame("1_45_100_38.png", 1, 45, 100, 38);
    creator.loadFrame("2_8_49_38.png", 2, 8, 49, 38);
    creator.loadFrame("3_7_48_38.png", 3, 7, 48, 38);
    creator.loadFrame("4_99_52_38.png", 4, 99, 52, 38);
    creator.loadFrame("5_98_51_38.png", 5, 98, 51, 38);
    creator.loadFrame("6_49_11_38.png", 6, 49, 11, 38);
    creator.loadFrame("7_48_10_38.png", 7, 48, 10, 38);
    creator.save("test.cgf");*/

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
                /*if(event.key.keysym.sym == SDLK_SPACE)
                    renderer.save(currentFrame);*/
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

