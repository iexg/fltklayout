#include <FL/Fl_Double_Window.H>
#include <fltklayout.h>

struct MyWindow : public Fl_Double_Window {
    fltklayout::Widgets widgets;      // collection of managed widgets in this window (indexed by name and Fl_Widget*)
    fltklayout::Factories factories;  // collection of 'factories' for widget descriptions (indexed by factory name)
    fltklayout::LayoutWidget *layout; // a custom widget loaded by this example

    MyWindow() : Fl_Double_Window(10,10,500,400) {
        factories.load_layouts_as_widgets("t.layout");
        begin();
        // Create widget 'w0' from factory 'Layout1' described in layout file t.layout
        layout=(fltklayout::LayoutWidget*)fltklayout::create_widget(widgets,factories,"Layout1","w0",0,0,w(),h(),"");
        end();
        resizable(layout);
    }
    virtual ~MyWindow() { }
};

int main(int argc,char **argv) {
    MyWindow win;
    win.show();
    Fl::run();
    return 0;
}

