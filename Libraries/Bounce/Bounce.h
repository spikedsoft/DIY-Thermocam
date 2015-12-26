#ifndef Bounce_h
#define Bounce_h

#include <inttypes.h>

class Bounce
{

public:
  Bounce(uint8_t pin, unsigned long interval_millis ); 
  void interval(unsigned long interval_millis); 
  int update(); 
  void rebounce(unsigned long interval); 
  int read();
  void write(int new_state);
  unsigned long duration();
	bool risingEdge();
	bool fallingEdge();
  
protected:
  int debounce();
  unsigned long  previous_millis, interval_millis, rebounce_millis;
  uint8_t state;
  uint8_t pin;
  uint8_t stateChanged;
};

#endif


