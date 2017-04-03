%feature("docstring") ::Slider
"""
Button module
""" ;

%feature("director") ::Slider;

%{
#include <../include/slider.h>
%}

CHANGE_OWNERSHIP(Slider)

%include "../include/slider.h"
