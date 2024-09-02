
#load sdl2 dll
wget https://github.com/libsdl-org/SDL/releases/download/release-2.30.5/SDL2-2.30.5-win32-x64.zip
unzip SDL2-2.30.5-win32-x64.zip
rm SDL2-2.30.5-win32-x64.zip README-SDL.txt

#load sdl2 ttf dll
wget https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-2.22.0-win32-x64.zip
unzip SDL2_ttf-2.22.0-win32-x64.zip
rm SDL2_ttf-2.22.0-win32-x64.zip README.txt
mkdir -p license
mv LICENSE.* license