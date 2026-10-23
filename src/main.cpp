#include "game.hpp"
#include <cstdio>
int main(){oc::Game g;if(!g.init()){std::fprintf(stderr,"SDL3 initialization failed\n");return 1;}g.run();g.shutdown();return 0;}
