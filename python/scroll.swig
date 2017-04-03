%feature("docstring") ::Scroll
"""
Button module
""" ;

%feature("director") ::Scroll;

%{
#include <../include/scroll.h>
%}

CHANGE_OWNERSHIP(Scroll)

%include "../include/scroll.h"
