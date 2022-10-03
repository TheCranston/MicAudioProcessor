//
// Image: LITWHITEVU
// dimension: 10x1
// Size: 0kb
//

#include "LITWHITEVU.h"

#define C(R,G,B) tgx::RGB565(R,G,B)

// image data
static const tgx::RGB565 LITWHITEVU_data[13*1] PROGMEM = {
C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255)};

// image object
const tgx::Image<tgx::RGB565> LITWHITEVU(LITWHITEVU_data, 13, 1);

#undef C
// end of file LITWHITEVU.cpp
