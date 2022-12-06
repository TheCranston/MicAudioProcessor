//
// Image: CompOn
// dimension: 12x12
// Size: 0kb
//

#include "CompOn.h"

#define C(R,G,B) tgx::RGB565(R,G,B)

// image data
static const tgx::RGB565 CompOn_data[12*12] PROGMEM = {
C(0,0,0), C(1,28,2), C(1,28,2), C(2,63,2), C(1,63,0), C(1,63,0), C(1,60,0), C(1,60,0), C(2,58,2), C(1,28,2), C(5,13,8), C(0,0,0), C(1,28,2), C(5,50,6), C(1,63,0), C(1,63,0), 
C(1,63,0), C(1,60,0), C(1,60,0), C(1,60,0), C(1,57,0), C(1,57,0), C(5,42,6), C(5,13,8), C(1,28,2), C(1,63,0), C(1,63,0), C(1,60,0), C(1,60,0), C(1,60,0), C(1,57,0), C(1,57,0), 
C(1,57,0), C(1,53,0), C(1,53,0), C(6,18,7), C(3,63,2), C(1,60,0), C(1,60,0), C(1,60,0), C(1,57,0), C(1,57,0), C(1,57,0), C(1,53,0), C(1,53,0), C(1,53,0), C(1,50,0), C(1,49,1), 
C(1,60,0), C(1,60,0), C(1,57,0), C(1,57,0), C(1,57,0), C(1,53,0), C(1,53,0), C(1,53,0), C(1,50,0), C(1,50,0), C(1,50,0), C(1,47,0), C(1,57,0), C(1,57,0), C(1,57,0), C(1,53,0), 
C(1,53,0), C(1,53,0), C(1,50,0), C(1,50,0), C(1,50,0), C(1,47,0), C(1,47,0), C(1,47,0), C(1,57,0), C(1,53,0), C(1,53,0), C(1,53,0), C(1,50,0), C(1,50,0), C(1,50,0), C(1,47,0), 
C(1,47,0), C(1,47,0), C(0,44,0), C(0,44,0), C(1,54,0), C(1,53,0), C(1,53,0), C(1,50,0), C(1,50,0), C(1,50,0), C(1,47,0), C(1,47,0), C(0,44,0), C(0,44,0), C(0,44,0), C(0,40,0), 
C(3,54,2), C(1,50,0), C(1,50,0), C(1,50,0), C(1,47,0), C(1,47,0), C(1,47,0), C(0,44,0), C(0,44,0), C(0,44,0), C(0,41,0), C(1,37,1), C(1,28,2), C(1,50,0), C(1,47,0), C(1,47,0), 
C(1,47,0), C(0,44,0), C(0,44,0), C(0,44,0), C(0,41,0), C(0,41,0), C(0,39,0), C(5,13,7), C(6,15,8), C(2,38,2), C(1,47,0), C(0,44,0), C(0,44,0), C(0,44,0), C(0,41,0), C(0,41,0), 
C(0,41,0), C(0,37,0), C(1,28,2), C(5,13,7), C(0,0,0), C(6,15,8), C(5,16,8), C(1,43,1), C(0,41,0), C(0,41,0), C(0,41,0), C(0,37,0), C(1,35,1), C(5,14,7), C(5,13,8), C(0,0,0)
};

// image object
const tgx::Image<tgx::RGB565> CompOn(CompOn_data, 12, 12);

#undef C
// end of file CompOn.cpp

