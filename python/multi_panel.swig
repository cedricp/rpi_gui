%feature("docstring") ::Multi_panel
"""
Button module
""" ;

%feature("director") ::Multi_panel;

%{
#include <../include/multi_panel.h>
%}

CHANGE_OWNERSHIP(Multi_panel)

%include "../include/multi_panel.h"
