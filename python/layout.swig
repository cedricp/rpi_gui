%feature("docstring") ::Layout
"""
Button module
""" ;

%feature("director") ::Layout;

%{
#include <../include/layout.h>
%}

CHANGE_OWNERSHIP(Layout)

%include "../include/layout.h"
