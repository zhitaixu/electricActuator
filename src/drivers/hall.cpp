#include "drivers/hall.h"

static int _u,_v,_w;

void HallReader::begin(int pinU, int pinV, int pinW){
  _u=pinU; _v=pinV; _w=pinW;
  pinMode(_u, INPUT);
  pinMode(_v, INPUT);
  pinMode(_w, INPUT);
}

HallSample HallReader::read(){
  HallSample s{};
  s.u = digitalRead(_u) ? 1:0;
  s.v = digitalRead(_v) ? 1:0;
  s.w = digitalRead(_w) ? 1:0;
  s.code = (s.u<<2) | (s.v<<1) | (s.w);
  return s;
}

bool HallReader::isValid(uint8_t code) const {
  switch(code){
    case 0b001:
    case 0b010:
    case 0b011:
    case 0b100:
    case 0b101:
    case 0b110:
      return true;
    default:
      return false;
  }
}