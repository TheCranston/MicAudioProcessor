//
// Image: LITGREENVU
// dimension: 10x1
// Size: 0kb
//

#include "LITGREENVU.h"

#define C(R,G,B) tgx::RGB565(R,G,B)

// image data
static const tgx::RGB565 LITGREENVU_data[13*1] PROGMEM = {
C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0),C(0,255,0)};

// image object
const tgx::Image<tgx::RGB565> LITGREENVU(LITGREENVU_data, 13, 1);

#undef C
// end of file LITGREENVU.cpp
