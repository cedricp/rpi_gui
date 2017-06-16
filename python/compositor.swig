%feature("docstring") ::Compositor
"""
Compositor module
""" ;

%{
#include <../include/compositor.h>
#include <../include/keycode.h>
%}

%include "../include/compositor.h"
%include "../include/keycode.h"
