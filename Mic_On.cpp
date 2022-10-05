//
// Image: Mic_On
// dimension: 32x18
// Size: 1kb
//

#include "Mic_On.h"

#define C(R,G,B) tgx::RGB565(R,G,B)

// image data
static const tgx::RGB565 Mic_On_data[32*18] PROGMEM = {
C(7,5,3), C(5,0,0), C(6,0,0), C(7,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), 
C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(8,0,0), C(7,0,0), C(6,0,0), C(5,0,0), C(7,5,3), 
C(6,1,1), C(6,0,0), C(8,0,0), C(10,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), 
C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(10,0,0), C(9,0,0), C(7,0,1), C(5,0,0), 
C(7,0,0), C(8,0,0), C(13,0,0), C(16,0,1), C(17,0,1), C(18,0,1), C(18,0,1), C(18,0,1), C(18,0,1), C(18,0,1), C(18,0,1), C(18,0,2), C(18,0,2), C(18,0,2), C(18,0,2), C(18,0,1), 
C(18,0,1), C(18,0,1), C(18,0,1), C(18,0,1), C(18,0,1), C(18,0,1), C(18,0,1), C(18,0,1), C(17,1,1), C(18,1,1), C(18,0,1), C(17,1,1), C(17,1,1), C(14,0,1), C(10,0,0), C(7,0,0), 
C(8,0,0), C(10,0,0), C(17,1,2), C(21,4,4), C(23,6,5), C(23,7,5), C(23,7,5), C(23,7,5), C(23,8,6), C(23,8,6), C(23,7,5), C(23,7,5), C(23,8,6), C(23,8,6), C(23,7,5), C(23,7,5), 
C(23,7,5), C(23,7,5), C(23,7,5), C(23,7,5), C(23,7,5), C(23,7,5), C(23,7,5), C(23,7,5), C(23,7,5), C(23,7,5), C(23,7,5), C(23,7,5), C(23,6,5), C(19,3,3), C(12,0,0), C(9,0,0), 
C(8,0,0), C(11,0,0), C(19,3,3), C(25,10,7), C(25,12,8), C(24,13,9), C(24,13,8), C(23,13,9), C(22,15,9), C(22,14,9), C(23,13,9), C(24,14,9), C(22,14,9), C(22,14,9), C(24,13,9), C(24,13,9), 
C(24,13,9), C(24,13,9), C(24,13,9), C(24,13,9), C(24,13,9), C(24,13,9), C(24,13,9), C(24,13,9), C(24,13,9), C(24,13,9), C(24,13,9), C(25,12,9), C(25,11,8), C(22,9,7), C(14,1,1), C(10,0,0), 
C(8,0,0), C(11,0,0), C(21,4,4), C(26,11,9), C(25,13,9), C(24,14,10), C(24,14,10), C(23,15,10), C(21,17,10), C(8,0,0), C(22,16,10), C(22,16,10), C(20,17,10), C(8,0,0), C(23,15,10), C(24,14,10), 
C(24,15,10), C(24,15,10), C(24,15,10), C(24,15,10), C(23,16,10), C(23,15,10), C(24,14,10), C(24,14,10), C(24,14,10), C(24,13,10), C(24,13,10), C(25,13,10), C(26,12,10), C(23,10,8), C(15,2,2), C(10,0,0), 
C(8,0,0), C(11,0,0), C(21,4,5), C(26,10,9), C(25,12,10), C(25,14,10), C(25,14,10), C(23,15,11), C(21,18,11), C(7,0,0), C(7,0,0), C(19,19,11), C(5,0,0), C(7,0,0), C(23,14,10), C(24,13,10), 
C(23,15,10), C(23,15,10), C(24,14,10), C(23,15,11), C(21,17,11), C(21,16,10), C(23,14,10), C(25,13,10), C(25,13,10), C(25,13,10), C(25,13,10), C(25,12,10), C(26,11,10), C(23,9,8), C(15,2,2), C(10,0,0), 
C(8,0,0), C(12,0,0), C(22,3,5), C(27,8,8), C(26,11,10), C(25,11,10), C(25,11,10), C(24,13,11), C(20,16,11), C(7,0,0), C(25,30,17), C(7,0,0), C(23,32,17), C(7,0,0), C(23,13,11), C(25,12,11), 
C(11,0,0), C(22,15,11), C(23,14,10), C(23,14,11), C(9,0,0), C(9,0,0), C(23,13,11), C(25,12,10), C(25,11,10), C(26,11,10), C(26,11,10), C(26,10,10), C(27,9,10), C(24,7,8), C(15,1,2), C(11,0,0), 
C(8,0,0), C(12,0,0), C(22,2,5), C(27,7,8), C(28,9,10), C(27,8,10), C(27,8,10), C(25,10,10), C(21,13,10), C(8,0,0), C(21,14,10), C(9,0,0), C(19,15,10), C(9,0,0), C(25,9,10), C(26,9,10), 
C(31,25,17), C(22,13,10), C(21,13,10), C(10,0,0), C(30,25,17), C(31,25,16), C(12,0,0), C(24,11,10), C(26,8,10), C(27,7,10), C(27,7,10), C(27,7,10), C(28,7,10), C(25,5,8), C(16,1,2), C(10,0,0), 
C(8,0,0), C(12,0,0), C(23,2,5), C(28,5,8), C(28,6,9), C(28,6,10), C(28,6,10), C(26,8,10), C(22,13,10), C(9,0,0), C(23,11,10), C(30,25,17), C(21,13,10), C(10,0,0), C(26,8,10), C(26,8,10), 
C(12,0,0), C(22,13,10), C(21,13,10), C(10,0,0), C(23,11,10), C(24,11,9), C(30,25,17), C(24,10,10), C(26,7,10), C(28,6,10), C(28,6,10), C(28,6,10), C(29,5,10), C(25,4,8), C(16,0,2), C(11,0,0), 
C(8,0,0), C(12,0,0), C(23,2,5), C(29,5,8), C(29,6,10), C(28,6,10), C(28,6,10), C(26,8,10), C(22,12,10), C(10,0,0), C(25,9,10), C(25,9,10), C(21,13,10), C(11,0,0), C(27,10,11), C(27,10,11), 
C(11,0,0), C(22,16,11), C(23,15,11), C(11,0,0), C(22,15,11), C(22,15,11), C(11,0,0), C(24,10,10), C(27,8,10), C(29,8,11), C(28,6,10), C(28,5,10), C(29,4,10), C(26,4,7), C(16,0,2), C(11,0,0), 
C(8,0,0), C(12,0,0), C(23,2,5), C(30,7,9), C(29,8,10), C(29,8,10), C(29,9,11), C(26,8,10), C(24,14,11), C(12,0,0), C(27,10,11), C(27,10,11), C(24,14,11), C(11,0,0), C(27,10,11), C(27,11,11), 
C(11,0,0), C(23,15,11), C(23,11,9), C(30,25,17), C(10,0,0), C(10,0,0), C(30,25,17), C(26,12,11), C(27,10,11), C(28,9,11), C(29,9,11), C(29,8,11), C(30,7,10), C(26,6,8), C(17,0,2), C(11,0,0), 
C(8,0,0), C(12,0,0), C(24,4,5), C(31,11,10), C(30,14,12), C(30,15,12), C(30,14,12), C(29,15,12), C(28,16,12), C(31,24,16), C(29,15,12), C(29,15,12), C(28,16,12), C(31,24,16), C(29,15,12), C(29,15,12), 
C(31,24,16), C(27,16,12), C(28,16,12), C(28,16,12), C(31,25,16), C(31,25,16), C(28,16,12), C(29,15,12), C(29,15,12), C(29,14,12), C(29,14,12), C(30,14,12), C(31,13,12), C(27,10,9), C(17,1,2), C(11,0,0), 
C(8,0,0), C(12,0,0), C(25,7,6), C(31,15,10), C(31,17,12), C(31,18,12), C(31,18,12), C(31,18,12), C(31,19,12), C(31,19,12), C(31,18,12), C(31,18,12), C(31,19,12), C(31,19,12), C(31,18,12), C(31,18,12), 
C(31,19,12), C(31,19,12), C(31,18,12), C(31,18,12), C(31,19,12), C(31,19,12), C(31,18,12), C(31,18,13), C(31,18,13), C(31,18,12), C(31,18,12), C(31,17,12), C(31,16,12), C(29,16,11), C(17,3,3), C(10,0,0), 
C(7,0,0), C(11,0,0), C(23,6,4), C(31,21,12), C(31,22,13), C(31,22,13), C(31,22,13), C(31,22,13), C(31,22,13), C(31,22,13), C(31,22,13), C(31,22,13), C(31,23,13), C(31,22,13), C(31,22,13), C(31,22,13), 
C(31,22,13), C(31,22,13), C(31,22,13), C(31,22,13), C(31,22,13), C(31,23,13), C(31,22,13), C(31,22,13), C(31,22,13), C(31,22,13), C(31,22,13), C(31,22,13), C(31,22,13), C(29,19,11), C(14,2,2), C(9,0,0), 
C(7,0,0), C(9,0,0), C(16,0,0), C(23,6,4), C(25,11,6), C(25,11,6), C(25,11,6), C(25,11,6), C(25,11,6), C(25,10,6), C(25,10,6), C(25,10,6), C(25,10,6), C(25,10,6), C(25,10,6), C(25,10,6), 
C(25,10,6), C(25,10,6), C(25,10,6), C(25,10,6), C(25,10,6), C(25,10,6), C(25,10,6), C(25,11,6), C(25,11,6), C(25,10,6), C(25,10,6), C(25,9,6), C(24,9,6), C(18,4,3), C(10,0,0), C(7,0,0), 
C(7,1,1), C(7,0,0), C(9,0,0), C(10,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), 
C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(11,0,0), C(10,0,0), C(10,0,0), C(9,0,0), C(7,0,0), C(8,5,3), 
C(8,5,3), C(7,1,1), C(8,1,1), C(8,1,0), C(9,1,0), C(9,1,0), C(9,1,0), C(9,1,0), C(9,1,0), C(9,1,0), C(9,1,0), C(9,1,0), C(9,1,0), C(9,1,0), C(9,1,0), C(9,1,0), 
C(9,1,0), C(9,1,0), C(9,1,0), C(9,1,0), C(8,1,0), C(8,1,0), C(9,1,1), C(9,1,1), C(9,1,0), C(9,1,0), C(9,1,1), C(8,1,0), C(8,1,0), C(7,1,1), C(6,2,1), C(6,5,3)
};

// image object
const tgx::Image<tgx::RGB565> Mic_On(Mic_On_data, 32, 18);

#undef C
// end of file Mic_On.cpp
