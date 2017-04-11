%feature("docstring") ::Navit_widget
"""
Button module
""" ;

%feature("director") ::Navit_widget;

%{
#include <../include/navitwidget.h>
%}

CHANGE_OWNERSHIP(Navit_widget)

%include "../include/navitwidget.h"
