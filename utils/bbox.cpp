#include "bbox.h"


std::ostream& operator<< (std::ostream & out, IBbox const& data)
{
    out << "Bounding box { xmin " << data.xmin() << ", xmax " << data.xmax() <<  ", ymin " << data.ymin() << ", ymax " << data.ymax() << " }";
    return out;
}

std::ostream& operator<< (std::ostream & out, FBbox const& data)
{
    out << "Bounding box { " << data.xmin() << ", " << data.xmax() <<  ", " << data.ymin() << ", " << data.ymax() << " }";
    return out;
}
