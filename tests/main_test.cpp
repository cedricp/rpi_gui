#include "compositor.h"
#include "button.h"
#include "layout.h"
#include "label.h"

int main(int argn, char** args)
{
    Widget* parent = COMPOSITOR->create_new_window();
    Layout* layout = new Layout(70, 150, 200, 200, "Child", parent);
    layout->set_style(LAYOUT_VERTICAL);

    Button* button = new Button(0, 0, 60, 60, "Button #1", layout);
    Button* button2 = new Button(0, 0, 60, 60, "Button #2", layout);
    Button* button3 = new Button(0, 0, 60, 60, "Button #3", layout);
    Label* lbl = new Label(0, 0, 60, 60, "Label test... >>>", layout);
//    button->transparent(true);
//    button2->transparent(true);
//    button3->transparent(true);
    layout->add_widget(button);
    layout->add_widget(button2);
    layout->add_widget(button3);
    layout->add_widget(lbl);

    button->set_image("/servers/Home/cedric/Desktop/Hawcons/Hawcons/SVG/Documents/Blue/Filled/icon-119-lock-rounded-open.svg");
    button2->set_image("/servers/Home/cedric/Desktop/Hawcons/Hawcons/SVG/Documents/Blue/Filled/icon-111-search.svg");
    button3->set_image("/servers/Home/cedric/Desktop/Hawcons/Hawcons/SVG/Documents/Blue/Filled/icon-121-combination-lock.svg");
    layout->set_bg_color(FColor(.1, .2, .3, 1.0));
    //ayout->set_fg_color(FColor(.5, .9, .2, 1.0));
    return COMPOSITOR->run();
}
