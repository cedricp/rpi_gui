#include "compositor.h"
#include "button.h"
#include "layout.h"
#include "label.h"
#include "scroll.h"
#include "slider.h"
#include "tab_widget.h"

int main(int argn, char** args)
{
    Widget* parent = COMPOSITOR->create_new_window();

    Tab_widget* tab = new Tab_widget(100, 100, 300, 300, "tab", parent);
    Scroll* scroll = new Scroll(0, 0, 300, 500, "SCROLL", parent);

    Layout* layout = new Layout(0, 0, 350, 500, "Child", scroll, LAYOUT_VERTICAL);
    Slider* sld = new Slider(0, 0, 60, 60, "Slider", layout);
    Label* lbl = new Label(0, 0, 60, 60, "Label test", layout);
    Button* button = new Button(0, 0, 60, 40, "Button #1", layout);
    Button* button2 = new Button(0, 0, 60, 40, "Button #2", layout);
    Button* button3 = new Button(0, 0, 60, 40, "Button #3", layout);

    Label* lbl2 = new Label(0, 0, 100, 80, "Test::2", parent);
    lbl2->bg_color(FColor(0,0,.6,1.));

    button->image("icon-111-search.svg");
    button2->image("icon-111-search.svg");
    button3->image("icon-111-search.svg");
    layout->bg_color(FColor(.1, .2, .3, 1.0));
    scroll->bg_color(FColor(.8, .8, .8, 1.0));

    tab->add_tab(scroll);
    tab->add_tab(lbl2);

    return COMPOSITOR->run();
}
