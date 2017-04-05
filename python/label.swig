%feature("docstring") ::Label
"""
Button module
""" ;

%feature("director") ::Label;

%{
#include <../include/label.h>
%}

CHANGE_OWNERSHIP(Label)

%include "../include/label.h"
