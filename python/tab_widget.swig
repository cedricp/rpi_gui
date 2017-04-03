%feature("docstring") ::Tab_widget
"""
Button module
""" ;

%feature("director") ::Tab_widget;

%{
#include <../include/tab_widget.h>
%}

CHANGE_OWNERSHIP(Tab_widget)

%include "../include/tab_widget.h"
