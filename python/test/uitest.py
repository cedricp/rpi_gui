import sys, os
sys.path.append("/sources/GUI/BUILD_X86/python")
import ui

class Test(ui.Button):
    def __init__(self, X, Y, W, H, name, parent):
        ui.Button.__init__(self,X, Y, W, H, name, parent)
        self.tiles_enabled(1)
        self.bg_color(ui.FColor(1,1,1,1))
        self.callback(self.cb, self)
        
    def cb(self, data):
        print "Callback OK !!! "
        
    def enter_event(self):
        print "entering"

    def leave_event(self):
        print "leaving"

def echo(data):
    print "echo"

comp = ui.get_compositor()
window = comp.create_new_window()
panel = ui.Multi_panel(0,0,window.w(), window.h(), "MP", window)
w = Test(0,0, 200, 200, "Hello", panel)
x = Test(0,0, 200, 200, "Hello2", panel)
but1=panel.add_tab(w)
but2=panel.add_tab(x)
comp.run()

