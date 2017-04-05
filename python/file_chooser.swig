%feature("docstring") ::File_chooser
"""
Button module
""" ;

%feature("director") ::File_chooser;

%{
#include <../include/file_chooser.h>
%}

CHANGE_OWNERSHIP(File_chooser)

%include "../include/file_chooser.h"
