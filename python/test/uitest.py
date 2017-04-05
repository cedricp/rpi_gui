import sys, os
sys.path.append("/sources/GUI/BUILD_X86/python")
import ui

class Test(ui.Button):
    def __init__(self, X, Y, W, H, name, parent):
        ui.Button.__init__(self,X, Y, W, H, name, parent)
        #self.tiles_enabled(1)
        #self.add_timer(500)
        #self.bg_color(ui.FColor(1,1,1,1))
        self.callback(self.cb, self)
        
    def timer_event(self, data):
        print "Timer!!"
        
    def cb(self, data):
        print "Callback OK !!! "
        
    def enter_event(self):
        ui.Button.enter_event(self)
        print "entering"

    def leave_event(self):
        ui.Button.leave_event(self)
        print "leaving"
        
    def draw(self):
        ui.Button.draw(self)
        self.painter().disable_texture()
        self.painter().color(ui.FColor(1,1,1,.5))
        self.painter().draw_quad(20, 20, 20, 20, True, True)
        
def echo(data):
    print "echo"

comp = ui.get_compositor()

txt = comp.painter().build_text(0, "hello", 0,0)
comp.painter().draw_text(txt)

window = comp.create_new_window()
panel = ui.Multi_panel(0,0,window.w(), window.h(), "MP", window)
w = Test(0,0, 200, 200, "Hello", panel)
x = Test(0,0, 200, 200, "Hello2", panel)
but1=panel.add_tab(w)
but2=panel.add_tab(x)

comp.run()

