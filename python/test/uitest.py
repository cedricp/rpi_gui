import sys, os
sys.path.append("/sources/GUI/BUILD_X86/python")
import ui


class Test(ui.Button):
    def __init__(self, X, Y, W, H, name, parent):
        ui.Button.__init__(self,X, Y, W, H, name, parent)
        self.tiles_enabled(1)
        self.bg_color(ui.FColor(1,1,1,1))
        
    def enter_event(self):
        print "entering"

    def leave_event(self):
        print "leaving"


comp = ui.get_compositor()
window = comp.create_new_window()
w = Test(0,0, 200, 200, "Hello", window)
window.tiles_enabled(1)
comp.run()

