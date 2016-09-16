#include "compositor.h"
#include "button.h"
#include "layout.h"
#include "label.h"
#include "scroll.h"
#include "slider.h"

int main(int argn, char** args)
{
    Widget* parent = COMPOSITOR->create_new_window();

    Scroll* scroll = new Scroll(100, 100, 200, 100, "SCROLL", parent);

    Layout* layout = new Layout(0, 0, 200, 200, "Child", scroll, LAYOUT_VERTICAL);
    Slider* sld = new Slider(0, 0, 60, 60, "Label test >>>", layout);
    //sld->fixed_height(20);
    Label* lbl = new Label(0, 0, 60, 60, "Label test >>>", layout);
    Button* button = new Button(0, 0, 60, 60, "Button #1", layout);
    Button* button2 = new Button(0, 0, 60, 60, "Button #2", layout);
    Button* button3 = new Button(0, 0, 60, 60, "Button #3", layout);

    button->set_image("/servers/Home/cedric/Desktop/Hawcons/Hawcons/SVG/Documents/Blue/Filled/icon-119-lock-rounded-open.svg");
    button2->set_image("/servers/Home/cedric/Desktop/Hawcons/Hawcons/SVG/Documents/Blue/Filled/icon-111-search.svg");
    button3->set_image("/servers/Home/cedric/Desktop/Hawcons/Hawcons/SVG/Documents/Blue/Filled/icon-121-combination-lock.svg");
    layout->bg_color(FColor(.1, .2, .3, 1.0));
    scroll->bg_color(FColor(.8, .8, .8, 1.0));

    return COMPOSITOR->run();
}
