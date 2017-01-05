#ifndef COLOR_H
#define COLOR_H

template <class T>
class Color{
    T m_red, m_green, m_blue, m_alpha;
public:
    Color(){
        m_red     = 0;
        m_green   = 0;
        m_blue    = 0;
        m_alpha   = 0;
    }
    
    Color(T r, T g, T b, T a){
        m_red     = r;
        m_green   = g;
        m_blue    = b;
        m_alpha   = a;
    }
    
    Color(const Color<T>& color){
        m_red     = color.m_red;
        m_green   = color.m_green;
        m_blue    = color.m_blue;
        m_alpha   = color.m_alpha; 
    }

    T red() const {return m_red;}
    T green() const {return m_green;}
    T blue() const {return m_blue;}
    T alpha() const {return m_alpha;}
    
    Color<T> darken(){
    	return Color<T>(m_red / 2, m_green / 2, m_blue / 2, m_alpha);
    }
};

typedef Color<char> IColor;
typedef Color<float> FColor;


#endif
