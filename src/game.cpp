#include "game.hpp"
#include <algorithm>
#include <cmath>
#include <random>
namespace oc {
bool Game::init(){
 if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS))return false;
 window=SDL_CreateWindow("OUTBREAK: CITY ZERO",ww,wh,SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY);if(!window)return false;
 renderer=SDL_CreateRenderer(window,nullptr);if(!renderer)return false;SDL_SetRenderVSync(renderer,1);
 cells.resize(W*H);generate();camX=W*S/2.0f-ww/2.0f;camY=H*S/2.0f-wh/2.0f;return true;
}
void Game::shutdown(){if(renderer)SDL_DestroyRenderer(renderer);if(window)SDL_DestroyWindow(window);SDL_Quit();}
bool Game::inside(int x,int y)const{return x>=0&&y>=0&&x<W&&y<H;}
Cell& Game::at(int x,int y){return cells[y*W+x];}
void Game::generate(){
 std::mt19937 r(27);for(int y=0;y<H;y++)for(int x=0;x<W;x++){bool road=x%9==0||y%8==0;bool park=(x>12&&x<22&&y>12&&y<23)||(x>57&&x<68&&y>32&&y<43);at(x,y).building=!road&&!park&&x%9!=1&&y%8!=1;}
 for(int i=0;i<150;i++){int x=r()%W,y=r()%H;if(!at(x,y).building)agents.push_back({(x+.5f)*S,(y+.5f)*S,0,0,25,false,true});}
 for(int i=0;i<42;i++){int x=r()%W,y=r()%H;if(!at(x,y).building)agents.push_back({(x+.5f)*S,(y+.5f)*S,0,0,18,true,true});}
}
SDL_FPoint Game::toWorld(float x,float y)const{return {(x-ww/2.0f)/zoom+camX+ww/2.0f,(y-wh/2.0f)/zoom+camY+wh/2.0f};}
SDL_FPoint Game::toScreen(float x,float y)const{return {(x-camX-ww/2.0f)*zoom+ww/2.0f,(y-camY-wh/2.0f)*zoom+wh/2.0f};}
SDL_FRect Game::rect(float x,float y,float w,float h)const{auto p=toScreen(x,y);return {p.x,p.y,w*zoom,h*zoom};}
void Game::color(Uint8 r,Uint8 g,Uint8 b,Uint8 a){SDL_SetRenderDrawColor(renderer,r,g,b,a);}
void Game::event(const SDL_Event&e){
 if(e.type==SDL_EVENT_QUIT)running=false;
 if(e.type==SDL_EVENT_WINDOW_RESIZED){ww=e.window.data1;wh=e.window.data2;}
 if(e.type==SDL_EVENT_KEY_DOWN&&!e.key.repeat){switch(e.key.key){case SDLK_ESCAPE:dragging=false;notice="Cancelled";break;case SDLK_1:tool=Tool::Select;notice="Select mode";break;case SDLK_2:tool=Tool::Wall;notice="Wall mode: drag to build";break;case SDLK_3:tool=Tool::Zone;notice="Zone mode: drag to paint";break;case SDLK_z:zone=static_cast<Zone>((static_cast<int>(zone)+1)%4);notice="Zone type changed";break;case SDLK_SPACE:paused=!paused;break;case SDLK_EQUALS:case SDLK_PLUS:speed=std::min(8.0f,speed*2);break;case SDLK_MINUS:speed=std::max(.25f,speed*.5f);break;default:break;}}
 if(e.type==SDL_EVENT_MOUSE_WHEEL)zoom=std::clamp(zoom*(e.wheel.y>0?1.12f:.89f),.45f,2.4f);
 if(e.type==SDL_EVENT_MOUSE_BUTTON_DOWN){float x=e.button.x,y=e.button.y;if(e.button.button==SDL_BUTTON_RIGHT||e.button.button==SDL_BUTTON_MIDDLE){panning=true;px=x;py=y;}else if(e.button.button==SDL_BUTTON_LEFT){dragging=true;sx=dx=x;sy=dy=y;}}
 if(e.type==SDL_EVENT_MOUSE_BUTTON_UP){if(e.button.button==SDL_BUTTON_RIGHT||e.button.button==SDL_BUTTON_MIDDLE)panning=false;if(e.button.button==SDL_BUTTON_LEFT&&dragging){dragAction(sx,sy,e.button.x,e.button.y);dragging=false;}}
 if(e.type==SDL_EVENT_MOUSE_MOTION){if(panning){camX-=(e.motion.x-px)/zoom;camY-=(e.motion.y-py)/zoom;px=e.motion.x;py=e.motion.y;}if(dragging){dx=e.motion.x;dy=e.motion.y;}}
 if(e.type==SDL_EVENT_FINGER_DOWN){dragging=true;sx=dx=e.tfinger.x*ww;sy=dy=e.tfinger.y*wh;}
 if(e.type==SDL_EVENT_FINGER_MOTION&&dragging){dx=e.tfinger.x*ww;dy=e.tfinger.y*wh;}
 if(e.type==SDL_EVENT_FINGER_UP&&dragging){dragAction(sx,sy,e.tfinger.x*ww,e.tfinger.y*wh);dragging=false;}
}
void Game::dragAction(float x1,float y1,float x2,float y2){
 auto a=toWorld(x1,y1),b=toWorld(x2,y2);int ax=std::clamp(int(std::min(a.x,b.x)/S),0,W-1),bx=std::clamp(int(std::max(a.x,b.x)/S),0,W-1),ay=std::clamp(int(std::min(a.y,b.y)/S),0,H-1),by=std::clamp(int(std::max(a.y,b.y)/S),0,H-1);
 if(tool==Tool::Wall){bool horizontal=std::abs(bx-ax)>=std::abs(by-ay);int len=horizontal?std::abs(bx-ax)+1:std::abs(by-ay)+1,cost=len*2;if(supplies<cost){notice="Not enough supplies";return;}walls.push_back(horizontal?Wall{ax,ay,bx,ay}:Wall{ax,ay,ax,by});supplies-=cost;notice="Wall built: "+std::to_string(cost)+" supplies";}
 else if(tool==Tool::Zone){for(int y=ay;y<=by;y++)for(int x=ax;x<=bx;x++)at(x,y).zone=zone;notice="Zone painted";}
 else {int n=0;for(auto&a:agents)if(a.alive&&int(a.x/S)>=ax&&int(a.x/S)<=bx&&int(a.y/S)>=ay&&int(a.y/S)<=by)n++;notice="Selected area contains "+std::to_string(n)+" agents";}
}
void Game::update(float dt){
 if(paused)return;dt=std::min(dt,.04f)*speed;
 for(auto&a:agents)if(a.alive){Agent*t=nullptr;float best=110;for(auto&b:agents)if(b.alive&&b.zombie!=a.zombie){float d=std::hypot(b.x-a.x,b.y-a.y);if(d<best){best=d;t=&b;}}
 if(t&&best>.1f){float inv=1/std::max(.1f,std::hypot(t->x-a.x,t->y-a.y)),dir=a.zombie?1.0f:-1.0f;a.vx=(t->x-a.x)*inv*a.speed*dir;a.vy=(t->y-a.y)*inv*a.speed*dir;}
 else {a.vx*=.9f;a.vy*=.9f;}
 float nx=a.x+a.vx*dt,ny=a.y+a.vy*dt;int gx=int(nx/S),gy=int(ny/S);bool block=!inside(gx,gy)||(inside(gx,gy)&&at(gx,gy).building);
 for(auto&w:walls)if((gx>=std::min(w.x1,w.x2)&&gx<=std::max(w.x1,w.x2)&&gy==w.y1)||(gy>=std::min(w.y1,w.y2)&&gy<=std::max(w.y1,w.y2)&&gx==w.x1))block=true;
 if(!block){a.x=nx;a.y=ny;}
 if(a.zombie&&t&&best<9){t->zombie=true;t->speed=18;}
 }
 infected=0;for(auto&a:agents)if(a.alive&&a.zombie)infected++;
}
void Game::render(){
 color(21,28,31);SDL_RenderClear(renderer);
 for(int y=0;y<H;y++)for(int x=0;x<W;x++){auto r=rect(x*S,y*S,S-1,S-1);auto&c=at(x,y);if(c.building)color(67,75,78);else if(x%9==0||y%8==0)color(43,49,52);else color(39,61,46);SDL_RenderFillRect(renderer,&r);
 switch(c.zone){case Zone::Safe:color(35,165,105,105);break;case Zone::Evacuation:color(55,125,235,105);break;case Zone::Containment:color(220,70,55,90);break;default:continue;}SDL_RenderFillRect(renderer,&r);}
 for(auto&w:walls){auto a=toScreen((w.x1+.5f)*S,(w.y1+.5f)*S),b=toScreen((w.x2+.5f)*S,(w.y2+.5f)*S);color(245,190,85);SDL_RenderLine(renderer,a.x,a.y,b.x,b.y);}
 for(auto&a:agents)if(a.alive){auto p=toScreen(a.x,a.y);color(a.zombie?235:70,a.zombie?65:205,a.zombie?55:140);float r=std::max(2.0f,3.2f*zoom);SDL_FRect q{p.x-r,p.y-r,2*r,2*r};SDL_RenderFillRect(renderer,&q);}
 color(12,17,21);SDL_FRect top{0,0,float(ww),56};SDL_RenderFillRect(renderer,&top);color(235,240,235);SDL_RenderDebugText(renderer,14,10,"OUTBREAK: CITY ZERO");std::string stats="SUPPLIES "+std::to_string(supplies)+"  INFECTED "+std::to_string(infected)+"  SPEED "+std::to_string(speed)+"x";SDL_RenderDebugText(renderer,14,29,stats.c_str());
 SDL_FRect bottom{0,float(wh-48),float(ww),48};color(12,17,21);SDL_RenderFillRect(renderer,&bottom);color(235,240,235);SDL_RenderDebugText(renderer,14,wh-36,"1 SELECT | 2 WALLS | 3 ZONES | Z CYCLE | SPACE PAUSE | +/- SPEED");SDL_RenderDebugText(renderer,14,wh-18,notice.c_str());
 if(dragging){SDL_FRect r{std::min(sx,dx),std::min(sy,dy),std::abs(dx-sx),std::abs(dy-sy)};color(240,205,95);SDL_RenderRect(renderer,&r);}SDL_RenderPresent(renderer);
}
void Game::run(){Uint64 last=SDL_GetTicks();while(running){SDL_Event e;while(SDL_PollEvent(&e))event(e);Uint64 now=SDL_GetTicks();float dt=float(now-last)/1000;last=now;update(dt);render();}}
}
