//
// Image: Limit_On
// dimension: 31x18
// Size: 1kb
//

#include "Limit_On.h"

#define C(R,G,B) tgx::RGB565(R,G,B)

// image data
static const tgx::RGB565 Limit_On_data[31*18] PROGMEM = {
C(1,5,3), C(1,7,4), C(0,8,5), C(0,9,6), C(0,9,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,9,6), C(0,10,6), 
C(0,10,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,10,6), C(0,9,6), C(0,9,5), C(1,8,5), C(1,5,4), C(2,6,4), C(1,8,5), 
C(1,11,6), C(0,15,8), C(0,17,10), C(0,17,10), C(0,17,10), C(0,17,10), C(0,17,10), C(0,17,10), C(0,17,10), C(0,17,10), C(0,17,10), C(0,17,10), C(0,18,10), C(0,17,10), C(0,17,10), C(0,17,10), 
C(0,17,10), C(0,17,10), C(0,17,10), C(0,18,10), C(0,18,10), C(0,17,10), C(0,17,10), C(0,18,10), C(0,18,10), C(0,17,10), C(0,16,9), C(1,13,8), C(1,10,6), C(2,6,4), C(0,11,7), C(1,19,11), 
C(1,28,16), C(0,32,18), C(0,33,18), C(0,33,18), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), 
C(0,34,19), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), C(0,33,19), C(1,30,17), C(1,24,14), C(0,15,8), C(1,8,5), C(0,14,9), C(3,29,16), C(3,42,23), 
C(2,47,26), C(2,47,26), C(2,47,26), C(1,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), 
C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(2,48,26), C(3,45,25), C(3,37,20), C(0,20,12), C(1,9,5), C(0,16,10), C(4,37,21), C(4,51,28), C(2,55,30), 
C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), 
C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(1,55,30), C(2,55,30), C(3,53,29), C(4,45,25), C(0,25,15), C(0,9,6), C(0,18,11), C(4,40,22), C(3,53,29), C(0,56,31), C(0,57,31), 
C(1,57,31), C(2,56,30), C(3,56,30), C(1,57,31), C(1,57,31), C(2,56,30), C(2,55,30), C(2,56,30), C(2,56,30), C(2,56,30), C(1,55,30), C(1,55,30), C(1,55,30), C(2,54,30), C(2,55,30), C(2,56,30), 
C(2,56,30), C(3,54,29), C(3,55,29), C(1,56,30), C(0,57,31), C(1,57,31), C(2,55,30), C(4,48,27), C(0,28,16), C(0,10,6), C(0,17,12), C(4,39,23), C(2,52,29), C(1,55,31), C(0,55,31), C(2,54,30), 
C(0,11,8), C(6,51,28), C(2,54,30), C(1,54,30), C(4,53,29), C(0,12,8), C(4,52,28), C(4,53,28), C(4,53,29), C(4,53,29), C(4,52,29), C(4,52,28), C(5,52,28), C(0,12,8), C(4,52,28), C(6,51,27), 
C(0,8,6), C(8,49,26), C(3,53,29), C(0,55,30), C(0,55,31), C(2,54,30), C(3,47,27), C(0,27,17), C(0,10,7), C(0,17,12), C(4,38,22), C(2,49,28), C(1,53,31), C(0,54,31), C(3,52,29), C(0,9,7), 
C(8,48,26), C(3,52,29), C(2,52,29), C(6,49,27), C(12,57,31), C(6,49,27), C(0,10,8), C(0,10,8), C(6,49,27), C(0,10,8), C(7,49,26), C(7,48,26), C(11,57,31), C(5,50,28), C(0,10,8), C(0,6,5), 
C(0,7,5), C(4,51,28), C(0,53,30), C(0,54,31), C(2,51,30), C(3,45,27), C(0,26,17), C(0,10,7), C(0,17,12), C(3,36,22), C(2,47,28), C(1,51,31), C(1,49,30), C(3,48,28), C(0,8,7), C(7,44,25), 
C(3,48,28), C(3,48,28), C(7,45,26), C(0,7,6), C(8,44,24), C(0,8,6), C(14,55,29), C(0,7,6), C(15,55,29), C(0,7,6), C(9,43,24), C(0,8,7), C(4,47,27), C(11,58,31), C(0,6,6), C(14,55,30), 
C(3,47,28), C(1,50,30), C(0,50,30), C(2,49,29), C(3,43,26), C(0,25,17), C(0,10,7), C(0,16,12), C(4,35,22), C(3,46,28), C(1,48,30), C(1,49,30), C(4,47,28), C(0,7,7), C(9,43,24), C(5,46,27), 
C(5,46,27), C(8,43,25), C(0,6,6), C(10,42,24), C(0,6,6), C(9,42,24), C(0,6,6), C(10,42,23), C(0,6,5), C(10,42,23), C(0,7,7), C(4,46,28), C(5,46,27), C(0,7,6), C(9,43,24), C(4,47,28), 
C(1,49,30), C(1,49,30), C(2,47,29), C(3,42,26), C(0,25,16), C(0,10,7), C(0,16,12), C(4,34,22), C(4,45,28), C(2,47,29), C(2,47,29), C(4,46,27), C(0,6,6), C(11,41,22), C(9,42,24), C(8,43,25), 
C(9,42,24), C(0,6,6), C(10,42,23), C(0,7,7), C(6,44,26), C(13,56,31), C(9,42,24), C(0,6,6), C(10,42,23), C(0,7,7), C(4,46,28), C(4,46,27), C(0,7,6), C(9,43,24), C(4,46,28), C(2,47,29), 
C(2,47,29), C(3,46,29), C(5,44,27), C(0,24,16), C(0,9,7), C(0,15,12), C(5,34,21), C(5,45,28), C(3,48,29), C(3,48,29), C(6,46,28), C(0,6,6), C(0,5,5), C(0,6,6), C(0,7,6), C(9,44,25), 
C(0,7,6), C(9,43,25), C(0,7,8), C(6,46,28), C(6,46,28), C(8,45,26), C(0,7,7), C(9,44,25), C(0,8,8), C(5,47,28), C(5,47,28), C(14,55,31), C(0,7,7), C(5,47,28), C(3,48,29), C(3,48,29), 
C(5,47,29), C(5,42,26), C(1,24,16), C(0,9,7), C(0,15,11), C(7,36,22), C(7,47,28), C(6,50,30), C(6,50,30), C(7,49,29), C(13,56,31), C(14,55,30), C(13,56,31), C(13,56,31), C(9,48,28), C(12,56,31), 
C(9,48,28), C(12,56,31), C(7,49,29), C(7,49,29), C(8,49,28), C(12,56,31), C(8,49,28), C(8,49,29), C(7,50,30), C(7,50,29), C(8,48,28), C(12,56,31), C(7,50,29), C(6,50,30), C(6,50,30), C(7,49,29), 
C(7,43,26), C(3,25,16), C(0,9,6), C(0,15,10), C(9,38,22), C(10,49,28), C(10,52,30), C(10,53,31), C(10,53,30), C(10,52,30), C(11,52,29), C(11,52,29), C(11,52,29), C(10,52,30), C(10,53,30), C(10,53,30), 
C(10,53,30), C(10,53,30), C(10,53,30), C(10,53,30), C(10,52,30), C(10,53,30), C(10,53,31), C(10,53,31), C(10,53,30), C(10,53,30), C(10,53,30), C(10,53,30), C(10,53,30), C(9,52,30), C(10,51,29), C(10,45,26), 
C(4,25,16), C(0,8,6), C(0,11,8), C(10,36,20), C(15,52,29), C(14,55,31), C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,30), 
C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,31), C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,30), C(14,55,30), C(14,54,30), C(14,47,26), C(4,20,13), 
C(1,7,5), C(0,8,5), C(4,18,10), C(11,37,20), C(11,41,23), C(11,41,22), C(11,42,22), C(11,42,22), C(11,42,22), C(11,41,22), C(11,41,22), C(11,41,22), C(11,41,22), C(11,41,22), C(11,41,22), C(11,41,22), 
C(11,41,22), C(11,41,23), C(11,41,22), C(11,41,23), C(11,41,22), C(11,41,23), C(11,41,22), C(11,41,22), C(11,41,22), C(11,41,22), C(11,41,22), C(11,41,22), C(11,40,22), C(7,27,15), C(1,10,7), C(2,6,5), 
C(2,7,4), C(2,7,5), C(1,9,6), C(1,10,6), C(1,10,6), C(1,10,6), C(1,10,6), C(1,10,6), C(1,10,6), C(1,10,6), C(1,10,6), C(1,10,6), C(1,10,6), C(1,10,6), C(1,10,6), C(1,10,6), 
C(1,10,6), C(1,10,6), C(1,9,6), C(1,9,6), C(1,10,6), C(1,10,6), C(1,10,6), C(1,10,6), C(1,9,6), C(1,10,6), C(1,9,6), C(1,9,6), C(1,7,5), C(2,7,4), C(3,7,5), C(4,8,5), 
C(4,8,5), C(3,9,5), C(3,10,5), C(3,10,5), C(3,10,5), C(3,9,6), C(3,9,6), C(3,10,5), C(3,10,5), C(3,9,6), C(3,9,6), C(3,10,5), C(3,10,5), C(3,10,6), C(3,10,6), C(3,10,6), 
C(3,10,6), C(3,9,6), C(3,9,6), C(3,9,6), C(3,9,6), C(3,9,6), C(3,9,6), C(4,10,6), C(3,9,6), C(3,9,6), C(4,9,5), C(3,9,5), C(4,8,4), C(4,8,5)};

// image object
const tgx::Image<tgx::RGB565> Limit_On(Limit_On_data, 31, 18);

#undef C
// end of file Limit_On.cpp

