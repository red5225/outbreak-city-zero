#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <vector>
namespace oc {
enum class Tool { Select, Wall, Zone };
enum class Zone { Normal, Safe, Evacuation, Containment };
struct Agent { float x,y,vx=0,vy=0,speed; bool zombie=false,alive=true; };
struct Cell { bool building=false; Zone zone=Zone::Normal; };
struct Wall { int x1,y1,x2,y2; };
class Game {
public: bool init(); void run(); void shutdown();
private:
 static constexpr int W=84,H=56,S=24;
 SDL_Window* window=nullptr; SDL_Renderer* renderer=nullptr;
 bool running=true,paused=false,dragging=false,panning=false;
 int ww=1280,wh=800,supplies=180,infected=42;
 float camX=0,camY=0,zoom=1,speed=1, sx=0,sy=0,dx=0,dy=0,px=0,py=0;
 Tool tool=Tool::Select; Zone zone=Zone::Containment;
 std::vector<Cell> cells; std::vector<Agent> agents; std::vector<Wall> walls;
 std::string notice="1 Select | 2 Walls | 3 Zones | Z cycle zones";
 void event(const SDL_Event&); void update(float); void render(); void generate();
 void dragAction(float,float,float,float); bool inside(int,int) const;
 Cell& at(int,int); SDL_FPoint toWorld(float,float) const; SDL_FPoint toScreen(float,float) const;
 SDL_FRect rect(float,float,float,float) const; void color(Uint8,Uint8,Uint8,Uint8=255);
};
}
