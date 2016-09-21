#include "matrix.h"

std::ostream& operator << (std::ostream & out, Matrix m)
{
	out << "Matrix : {" << std::endl;
	for (int i = 0; i < 16; ++i){
		if (i != 0 && i%4 == 0) out << std::endl;
		out << m[i] << ",";
	}
	out << "}" << std::endl;
}
