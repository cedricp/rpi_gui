%feature("docstring") ::Button
"""
Button module
""" ;

%feature("director") ::Button;

%{
#include <../include/button.h>
%}

CHANGE_OWNERSHIP(Button)

%include "../include/button.h"
