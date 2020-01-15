#pragma once

#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include "fltklayout.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Color_Chooser.H>

namespace fltklayout {

struct PropertiesWindow : public Fl_Double_Window {
    Widgets &widgets;

    Fl_Scroll *g;
    Fl_Pack *names;
    Fl_Pack *props;

    struct PropertiesWidget {
        static const int TYPE_STRING=1;
        static const int TYPE_INT=2;
        static const int TYPE_ENUM=3;
        static const int TYPE_BITMASK=4;
        static const int TYPE_BOOL=5;
        static const int TYPE_COLOR=6;
        int type=TYPE_STRING;

        PropertiesWindow *win;
        std::string name;
        Fl_Widget *widget=nullptr;

        std::map<int,int> enums;

        PropertiesWidget() { }

        void init(PropertiesWindow *_win,std::string &_name,const std::string &type_info) {
            win=_win;
            name=_name;

            const int w=250-20,h=20;

            Fl_Box *nbox=new Fl_Box(0,0,150,20);
            nbox->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
            nbox->copy_label(name.c_str());
            win->names->add(nbox);

            type=TYPE_STRING;
            if (type_info=="string") {
                widget=new Fl_Input(0,0,w,h);
            } else if (type_info=="int") {
                widget=new Fl_Int_Input(0,0,w,h);
                type=TYPE_INT;
            } else if (type_info.size()>4 && type_info.substr(0,5)=="enum{") {
                widget=new Fl_Choice(0,0,w,h);
                size_t sep=4;
                int idx=0,v=-1;
                for(;;) {
                    size_t next_sep=type_info.find_first_of(",}",sep+1);
                    if (next_sep==std::string::npos) break;
                    if (next_sep>sep+1) {
                        std::string val(type_info,sep+1,next_sep-(sep+1));
                        ++v;
                        const size_t eq=val.find('=');
                        if (eq!=std::string::npos)
                            v=atoi(val.c_str()+eq+1);
                        else
                            val+="="+std::to_string(v);
                        ((Fl_Choice*)widget)->add(val.c_str());
                        enums[v]=idx++;
                    }
                    sep=next_sep;
                }
                type=TYPE_ENUM;
            } else if (type_info.size()>8 && type_info.substr(0,8)=="bitmask{") {
                Fl_Pack *p=new Fl_Pack(0,0,w,h);
                p->type(Fl_Pack::HORIZONTAL);
                p->begin();
                Fl_Int_Input *i=new Fl_Int_Input(0,0,50,h);
                i->callback(win->_callback,this);
                Fl_Menu_Button *m=new Fl_Menu_Button(0,0,w-50,h);
                m->copy_label("bitmask");

                size_t sep=7;
                int idx=0,v=1;
                for(;;) {
                    size_t next_sep=type_info.find_first_of(",}",sep+1);
                    if (next_sep==std::string::npos) break;
                    if (next_sep>sep+1) {
                        std::string val(type_info,sep+1,next_sep-(sep+1));
                        const size_t eq=val.find('=');
                        if (eq!=std::string::npos)
                            v=atoi(val.c_str()+eq+1);
                        else
                            val+="="+std::to_string(v);
                        m->add(val.c_str(),0,win->_callback,this,FL_MENU_TOGGLE);
                        v>>=1;
                    }
                    sep=next_sep;
                }
                p->end();
                p->resizable(m);
                widget=p;
                type=TYPE_BITMASK;
            } else if (type_info=="color") {
                widget=new Fl_Button(0,0,w,h);
                type=TYPE_COLOR;
            } else if (type_info=="bool") {
                widget=new Fl_Choice(0,0,w,h);
                ((Fl_Choice*)widget)->add("OFF=0");
                ((Fl_Choice*)widget)->add("ON=1");
                type=TYPE_BOOL;
            } else {
                widget=new Fl_Output(0,0,w,h);
                widget->color(FL_GRAY);
            }

            win->props->add(widget);
        }

        void set(const std::string &v) {
            const int n=atoi(v.c_str());
            switch (type) {
            case TYPE_STRING: ((Fl_Input*)widget)->value(v.c_str()); break;
            case TYPE_INT: ((Fl_Int_Input*)widget)->value(v.c_str()); break;
            case TYPE_ENUM:
                if (enums.count(n)==0) {
                    std::string choice="\?\?\?="+std::to_string(n);
                    ((Fl_Choice*)widget)->add(choice.c_str());
                    enums[n]=enums.size();
                }
                ((Fl_Choice*)widget)->value(enums[n]);
                break;
            case TYPE_BITMASK: {
                Fl_Pack *p=(Fl_Pack*)widget;
                Fl_Int_Input *input=(Fl_Int_Input*)p->child(0);
                input->value(v.c_str());
                Fl_Menu_Button *m=(Fl_Menu_Button*)p->child(1);

                for (int i=0;i<m->menu()->size()-1;i++) {
                    Fl_Menu_Item *item=const_cast<Fl_Menu_Item*>(m->menu()+i);
                    std::string label=item->label();  // XXX=128
                    size_t sep=label.find('=');
                    int bit=atoi(label.c_str()+sep+1);
                    if (n&bit)
                        item->set();
                    else
                        item->clear();
                }

                break;
            }
            case TYPE_COLOR: ((Fl_Button*)widget)->color(n); break;
            case TYPE_BOOL: ((Fl_Choice*)widget)->value(!!n); break;
            default: ((Fl_Output*)widget)->value(v.c_str()); break;
            }
        }

        std::string get() {
            switch (type) {
                case TYPE_STRING: return ((Fl_Input*)widget)->value(); 
                case TYPE_INT: return ((Fl_Int_Input*)widget)->value();
                case TYPE_BOOL:
                case TYPE_ENUM: {
                    std::string v=((Fl_Choice*)widget)->text();
                    size_t sep=v.find('=');
                    return sep!=std::string::npos ? v.substr(sep+1) : v;
                }
                case TYPE_BITMASK: {
                    Fl_Pack *p=(Fl_Pack*)widget;
                    Fl_Int_Input *i=(Fl_Int_Input*)p->child(0);
                    Fl_Menu_Button *m=(Fl_Menu_Button*)p->child(1);
                    int n=0;
                    for (int i=0;i<m->menu()->size()-1;i++) {
                        const Fl_Menu_Item *item=m->menu()+i;
                        if (item->value()) {
                            std::string label=item->label();  // XXX=128
                            size_t sep=label.find('=');
                            int bit=atoi(label.c_str()+sep+1);
                            n|=bit;
                        }
                    }
                    return std::to_string(n);
                }
                case TYPE_COLOR: return std::to_string((int)((Fl_Button*)widget)->color());
                default: return ((Fl_Output*)widget)->value();
            }
        }
    };

    std::map<std::string,PropertiesWidget> properties;

    Fl_Widget *current=nullptr;

    PropertiesWindow(Widgets &widgets,int x,int y)
    : Fl_Double_Window(x,y,400,600),widgets(widgets)
    {
        copy_label("Properties Window");
        g=new Fl_Scroll(0,0,w(),h());
        names=new Fl_Pack(0,0,150,h());
        g->add(names);
        props=new Fl_Pack(150,0,w()-150-20,h());
        g->add(props);
        g->resizable(props);
        resizable(g);
        end();
    }
    virtual ~PropertiesWindow() { }

    virtual void resize(int X,int Y,int W,int H) {
        g->size(W,H);
        props->size(W-names->w()-20,props->h()); // -20 for scrollbar
        init_sizes();
        Fl_Double_Window::resize(X,Y,W,H);
    }

    void select(Fl_Widget *o) {
        names->clear();
        props->clear();
        properties.clear();
        current=o;
        if (current)  {
            FactoryInterface *f=widgets.get_factory(current);
            auto property_info=f->get_property_info();

            std::vector<std::string> keys;
            for (auto &p : property_info)
                keys.push_back(p.first);
            std::map<std::string,int> priority={
                { "name",100 }, { "factory",99 }, { "parent",98 }, 
                { "label",90 }, { "labelfont",89 }, { "labelsize",88 }, { "labelcolor",87 }, { "labeltype",86 },
                { "x",-97 }, { "y",-98 }, { "w",-99 }, { "h",-100 }
            };
            std::sort(keys.begin(),keys.end(),[&](const std::string &a,const std::string &b) {
                return priority[a]>priority[b];
            });

            for (auto &key : keys) {
                PropertiesWidget &pw=properties[key];
                pw.init(this,key,property_info[key]);
                pw.widget->callback((Fl_Callback*)_callback,&pw);
            }
        }
        update_values();
        redraw();
    }

    void update_values() {
        if (current) {
            FactoryInterface *f=widgets.get_factory(current);
            if (!f) { select(NULL); return; }

            auto property_names=f->get_property_names();
            for (auto &name : property_names)
                properties[name].set(f->get_property(&widgets,current,name));
            redraw();
        }
    }

    static void _callback(Fl_Widget *o,void *user_data) { 
         PropertiesWidget *pw=(PropertiesWidget*)user_data;
         PropertiesWindow *win=pw->win;
         win->callback(o,pw);
    }

    void callback(Fl_Widget *o,PropertiesWidget *pw) {

        std::string val;
        if (pw->type==PropertiesWidget::TYPE_COLOR) {
            unsigned char r,g,b;
            Fl::get_color(pw->widget->color(),r,g,b);
            if (!fl_color_chooser("Choose a Color",r,g,b,-1))
                return;
            Fl_Color c=fl_rgb_color(r,g,b);
            val=std::to_string((int)c);
        } else if (pw->type==PropertiesWidget::TYPE_BITMASK) {
            // if input box changed get value from that, otherwise get values from menu
            Fl_Pack *p=(Fl_Pack*)pw->widget;
            Fl_Int_Input *input=(Fl_Int_Input*)p->child(0);
            val= o==input ? input->value() : pw->get();
        } else
            val=pw->get();

        FactoryInterface *f=widgets.get_factory(current);
        if (f) { 
            auto old=f->get_property(&widgets,current,pw->name);
            if (val!=old) {
                create_undo_point();
                f->set_property(&widgets,current,pw->name,val);
                current->redraw();
            }
        }
        update_values();
    }

    virtual void create_undo_point()=0;
};

} // namespace fltklayout


