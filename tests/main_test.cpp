#include "../include/navitwidget.h"
#include "../include/navitwidget.h"
#include "compositor.h"
#include "button.h"
#include "layout.h"
#include "label.h"
#include "scroll.h"
#include "slider.h"
#include "multi_panel.h"
#include "file_chooser.h"

int main(int argn, char** args)
{
    Widget* parent = COMPOSITOR->create_new_window();
    parent->backgroung_gradient_enable(true);

    Multi_panel* tab = new Multi_panel(0, 0, parent->w(), parent->h(), "multipane", parent);
    Scroll* scroll = new Scroll(0, 0, 300, 500, "Radio tab", parent);

    Layout* layout = new Layout(0, 0, 350, 500, "Child", scroll, LAYOUT_VERTICAL);
    Slider* sld = new Slider(0, 0, 60, 60, "Slider", layout);
    Label* lbl = new Label(0, 0, 60, 60, "Label test", layout);
    Button* button = new Button(0, 0, 60, 40, "Button #1", layout);
    Button* button2 = new Button(0, 0, 60, 40, "Button #2", layout);
    Button* button3 = new Button(0, 0, 60, 40, "Button #3", layout);

    Navit_widget* navit = new Navit_widget(10,10,500, 400, "navit", parent);

    Label* lbl2 = new Label(0, 0, 100, 80, "MP3 & Music content", parent);
    lbl2->bg_color(FColor(0,0,.6,1.));

    button->image("svg/search.svg");
    button2->image("svg/search.svg");
    button3->image("svg/search.svg");
    layout->bg_color(FColor(.1, .2, .3, 1.0));
    scroll->bg_color(FColor(.8, .8, .8, 1.0));

    tab->add_tab(scroll);
    tab->add_tab(lbl2);
    tab->add_tab(navit);

    File_chooser* fc = new File_chooser(0, 0, 300, 400, "File", parent);
    tab->add_tab(fc);

    return COMPOSITOR->run();
}
