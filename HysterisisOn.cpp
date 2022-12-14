//
// Image: HysterisisOn
// dimension: 32x13
// Size: 1kb
//

#include "HysterisisOn.h"

#define C(R,G,B) tgx::RGB565(R,G,B)

// image data
static const tgx::RGB565 HysterisisOn_data[32*13] PROGMEM = {
C(0,2,2), C(0,10,6), C(0,11,7), C(0,13,8), C(0,14,9), C(0,15,9), C(0,15,9), C(0,15,9), C(0,16,9), C(0,16,9), C(0,15,9), C(0,15,9), C(0,16,9), C(0,16,9), C(0,15,9), C(0,15,9), 
C(0,16,9), C(0,16,9), C(0,15,9), C(0,15,9), C(0,16,9), C(0,16,9), C(0,15,9), C(0,15,9), C(0,16,9), C(0,16,9), C(0,15,9), C(0,15,9), C(0,14,8), C(0,12,8), C(0,10,7), C(0,3,3), 
C(1,9,6), C(1,13,8), C(7,35,19), C(4,36,20), C(8,49,27), C(7,51,27), C(7,50,27), C(7,50,27), C(7,50,27), C(7,50,27), C(8,49,27), C(8,49,27), C(7,50,27), C(7,50,27), C(8,50,27), C(8,49,27), 
C(7,50,27), C(7,50,27), C(7,50,27), C(7,50,27), C(7,50,27), C(7,50,27), C(7,50,27), C(7,51,27), C(7,51,27), C(7,51,27), C(7,51,27), C(8,50,27), C(6,44,24), C(8,42,23), C(3,23,13), C(0,11,7), 
C(0,9,6), C(0,15,9), C(6,39,22), C(6,49,27), C(4,53,29), C(3,55,30), C(3,55,30), C(3,55,30), C(4,55,30), C(5,54,29), C(6,52,28), C(4,47,26), C(4,54,29), C(4,54,29), C(4,47,26), C(6,52,28), 
C(4,54,29), C(4,55,30), C(4,55,30), C(4,55,30), C(4,55,30), C(3,55,30), C(3,55,30), C(3,55,30), C(3,55,29), C(3,55,30), C(3,55,30), C(4,54,30), C(5,53,29), C(6,46,26), C(2,26,15), C(0,12,8), 
C(0,10,6), C(0,15,10), C(4,39,23), C(4,50,29), C(2,54,30), C(0,55,30), C(0,55,30), C(1,55,31), C(1,54,30), C(3,53,29), C(6,51,28), C(0,16,11), C(3,53,29), C(3,53,29), C(0,16,11), C(6,51,28), 
C(4,53,29), C(2,54,30), C(2,54,30), C(2,54,30), C(1,54,30), C(1,55,30), C(1,55,31), C(1,55,30), C(0,55,31), C(0,55,31), C(1,55,31), C(1,54,31), C(2,53,30), C(4,46,26), C(1,26,15), C(0,12,8), 
C(0,10,7), C(0,15,10), C(3,39,23), C(3,49,28), C(2,52,30), C(1,53,30), C(1,53,31), C(1,53,31), C(2,53,30), C(3,51,29), C(7,48,27), C(0,14,10), C(4,51,29), C(4,51,29), C(0,14,11), C(7,48,27), 
C(5,47,27), C(5,50,28), C(5,50,28), C(3,48,28), C(2,52,30), C(1,53,30), C(1,53,30), C(1,53,31), C(1,53,30), C(1,53,30), C(1,53,31), C(2,53,30), C(2,51,30), C(3,45,26), C(1,25,16), C(0,12,8), 
C(0,10,7), C(0,15,10), C(3,37,22), C(3,47,28), C(2,51,30), C(0,50,29), C(0,50,30), C(0,50,30), C(1,49,29), C(3,48,28), C(6,45,26), C(0,13,10), C(5,46,27), C(8,54,31), C(0,13,10), C(8,44,26), 
C(0,13,10), C(7,45,26), C(7,45,26), C(0,14,11), C(3,48,28), C(1,50,29), C(1,50,30), C(0,50,30), C(0,50,30), C(0,50,30), C(1,50,30), C(1,49,29), C(2,49,29), C(4,43,26), C(1,25,15), C(0,11,8), 
C(0,10,7), C(0,15,10), C(3,36,22), C(3,45,28), C(2,48,29), C(1,49,30), C(1,49,30), C(1,49,30), C(1,49,30), C(3,47,28), C(7,44,26), C(0,12,10), C(0,13,10), C(0,13,10), C(0,12,10), C(8,43,25), 
C(0,11,9), C(9,43,25), C(10,42,24), C(0,12,10), C(4,47,28), C(2,48,30), C(2,49,30), C(1,49,30), C(1,49,30), C(1,48,30), C(1,49,30), C(2,48,30), C(3,47,29), C(4,42,25), C(1,24,15), C(0,11,8), 
C(0,10,7), C(0,15,10), C(3,35,22), C(4,44,28), C(3,47,29), C(2,47,29), C(2,47,29), C(2,47,29), C(2,47,29), C(4,46,28), C(7,44,26), C(0,12,10), C(8,43,26), C(8,43,26), C(0,12,10), C(9,43,25), 
C(13,50,28), C(0,10,8), C(0,9,8), C(0,10,9), C(5,46,28), C(3,48,29), C(2,48,29), C(2,47,29), C(2,48,30), C(2,47,29), C(2,47,29), C(3,47,29), C(4,46,29), C(6,44,27), C(2,23,15), C(0,10,8), 
C(0,9,6), C(0,14,10), C(4,35,22), C(5,44,28), C(5,47,29), C(4,48,29), C(4,48,29), C(4,48,29), C(4,47,29), C(5,47,28), C(8,45,26), C(0,12,10), C(7,45,27), C(7,45,27), C(0,12,10), C(8,43,25), 
C(10,42,25), C(12,44,25), C(13,43,25), C(0,10,9), C(6,46,27), C(4,48,29), C(4,48,29), C(4,48,29), C(3,47,29), C(3,48,29), C(4,47,29), C(4,47,29), C(5,46,29), C(6,42,25), C(2,23,15), C(0,10,8), 
C(1,9,6), C(1,14,9), C(7,36,22), C(9,45,27), C(8,46,28), C(8,49,29), C(8,49,29), C(8,49,29), C(8,49,29), C(9,48,28), C(10,47,27), C(12,51,29), C(9,48,28), C(9,48,28), C(12,51,29), C(11,46,27), 
C(0,10,8), C(0,10,8), C(0,9,8), C(14,49,28), C(9,48,28), C(8,49,29), C(8,49,29), C(8,49,29), C(7,49,29), C(7,49,29), C(8,49,29), C(8,48,29), C(8,46,27), C(9,42,25), C(4,24,15), C(0,9,7), 
C(2,8,5), C(2,13,8), C(11,36,21), C(10,38,22), C(13,47,27), C(14,50,28), C(13,50,28), C(13,50,28), C(13,49,28), C(14,49,28), C(14,49,27), C(14,49,27), C(14,50,27), C(14,50,27), C(14,49,27), C(14,49,27), 
C(15,49,27), C(15,49,27), C(15,48,27), C(15,49,27), C(14,50,28), C(13,50,28), C(13,50,28), C(13,50,28), C(13,50,28), C(13,50,28), C(14,49,28), C(12,44,25), C(13,44,25), C(4,24,15), C(6,23,13), C(0,8,6), 
C(4,11,6), C(2,8,5), C(0,8,6), C(0,9,6), C(0,10,7), C(0,10,7), C(0,10,7), C(0,11,7), C(0,10,8), C(0,11,8), C(0,11,8), C(0,11,8), C(0,11,7), C(0,11,7), C(0,10,7), C(0,10,7), 
C(0,10,7), C(0,10,7), C(0,10,7), C(0,10,7), C(0,10,7), C(0,11,7), C(0,11,8), C(0,11,8), C(0,11,8), C(0,11,8), C(0,11,8), C(0,10,8), C(0,10,8), C(0,10,7), C(0,9,6), C(2,12,7), 
C(0,0,0), C(0,0,1), C(0,0,1), C(0,1,1), C(0,1,1), C(0,1,1), C(0,1,1), C(0,2,2), C(0,2,3), C(5,23,13), C(4,24,14), C(4,24,14), C(5,23,13), C(0,2,3), C(0,1,1), C(0,1,1), 
C(0,1,1), C(0,1,1), C(0,1,1), C(0,1,1), C(0,1,1), C(0,2,2), C(5,24,14), C(4,24,15), C(4,24,15), C(4,24,15), C(4,24,15), C(4,24,15), C(4,24,15), C(5,23,14), C(6,22,13), C(0,2,3)
};

// image object
const tgx::Image<tgx::RGB565> HysterisisOn(HysterisisOn_data, 32, 13);

#undef C
// end of file HysterisisOn.cpp

