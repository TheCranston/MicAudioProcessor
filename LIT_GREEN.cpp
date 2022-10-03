//
// Image: LIT_GREEN
// dimension: 10x1
// Size: 0kb
//

#include "LITGREEN.h"

#define C(R,G,B) tgx::RGB565(R,G,B)

// image data
//static const tgx::RGB565 LITGREEN_data[10*1] PROGMEM = {
//C(16,63,0),C(16,63,0),C(16,63,0),C(16,63,0),C(16,63,0),C(16,63,0),C(16,63,0),C(16,63,0),C(16,63,0),C(16,63,0)};

static const tgx::RGB565 LITGREEN_data[10*1] PROGMEM = {
C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(2,255,0),C(2,255,0)};


// image object
const tgx::Image<tgx::RGB565> LITGREEN(LITGREEN_data, 10, 1);

#undef C
// end of file LITGREEN.cpp
