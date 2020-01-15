#include <FL/Fl_Double_Window.H>
#include <fltklayout.h>

#include <iostream>

struct MyWindow : public Fl_Double_Window {
    fltklayout::Widgets widgets;
    fltklayout::Factories factories;
    
    fltklayout::LayoutWidget *layout;

    MyWindow() : Fl_Double_Window(10,10,500,400)
    {
        factories.load_layouts_as_widgets("t.layout");
        begin();
        layout=(fltklayout::LayoutWidget*)fltklayout::create_widget(widgets,factories,"Layout1","w0",0,0,w(),h(),"");
        end();
        resizable(layout);

        std::cout << "widgets in this window..." << std::endl;
        for (auto &name : widgets.get_names())
            std::cout << "    " << name << std::endl;

        std::cout << "widgets in the layout instance..." << std::endl;
        for (auto &name : layout->widgets.get_names())
            std::cout << "    " << name << std::endl;
    }
    virtual ~MyWindow() { }
};

int main(int argc,char **argv) {
    MyWindow win;
    win.show();
    Fl::run();
    return 0;
}

