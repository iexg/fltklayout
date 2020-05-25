#include <FL/Fl_Double_Window.H>
#include <fltklayout.h>

#include <table.h>

#include <iostream>

// subclass of Fl_Table specific to this example
struct MyGrid : public fltklayout::StringTable {
    MyGrid(int X,int Y,int W,int H,const char *L=0) : fltklayout::StringTable(X,Y,W,H) { }
};

struct MyWindow : public Fl_Double_Window {
    fltklayout::Widgets widgets;
    fltklayout::Factories factories;
    fltklayout::LayoutWidget *layout;

    // a custom factory that known how to create MyGrid objects
    struct MyGridWidgetFactory : public fltklayout::WidgetFactoryBase {
        MyGridWidgetFactory(fltklayout::Factories *factories,const std::string &factory_name) : fltklayout::WidgetFactoryBase(factories,factory_name) { }
        virtual ~MyGridWidgetFactory() { }

        virtual bool border_visible() override { return true; }
        virtual bool is_group() override { return false; }

        virtual Fl_Widget *create(fltklayout::Widgets *widgets,const std::string &name,int x,int y,int w,int h,const std::string &label) override {
            Fl_Widget *o=new MyGrid(x,y,w,h,""); // <--- this is where MyGrid really gets created
            o->copy_label(label.c_str());
            fltklayout::WidgetInfo *winfo=(new fltklayout::WidgetInfo())->init(widgets,name,this,o);
            widgets->add_widget(winfo);
            o->callback(fltklayout::callback_helper,widgets);
            return o;
        }
    };

    MyWindow() : Fl_Double_Window(10,10,500,400)
    {
        factories.load_layouts_as_widgets("t.layout"); // load the descriptions from file
        factories.add_factory(new MyGridWidgetFactory(&factories,"name=myGrid")); // intercept creation of 'myGrid' objects and use our custom factory instead
        begin();
        layout=(fltklayout::LayoutWidget*)fltklayout::create_widget(widgets,factories,"Layout1","w0",0,0,w(),h(),"");
        end();
        resizable(layout);

        std::cout << "widgets in this window..." << std::endl;
        for (auto &name : widgets.get_names())
            std::cout << "    " << name << std::endl;

        std::cout << "widgets in the layout instance object..." << std::endl;
        for (auto &name : layout->widgets.get_names())
            std::cout << "    " << name << std::endl;

        // how to find widgets inside layouts we just loaded...
        fltklayout::StringTable *dont_press=(fltklayout::StringTable*)layout->widgets.get_widget("myGrid:0");

        // how to get find extra WidgetInfo data and use it to add a callback...
        fltklayout::WidgetInfo *i=layout->widgets.get_info(dont_press);
        i->callback=[this](Fl_Widget *w,fltklayout::WidgetInfo *i){ 
            auto &t=*(fltklayout::StringTable*)w;
            const int R=t.callback_row(),C=t.callback_col(); // visible row,col on screen (not actual row,col in backing store)
            const int DR=t.row_headers.view2idx(R),DC=t.column_headers.view2idx(C); // row,col in backing store

            switch (t.callback_context()) {
                case fltklayout::StringTable::CONTEXT_ROW_HEADER: printf("CONTEXT_ROW_HEADER R=%d name=%s\n",R,t.row_headers.headers[DR]->name.c_str()); break;
                case fltklayout::StringTable::CONTEXT_COL_HEADER: printf("CONTEXT_COL_HEADER C=%d name=%s\n",C,t.column_headers.headers[DC]->name.c_str()); break;
                case fltklayout::StringTable::CONTEXT_CELL: printf("CONTEXT_CELL R=%d C=%d => DR=%d DC=%d\n",R,C,DR,DC); break;
                case fltklayout::StringTable::CONTEXT_TABLE: printf("CONTEXT_TABLE\n"); break;
                default: printf("other\n"); break;
            }
        };
    }
    virtual ~MyWindow() { }
};

int main(int argc,char **argv) {
    MyWindow win;
    win.show();
    Fl::run();
    return 0;
}

