
%module hw

%include <stdint.i>

%{
#include <../hardware/include/rda5807fm.h>
#include <../hardware/include/tda7419.h>
#include <../hardware/include/lcd.h>
%}

%include "../hardware/include/rda5807fm.h"
%include <../hardware/include/tda7419.h>
%include <../hardware/include/lcd.h>
