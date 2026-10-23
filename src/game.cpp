#include "game.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <string>
namespace oc {
bool Game::init(){
 if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)) return false;
 window=SDL_CreateWindow("OUTBREAK: CITY ZERO",ww,wh,SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY);
 if(!window) return false;
 renderer=SDL_CreateRenderer(window,nullptr); if(!renderer) return false;
 SDL_SetRenderVSync(renderer,1); loadAssets(); cells.resize(W*H); generate();
 camX=W*S/2.0f-ww/2.0f; camY=H*S/2.0f-wh/2.0f; return true;
}
SDL_Texture* Game::loadTexture(const char* name,bool keyed){
 const char* paths[] = {name, nullptr};
 std::string local=std::string("assets/")+name;
 SDL_Surface* surface=SDL_LoadBMP(local.c_str());
 if(!surface){std::string bundle=std::string("../Resources/assets/")+name;surface=SDL_LoadBMP(bundle.c_str());}
 if(!surface){std::string bundle=std::string("Resources/assets/")+name;surface=SDL_LoadBMP(bundle.c_str());}
 if(!surface)return nullptr;
 if(keyed) SDL_SetSurfaceColorKey(surface,true,SDL_MapRGB(SDL_GetPixelFormatDetails(surface->format),surface->format,38,65,47));
 SDL_Texture* texture=SDL_CreateTextureFromSurface(renderer,surface);SDL_DestroySurface(surface);return texture;
}
void Game::loadAssets(){
 grassTex=loadTexture("grass.bmp",false);roadTex=loadTexture("road.bmp",false);buildingTex=loadTexture("building.bmp",false);
 zombieTex=loadTexture("zombie.bmp",true);civilianTex=loadTexture("civilian.bmp",true);barricadeTex=loadTexture("barricade.bmp",true);turretTex=loadTexture("turret.bmp",true);
}
void Game::shutdown(){for(auto*t:{grassTex,roadTex,buildingTex,zombieTex,civilianTex,barricadeTex,turretTex})if(t)SDL_DestroyTexture(t);if(renderer)SDL_DestroyRenderer(renderer);if(window)SDL_DestroyWindow(window);SDL_Quit();}
bool Game::inside(int x,int y)const{return x>=0&&y>=0&&x<W&&y<H;}
Cell& Game::at(int x,int y){return cells[y*W+x];}
void Game::generate(){
 std::mt19937 r(27);
 for(int y=0;y<H;y++)for(int x=0;x<W;x++){bool road=(x%9==0)||(y%8==0);bool park=(x>12&&x<23&&y>12&&y<23)||(x>57&&x<68&&y>32&&y<43);at(x,y).building=!road&&!park&&(x%9!=1&&y%8!=1);}
 for(int i=0;i<150;i++){int x=r()%W,y=r()%H;if(!at(x,y).building)agents.push_back({(x+.5f)*S,(y+.5f)*S,0,0,25,false,true});}
 for(int i=0;i<42;i++){int x=r()%W,y=r()%H;if(!at(x,y).building)agents.push_back({(x+.5f)*S,(y+.5f)*S,0,0,18,true,true});}
}
SDL_FPoint Game::toWorld(float x,float y)const{return {(x-ww/2.0f)/zoom+camX+ww/2.0f,(y-wh/2.0f)/zoom+camY+wh/2.0f};}
SDL_FPoint Game::toScreen(float x,float y)const{return {(x-camX-ww/2.0f)*zoom+ww/2.0f,(y-camY-wh/2.0f)*zoom+wh/2.0f};}
SDL_FRect Game::rect(float x,float y,float w,float h)const{auto p=toScreen(x,y);return {p.x,p.y,w*zoom,h*zoom};}
void Game::color(Uint8 r,Uint8 g,Uint8 b,Uint8 a){SDL_SetRenderDrawColor(renderer,r,g,b,a);}
bool Game::blocked(int x,int y)const{
 if(!inside(x,y)||cells[y*W+x].building)return true;
 for(auto&w:walls)if((x>=std::min(w.x1,w.x2)&&x<=std::max(w.x1,w.x2)&&y==w.y1&&w.y1==w.y2)||(y>=std::min(w.y1,w.y2)&&y<=std::max(w.y1,w.y2)&&x==w.x1&&w.x1==w.x2))return true;
 for(auto&d:defenses)if(d.type==DefenseType::Barricade&&d.x==x&&d.y==y)return true;return false;
}
bool Game::placeDefense(int x,int y,DefenseType t){
 if(!inside(x,y)||at(x,y).building){notice="Can't place a defense inside a building";return false;}
 for(auto&d:defenses)if(d.x==x&&d.y==y){notice="That tile already has a defense";return false;}
 int cost=t==DefenseType::Barricade?12:35;if(supplies<cost){notice="Not enough supplies";return false;}
 defenses.push_back({x,y,t,t==DefenseType::Barricade?180.0f:100.0f,0});supplies-=cost;
 notice=t==DefenseType::Barricade?"Barricade placed — zombies must break through or go around":"Turret placed — automatically targets nearby zombies";return true;
}
void Game::event(const SDL_Event&e){
 if(e.type==SDL_EVENT_QUIT)running=false;
 if(e.type==SDL_EVENT_WINDOW_RESIZED){ww=e.window.data1;wh=e.window.data2;}
 if(e.type==SDL_EVENT_KEY_DOWN&&!e.key.repeat)switch(e.key.key){
 case SDLK_ESCAPE:dragging=false;notice="Placement cancelled";break;case SDLK_1:tool=Tool::Select;notice="Select / inspect";break;
 case SDLK_2:tool=Tool::Wall;notice="Optional wall: drag to place";break;case SDLK_3:tool=Tool::Zone;notice="Zone paint: drag across tiles";break;
 case SDLK_4:tool=Tool::Barricade;notice="Barricade mode: tap or drag to place";break;case SDLK_5:tool=Tool::Turret;notice="Turret mode: tap or drag to place";break;
 case SDLK_Z:zone=static_cast<Zone>((static_cast<int>(zone)+1)%4);notice="Zone type changed";break;case SDLK_SPACE:paused=!paused;break;
 case SDLK_EQUALS:case SDLK_PLUS:speed=std::min(8.0f,speed*2);break;case SDLK_MINUS:speed=std::max(.25f,speed*.5f);break;default:break;}
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
 if(tool==Tool::Wall){int len=std::max(bx-ax,by-ay)+1,cost=len*2;if(supplies<cost){notice="Not enough supplies";return;}walls.push_back({ax,ay,bx,by});supplies-=cost;notice="Optional wall placed";}
 else if(tool==Tool::Zone){for(int y=ay;y<=by;y++)for(int x=ax;x<=bx;x++)at(x,y).zone=zone;notice="Zone painted (zones do not block movement)";}
 else if(tool==Tool::Barricade||tool==Tool::Turret){int x=std::clamp(int(b.x/S),0,W-1),y=std::clamp(int(b.y/S),0,H-1);placeDefense(x,y,tool==Tool::Barricade?DefenseType::Barricade:DefenseType::Turret);}
 else{int n=0;for(auto&ag:agents)if(ag.alive&&ag.x/S>=ax&&ag.x/S<=bx&&ag.y/S>=ay&&ag.y/S<=by)n++;notice="Selected area contains "+std::to_string(n)+" agents";}
}
void Game::update(float dt){
 if(paused)return;dt=std::min(dt,.04f)*speed;
 for(auto&d:defenses){if(d.type!=DefenseType::Turret)continue;d.cooldown-=dt;if(d.cooldown>0)continue;Agent*target=nullptr;float best=180;
 for(auto&a:agents)if(a.alive&&a.zombie){float dist=std::hypot(a.x-(d.x+.5f)*S,a.y-(d.y+.5f)*S);if(dist<best){best=dist;target=&a;}}
 if(target){target->alive=false;d.cooldown=.8f;}}
 for(auto&a:agents)if(a.alive){
  Agent*target=nullptr;float best=140;if(a.zombie)for(auto&b:agents)if(b.alive&&!b.zombie){float dist=std::hypot(b.x-a.x,b.y-a.y);if(dist<best){best=dist;target=&b;}}
  if(target){float dist=std::max(1.0f,std::hypot(target->x-a.x,target->y-a.y));a.vx=(target->x-a.x)/dist*a.speed;a.vy=(target->y-a.y)/dist*a.speed;if(dist<9){target->zombie=true;target->speed=18;target->vx=target->vy=0;}}
  else if(!a.zombie){a.vx*=.94f;a.vy*=.94f;if(std::abs(a.vx)+std::abs(a.vy)<1){a.vx=std::sin(SDL_GetTicks()/1000.0f+a.x)*3;a.vy=std::cos(SDL_GetTicks()/1000.0f+a.y)*3;}}
  float nx=a.x+a.vx*dt,ny=a.y+a.vy*dt;int gx=int(nx/S),gy=int(ny/S);
  if(blocked(gx,gy)){bool hit=false;if(a.zombie)for(auto&d:defenses)if(d.type==DefenseType::Barricade&&d.x==gx&&d.y==gy){d.hp-=16*dt;hit=true;break;}if(!hit){a.vx=-a.vx*.5f;a.vy=-a.vy*.5f;}}
  else{a.x=std::clamp(nx,0.0f,W*S-1.0f);a.y=std::clamp(ny,0.0f,H*S-1.0f);}
 }
 defenses.erase(std::remove_if(defenses.begin(),defenses.end(),[&](const Defense&d){if(d.hp<=0){notice="A barricade was destroyed";return true;}return false;}),defenses.end());
 infected=0;for(auto&a:agents)if(a.alive&&a.zombie)infected++;
}
void Game::render(){
 color(15,20,22);SDL_RenderClear(renderer);
 for(int y=0;y<H;y++)for(int x=0;x<W;x++){auto r=rect(x*S,y*S,S-1,S-1);auto&c=at(x,y);SDL_Texture* tile=c.building?buildingTex:((x%9==0||y%8==0)?roadTex:grassTex);
 if(tile) SDL_RenderTexture(renderer,tile,nullptr,&r); else {if(c.building)color(61,67,69);else if(x%9==0||y%8==0)color(49,53,55);else color(38,65,47);SDL_RenderFillRect(renderer,&r);}
 if(c.building&&!buildingTex){auto roof=rect(x*S+3,y*S+3,S-7,S-7);color(74,79,81);SDL_RenderFillRect(renderer,&roof);}
 if(c.zone!=Zone::Normal){switch(c.zone){case Zone::Safe:color(35,165,105,95);break;case Zone::Evacuation:color(55,125,235,95);break;case Zone::Containment:color(220,70,55,95);break;default:break;}SDL_RenderFillRect(renderer,&r);}}
 for(auto&w:walls){auto a=toScreen((w.x1+.5f)*S,(w.y1+.5f)*S),b=toScreen((w.x2+.5f)*S,(w.y2+.5f)*S);color(239,190,85);SDL_RenderLine(renderer,a.x,a.y,b.x,b.y);}
 for(auto&d:defenses){auto p=toScreen((d.x+.5f)*S,(d.y+.5f)*S);float sz=std::max(5.0f,12*zoom);SDL_Texture* sprite=d.type==DefenseType::Barricade?barricadeTex:turretTex;SDL_FRect sr{p.x-sz,p.y-sz,sz*2,sz*2};if(sprite)SDL_RenderTexture(renderer,sprite,nullptr,&sr);else{color(201,148,72);SDL_RenderFillRect(renderer,&sr);}}
 for(auto&a:agents)if(a.alive){auto p=toScreen(a.x,a.y);float r=std::max(4.0f,8.0f*zoom);SDL_FRect f{p.x-r,p.y-r,r*2,r*2};SDL_Texture* sprite=a.zombie?zombieTex:civilianTex;if(sprite)SDL_RenderTexture(renderer,sprite,nullptr,&f);else{color(a.zombie?230:85,a.zombie?75:205,a.zombie?60:155);SDL_RenderFillRect(renderer,&f);}}
 color(12,17,21);SDL_FRect top{0,0,float(ww),78};SDL_RenderFillRect(renderer,&top);color(240,242,238);SDL_RenderDebugText(renderer,14,10,"OUTBREAK: CITY ZERO | OPEN CITY DEFENSE");
 std::string stats="SUPPLIES "+std::to_string(supplies)+"   ZOMBIES "+std::to_string(infected)+"   SPEED "+std::to_string(speed)+"x";SDL_RenderDebugText(renderer,14,29,stats.c_str());
 SDL_RenderDebugText(renderer,14,49,"1 SELECT  2 WALL  3 ZONES  4 BARRICADE  5 TURRET  SPACE PAUSE  +/- SPEED");
 color(12,17,21);SDL_FRect bot{0,float(wh-28),float(ww),28};SDL_RenderFillRect(renderer,&bot);color(230,235,230);SDL_RenderDebugText(renderer,14,wh-19,notice.c_str());
 if(dragging&&tool==Tool::Select){color(240,205,95);SDL_FRect r{std::min(sx,dx),std::min(sy,dy),std::abs(dx-sx),std::abs(dy-sy)};SDL_RenderRect(renderer,&r);}SDL_RenderPresent(renderer);
}
void Game::run(){Uint64 last=SDL_GetTicks();while(running){SDL_Event e;while(SDL_PollEvent(&e))event(e);Uint64 now=SDL_GetTicks();float dt=float(now-last)/1000.0f;last=now;update(dt);render();}}
}
