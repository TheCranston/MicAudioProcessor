//
// Image: LIT_GREEN
// dimension: 10x1
// Size: 0kb
//

#include "LITWHITE.h"

#define C(R,G,B) tgx::RGB565(R,G,B)

// image data
static const tgx::RGB565 LITWHITE_data[10*1] PROGMEM = {
C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255),C(255,255,255)};

// image object
const tgx::Image<tgx::RGB565> LITWHITE(LITWHITE_data, 10, 1);

#undef C
// end of file LITWHITE.cpp
