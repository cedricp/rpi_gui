import sys, os
sys.path.append("/sources/GUI/BUILD_X86/python")
import ui

class Test(ui.Widget):
     def __init__(self, X=0, Y=0, W=0, H=0, name='', parent = None):
        ui.Widget.__init__(self,X, Y, W, H, name, parent)
        vtx = [10., 0., 6., 70., 4., 70., 0., 0.]
        data = ui.vertex_container(vtx)
        self.v = self.painter().create_polygon_2d(data)
        self.add_timer(5)
        self.r = 10.
        
        self.wid = ui.Button(0,0,200,200, "OK?")
        self.wid.callback(self.cb, self.wid)
        self.wid.hide()
        
     def mouse_release_event(self, but):
        self.wid.resize(200,200)
        self.wid.modal(300,140)
     
     def cb(self, data):
        print "CB"
        self.wid.hide()
     
     def timer_event(self, data):
        if self.hidden():
            return False
        self.r += 1.
        self.dirty(True)
        return True
        
     def draw(self):
        area = self.drawing_area()
        self.push_model_matrix()
        self.translate(area.width()/2., area.height()/2., 0.)
        self.rotate(0,0,1,self.r)
        self.translate(-10, -20, 0.)
        self.scale(2,2,2)
        self.painter().disable_texture()
        self.painter().draw_solid_polygon_2d(self.v)
        self.pop_model_matrix()

class Radio(ui.Widget):
    def __init__(self, X, Y, W, H, name, parent):
        ui.Widget.__init__(self,X, Y, W, H, name, parent)
        # Look for the font resource file
        fnt_resource = self.painter().locate_resource("fonts/custom.ttf")
        # Enable background tiling
        self.tiles_enabled(True)
        
        # Define layouts
        self.layout_v = ui.Layout(0, 0,self.w(), self.h(),"global_layout", self, ui.LAYOUT_VERTICAL)
        self.layout_tune = ui.Layout(0, 0, self.w(), 85,"radio_layout", self.layout_v, ui.LAYOUT_HORIZONTAL)
        self.layout_tune.fixed_height(85)
        
        self.tune_display = ui.Label()
        self.bigfonts = self.tune_display.load_fonts(fnt_resource, 80)
        self.tune_display.fixed_width(300)
        self.tune_display.fg_color(ui.FColor(.2,.2,.8,1.))
        self.tune_display.label("107.70")
        
        self.button_scan_low = ui.Button(0, 0, 80, 0)
        self.button_scan_low.use_fonts_id(self.bigfonts)
        self.button_scan_low.label("<<")
        
        self.button_scan_hi = ui.Button(0, 0, 80, 0)
        self.button_scan_hi.use_fonts_id(self.bigfonts)
        self.button_scan_hi.label(">>")
        
        self.rds_display = ui.Label()
        self.rds_display.fg_color(ui.FColor(.4,.2,.4,1.))
        self.rds_display.use_fonts_id(self.bigfonts)
        self.rds_display.fixed_height(80)
        self.rds_display.backgroung_gradient_enable(True)
        self.rds_display.gradient(ui.FColor(.0,.0,.0,1), ui.FColor(.7,.7,.7,1))
        self.rds_display.tiles_enabled(True)
        self.rds_display.label("RADIO #1")
        
        self.lay = ui.Layout(0, 0, self.w(), 85,"??", None, ui.LAYOUT_HORIZONTAL)
        self.file_man = ui.File_chooser()
        self.file_man2 = Test()
        
        self.button_scan_low.parent(self.layout_tune)      
        self.tune_display.parent(self.layout_tune)
        self.button_scan_hi.parent(self.layout_tune)  
        
        self.rds_display.parent(self.layout_v)
        self.file_man.parent(self.lay)
        self.file_man2.parent(self.lay)
        self.lay.parent(self.layout_v)
        
        
comp = ui.get_compositor()

window = comp.create_new_window()
panel = ui.Multi_panel(0,0,window.w(), window.h(), "MP", window)
panel.buttons_ratio(8)

radio_tab = Radio(0,0, 200, 200, "RADIO", window)
x = ui.Button(0,0, 200, 200, "MUSIC", panel)
navit = ui.Navit_widget(0,0,0,0,"NAVIT")

but1=panel.add_tab(radio_tab)
but2=panel.add_tab(x)
but3=panel.add_tab(navit)


comp.run()

