#include <FL/Fl_Double_Window.H>
#include <fltklayout.h>

#include <iostream>

Fl_Widget *clone(fltklayout::Widgets &widgets,fltklayout::Factories &factories,Fl_Widget *o,const std::string &new_widget_name) {

    fltklayout::WidgetInfo *i=widgets.get_info(o);
    if (!i) return NULL; // cant find metadata

    // metadata provides factory we can use to create a new instance
    Fl_Widget *n=i->factory->create(&widgets,new_widget_name,0,0,1,1,"");

    // then we duplicate all the properties
    fltklayout::PropertyMap props=widgets.get_properties(o);
    for (const auto &p : props) {
        if (p.first!="name" && p.first!="parent") // prevent duplicating name or moving to group
            i->factory->set_property(&widgets,n,p.first,p.second);
    }

    return n;
}

struct MyWindow : public Fl_Double_Window {
    fltklayout::Widgets widgets;      // collection of managed widgets in this window (indexed by name and Fl_Widget*)
    fltklayout::Factories factories;  // collection of 'factories' for widget descriptions (indexed by factory name)

    Fl_Widget *b1,*b2;

    MyWindow() : Fl_Double_Window(10,10,500,400) {

        begin();

        // find factory that can construct Fl_Button objects
        fltklayout::FactoryInterface *factory=factories.get_factory("Fl_Button");

        // create a Fl_Button called "button1"
        b1=factory->create(&widgets,"button1",50,50,100,50,"");
        factory->set_property(&widgets,b1,"label","Press Me!"); // same as b1->copy_label("Press Me!");
        factory->set_property(&widgets,b1,"labelcolor","1"); // same as b1->labelcolor(1);
        
        // set callback
        fltklayout::WidgetInfo *i1=widgets.get_info(b1);
        i1->callback=[this](Fl_Widget *w,fltklayout::WidgetInfo *i){
            std::cout << "Button1 pressed!" << std::endl;
        };

        // clone first button
        b2=clone(widgets,factories,b1,"button2");
        b2->resize(50,100,100,50);

        // set callback (callback was not cloned)
        fltklayout::WidgetInfo *i2=widgets.get_info(b2);
        i2->callback=[this](Fl_Widget *w,fltklayout::WidgetInfo *i){
            std::cout << "Button2 pressed!" << std::endl;
        };

        end();
    }
    virtual ~MyWindow() { }
};

int main(int argc,char **argv) {
    MyWindow win;
    win.show();
    Fl::run();
    return 0;
}

