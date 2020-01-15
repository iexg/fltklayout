#pragma once

#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include "fltklayout.h"
#include "properties.h"

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/names.h>

namespace fltklayout {

class Popup {
    std::vector<std::string> items;
public:
    Popup() { }

    void clear() { items.clear(); }

    std::string text(const int n) { return n>=0 && n<(int)items.size() ? items[n].substr(1) : ""; }

    void push_back(const std::string &item,const bool is_active=true) {
        char a=is_active ? '1' : '0';
        items.push_back(std::string(&a,1)+item);
    }

    std::string show(const int x,const int y) {
        Fl::first_window()->begin();
        Fl_Menu_Button m(1,1,1,1);
        Fl::first_window()->end();

        for (int n=0;n<(int)items.size();n++) {
            const std::string i=text(n);
            if (i!="---") {
                int flags= items[n][0]=='0' ? FL_MENU_INACTIVE : 0;
                if (text(n+1)=="---") flags|=FL_MENU_DIVIDER;
                m.add(i.c_str(),0,0,0,flags);
            }
        }
        const Fl_Menu_Item *menu_choice=m.menu()->popup(x,y,0,0,0);
        if (!menu_choice) return "";

        char path[1024];
        if (m.item_pathname(path,sizeof(path),menu_choice)!=0) return "";

        return path[0]=='/' ? path+1 : path;
    }
};

class ToolTip : public Fl_Menu_Window {
    std::string tip;

public:
    ToolTip() : Fl_Menu_Window(10,10) { set_override(); end(); }

    void draw() {
        draw_box(FL_BORDER_BOX,0,0,w(),h(),Fl_Color(165));
        fl_color(FL_BLACK);
        fl_font(labelfont(),labelsize());
        fl_draw(tip.c_str(),4,4,w()-8,h()-8,Fl_Align(FL_ALIGN_LEFT|FL_ALIGN_WRAP));
    }
    void text(const std::string &new_tip) {
        tip=new_tip;
        int W=w(),H=h();
        fl_measure(tip.c_str(),W,H,0);
        W+=6;
        size(W,H);
        redraw();
    }
};

class DesignWindow : public Fl_Overlay_Window {
public:
    Widgets widgets;
    Factories factories;

    Fl_Menu_Bar *menu;
    int menu_height;
    static void _menu_callback(Fl_Widget *w,void *user_data) { 
        DesignWindow &win=*(DesignWindow*)user_data;
        char path[256];
        if (win.menu->item_pathname(path,sizeof(path)-1)==0)
            win.menu_callback(path);
    }

    struct TabScroll : public Fl_Scroll {
        TabScroll(int x,int y,int w,int h) : Fl_Scroll(x,y,w,h) { }

        virtual bool xy_inside(Fl_Overlay_Window *win,int mx,int my) {
            int x,y,w,h;
            bbox(x,y,w,h);
            return mx>=x && mx<x+w && my>=y && my<y+h;
        }
    };

    struct Tab : public TabScroll {
        DesignWindow &dw;

        struct PopoutTab : public Fl_Overlay_Window {
            TabScroll *scroll;
            Tab &tab;

            static void window_callback(Fl_Widget *widget, void *o) { ((PopoutTab*)o)->window_closed(); }
            void window_closed() { 
                while (scroll->children()) {
                    Fl_Widget *o=scroll->child(0);
                    int ox=o->x()-scroll->x(),oy=o->y()-scroll->y();
                    o->parent()->remove(o);
                    o->position(tab.x()+ox,tab.y()+oy);
                    tab.add(o);
                }
                scroll->clear();
                hide();
                tab.popout=NULL;
                tab.redraw();
                Fl::delete_widget(this);
            }

            PopoutTab(Tab &tab,int x,int y,int w,int h) : Fl_Overlay_Window(x,y,w,h),tab(tab) {
                copy_label(tab.label());
                begin();
                scroll=new TabScroll(0,0,w,h);
                scroll->labelfont(FL_HELVETICA_BOLD);
                scroll->box(FL_DOWN_BOX);
                scroll->color(FL_BACKGROUND_COLOR);
                scroll->copy_label(tab.label());

                resizable(scroll);
                scroll->begin();
                while (tab.children()) {
                    Fl_Widget *o=tab.child(0);
                    int ox=o->x()-tab.x(),oy=o->y()-tab.y();
                    o->parent()->remove(o);
                    o->position(ox,oy);
                    scroll->add(o);
                }
                scroll->end();
                end();
                tab.clear();
                tab.redraw();
                callback(window_callback,this);
            }
            ~PopoutTab() { delete scroll; }

            virtual int handle(int e) { 
                if (e!=FL_NO_EVENT && e!=FL_ENTER && e!=FL_LEAVE && tab.dw.handle(this,&tab,e))
                    return 1;
                return Fl_Overlay_Window::handle(e);
            }
            virtual void draw_overlay() { tab.dw.render_overlay(&tab); }

            void rename(const std::string &name) {
                copy_label(name.c_str());
                scroll->copy_label(name.c_str());
            }
        };
        PopoutTab *popout=nullptr;
       
        Tab(DesignWindow &dw,int x,int y,int w,int h) : TabScroll(x,y,w,h),dw(dw) { }
        ~Tab() { delete popout; }

        void rename(const std::string &name) {
            copy_label(name.c_str());
            if (popout) popout->rename(name);
        }

        virtual bool xy_inside(Fl_Overlay_Window *win,int mx,int my) {
            return win==popout ? popout->scroll->xy_inside(win,mx,my) : TabScroll::xy_inside(win,mx,my);
        }
        virtual Fl_Group *as_group() { return popout ? popout->scroll->as_group() : Fl_Scroll::as_group(); }

        std::vector<Fl_Widget*> get_child_widgets() {
            std::vector<Fl_Widget*> tab_widgets;
            dw.widgets.get_child_widgets(as_group(),tab_widgets);
            return tab_widgets;
        }

        void clear() { 
            if (popout)
                popout->scroll->clear();
            else
                TabScroll::clear();
        }
    };

    Fl_Tabs *tabs;
    int tab_label_height;
    static void _tabs_callback(Fl_Widget *w,void *user_data) { 
        DesignWindow &win=*(DesignWindow*)user_data;
        win.tabs_callback();
    }

    struct MyPropertiesWindow : public PropertiesWindow {
        DesignWindow &dwin;
        MyPropertiesWindow(DesignWindow &dwin,int x,int y) : PropertiesWindow(dwin.widgets,x,y),dwin(dwin) { }
        virtual ~MyPropertiesWindow() { }
        virtual void create_undo_point() { dwin.create_undo_point(); }
    };
    PropertiesWindow *properties_window;

    struct Rect { 
        int x1,y1,x2,y2; 
        Rect(int x1,int y1,int x2,int y2) : x1(x1),y1(y1),x2(x2),y2(y2) { }
        int x() { return std::min(x1,x2); }
        int y() { return std::min(y1,y2); }
        int w() { return std::abs(x2-x1); }
        int h() { return std::abs(y2-y1); }
    };
    Rect srect={0,0,0,0};
    int snap=10;
    int sx=0,sy=0;
    void init_sizes(Fl_Widget *o) { // recusrively call Fl_Group::init_sizes() on groups in tree
        if (!o || !o->as_group()) return;
        Fl_Group *g=o->as_group();
        g->init_sizes();
        for (int i=0;i<g->children();i++)
            init_sizes(g->child(i));
    }

    enum action_t {
        ACTION_NONE,
        ACTION_SIZE,
        ACTION_SELECT,
        ACTION_DRAG,
        ACTION_POPUP_ADD_WIDGET,
        ACTION_POPUP_TOGGLE_RESIZABLE,
        ACTION_POPUP_WIDGET_MOVE,
        ACTION_POPUP_WIDGET_EXPAND,
        ACTION_POPUP_WIDGET_REDUCE,
        ACTION_POPUP_WIDGET_DELETE,
        ACTION_POPUP_SELECTION_DELETE,
        ACTION_LAYOUT_ADD,
        ACTION_LAYOUT_RENAME,
        ACTION_LAYOUT_DELETE,
        ACTION_CUT,
        ACTION_PASTE,
        ACTION_FILE_NEW,
        ACTION_FILE_LOAD_LAYOUTS,
        ACTION_FILE_LOAD_LAYOUT_WIDGETS,
    };
    action_t action=ACTION_NONE;
    Tab *action_tab=nullptr;
    void action_start(const action_t a,Tab *tab=nullptr) {
        action_end();
        action=a;
        action_tab=tab;
        if (a!=ACTION_SELECT) create_undo_point();
    }
    void action_end() {
        if (action!=ACTION_NONE) {
            if (primary) init_sizes(primary->parent());
            update_layout_widgets(action_tab);
            action=ACTION_NONE;
            action_tab=nullptr;
        }
    }

    Fl_Widget *primary=nullptr;
    std::set<Fl_Widget*> selected;
    void select_primary(Fl_Widget *o) {
        if (o) selected.insert(o);
        primary=o;
        need_redraw_overlay=true;
        properties_window->select(primary);
    }
    void clear_selection(const std::string &why=std::string()) {
        selected.clear();
        select_primary(NULL);
        need_redraw_overlay=true;
    }

    int snapx(int x) { return x-x%snap; }
    int snapy(int y) { return y-y%snap; }

    bool xy_inside_widget(int x,int y,Fl_Widget *o,const int extra_pix=10) { 
        return o && x>=o->x()-extra_pix && x<o->x()+o->w()+extra_pix && y>=o->y()-extra_pix && y<o->y()+o->h()+extra_pix; 
    }
    bool widget_inside_rect(Fl_Widget *o,int x,int y,int w,int h) { return o && o->x()>=x && o->x()+o->w()<=x+w && o->y()>=y && o->y()+o->h()<=y+h; }

    void get_widgets_at(Fl_Widget *o,int x,int y,std::vector<Fl_Widget*> &out) {
        Fl_Group *g=o ? o->as_group() : NULL;
        for (int i=0;g && i<g->children();i++) {
            Fl_Widget *o=g->child(i);
            if (!widgets.is_managed(o)) continue;
            if (xy_inside_widget(x,y,o)) out.push_back(o);
            get_widgets_at(o,x,y,out);
        }
    }

    void remove_children(std::set<Fl_Widget*> &w) {
        for (auto it=w.begin();it!=w.end();) {
            Fl_Widget *o=*it;
            bool parent_in_set=false;
            for (Fl_Widget *p=o->parent();p && !parent_in_set;p=p->parent())
                parent_in_set=w.count(p);
            if (parent_in_set)
                w.erase(it++);
            else
                ++it;
        }
    }

    void tab_body_right_click_menu(int x,int y) {

        Tab *tab=tabs_current();

        Fl_Widget *parent= primary ? primary->parent() : NULL;
        Fl_Widget *managed_parent=widgets.get_parent(primary);
        
        Popup p;
        for (auto &name : factories.get_factory_names()) 
            p.push_back("Add/"+name,!has_dependency(name,tab->label()));
        p.push_back("Widget/Select Parent",managed_parent!=NULL);
        p.push_back("Widget/Select Syblings",primary!=NULL);
        p.push_back("Widget/Toggle Resizable",primary!=NULL);
        p.push_back("Widget/Move/Left",primary!=NULL);
        p.push_back("Widget/Move/Right",primary!=NULL);
        p.push_back("Widget/Move/Up",primary!=NULL);
        p.push_back("Widget/Move/Down",primary!=NULL);
        p.push_back("Widget/Expand/Left",primary!=NULL);
        p.push_back("Widget/Expand/Right",primary!=NULL);
        p.push_back("Widget/Expand/Up",primary!=NULL);
        p.push_back("Widget/Expand/Down",primary!=NULL);
        p.push_back("Widget/Reduce",primary && widgets.get_factory(primary)->is_group() && primary->as_group()->children());
        p.push_back("Widget/Delete/Delete",primary!=NULL);
        p.push_back("Selection/Clear",!selected.empty());
        p.push_back("Selection/Delete/Delete",!selected.empty());
        p.push_back("Layout/Add New");
        p.push_back("Layout/Rename");
        p.push_back("Layout/Popout",tab->popout==nullptr);
        p.push_back("Layout/Popin",tab->popout!=nullptr);
        p.push_back("Layout/Delete/Delete");

        std::string path=p.show(x,y);
        if (path.size()>4 && path.substr(0,4)=="Add/") {
            std::string name=widgets.get_unique_name();
            std::string factory=path.substr(4);
            Fl_Widget *o=create_widget(widgets,factories,factory,name,snapx(x),snapy(y),50,50,name);
            if (o) {
                action_start(ACTION_POPUP_ADD_WIDGET,tab);
                Fl_Group *g=tab->as_group();
                if (primary && widgets.get_factory(primary)->is_group())
                    g=primary->as_group();
                g->add(o);
                g->end();
                action_end();
                need_redraw_overlay=true;
            }
        } else if (path=="Widget/Select Parent") {
            clear_selection();
            select_primary(managed_parent);
        } else if (path=="Widget/Select Syblings") {
            clear_selection();
            Fl_Group *p=managed_parent ? managed_parent->as_group() : parent->as_group();
            for (int i=0;i<p->children();i++) {
                Fl_Widget *o=p->child(i);
                if (widgets.is_managed(o)) 
                    selected.insert(o);
            }
            need_redraw_overlay=true;
        } else if (path=="Widget/Toggle Resizable") {
            action_start(ACTION_POPUP_TOGGLE_RESIZABLE,tab);
            Fl_Group *p=parent->as_group();
            p->resizable(p->resizable()==primary ? NULL : primary);
            action_end();
            need_redraw_overlay=true;
        } else if (path.compare(0,12,"Widget/Move/")==0) {
            action_start(ACTION_POPUP_WIDGET_MOVE,tab);
            Fl_Widget *p=managed_parent ? managed_parent : parent;
            FactoryInterface *f=widgets.get_factory(primary);
            const std::string dir=path.substr(12);
            if (dir=="Left")
                f->resize(primary,p->x(),primary->y(),primary->w(),primary->h());
            else if (dir=="Right")
                f->resize(primary,p->x()+p->w()-primary->w(),primary->y(),primary->w(),primary->h());
            else if (dir=="Up")
                f->resize(primary,primary->x(),p->y(),primary->w(),primary->h());
            else if (dir=="Down")
                f->resize(primary,primary->x(),p->y()+p->h()-primary->h(),primary->w(),primary->h());
            action_end();
            need_redraw_overlay=true;
        } else if (path.compare(0,14,"Widget/Expand/")==0) {
            action_start(ACTION_POPUP_WIDGET_EXPAND,tab);
            Fl_Widget *p=managed_parent ? managed_parent : parent;
            FactoryInterface *f=widgets.get_factory(primary);
            const std::string dir=path.substr(14);
            if (dir=="Left")
                f->resize(primary,p->x(),primary->y(),primary->w()+primary->x()-p->x(),primary->h());
            else if (dir=="Right")
                f->resize(primary,primary->x(),primary->y(),p->x()+p->w()-primary->x(),primary->h());
            else if (dir=="Up")
                f->resize(primary,primary->x(),p->y(),primary->w(),primary->y()+primary->h()-p->y());
            else if (dir=="Down")
                f->resize(primary,primary->x(),primary->y(),primary->w(),p->y()+p->h()-primary->y());
            action_end();
            need_redraw_overlay=true;
        } else if (path=="Widget/Reduce") {
            Fl_Group *g=primary->as_group();
            FactoryInterface *f=widgets.get_factory(primary);
            struct XYWH { int x,y,w,h; };
            std::vector<XYWH> dimensions;
            for (int i=0;i<g->children();i++) {
                Fl_Widget *o=g->child(i);
                dimensions.push_back({o->x(),o->y(),o->w(),o->h()});
            }
            const int imax=std::numeric_limits<int>::max();
            int minx=imax,miny=imax,maxx=0,maxy=0;
            for (int i=0;i<g->children();i++) {
                Fl_Widget *o=g->child(i);
                if (o->x()<minx) minx=o->x();
                if (o->y()<miny) miny=o->y();
                if (o->x()+o->w()>maxx) maxx=o->x()+o->w();
                if (o->y()+o->h()>maxy) maxy=o->y()+o->h();
            }
            if (g->x()!=minx || g->y()!=miny || g->x()+g->w()!=maxx || g->y()+g->w()!=maxy) {
                action_start(ACTION_POPUP_WIDGET_REDUCE,tab);
                f->resize(primary,minx,miny,maxx-minx,maxy-miny);
                for (int i=0;i<g->children();i++) {
                    Fl_Widget *o=g->child(i);
                    auto &d=dimensions[i];
                    o->resize(d.x,d.y,d.w,d.h);
                }
                init_sizes(primary->parent());
                action_end();
            }
        } else if (path=="Widget/Delete/Delete") {
            action_start(ACTION_POPUP_WIDGET_DELETE,tab);
            selected.erase(primary);
            delete primary;
            select_primary(NULL);
            action_end();
        } else if (path=="Selection/Clear") {
            clear_selection();
        } else if (path=="Selection/Delete/Delete") {
            action_start(ACTION_POPUP_SELECTION_DELETE,tab);
            remove_children(selected);
            for (auto &o : selected) 
                delete o;
            clear_selection();
            action_end();
        } else if (path=="Layout/Add New") {
            action_start(ACTION_LAYOUT_ADD);
            std::string tab_name;
            for (int n=1;tab_name.empty() || tabs_get(tab_name);n++)
                tab_name="Layout"+std::to_string(n);
            tabs_add(tab_name);
            action_end();
            need_redraw_overlay=true;
        } else if (path=="Layout/Rename") {
            const char *name=fl_input("Rename Layout",tab->label());
            for (int i=0;name && i<tabs->children();i++) {
                if (strcmp(name,tabs->child(i)->label())==0) {
                    fl_message("Layout with name '%s' already exists!",name);
                    name=NULL;
                }
            }
            if (name) {
                action_start(ACTION_LAYOUT_RENAME);
                const std::string old_name=tab->label();
                tabs_rename(tab,name);
                FactoryInterface *f=factories.get_factory(old_name);
                factories.rename_factory(f,name);
                action_end();
            }
            need_redraw_overlay=true;
        } else if (path=="Layout/Popout") {
            tab->popout=new Tab::PopoutTab(*tab,tab->x()+50,tab->y()+50,tab->w(),tab->h());
            tab->popout->show();
        } else if (path=="Layout/Popin") {
            tab->popout->window_closed();
        } else if (path=="Layout/Delete/Delete") {
            action_start(ACTION_LAYOUT_DELETE);
            clear_selection();
            tabs->remove(tab);
            Fl::delete_widget(tab);
            if (tabs->children()==0)
                tabs_add("Layout1");
            update_layout_widgets(NULL);
            action_end();
        }
    }

    bool need_redraw_overlay=false;
    Tab *last_tab=nullptr;
    virtual int handle(int e) {
        if (e!=FL_NO_EVENT && e!=FL_ENTER && e!=FL_LEAVE) {
            Tab *t=tabs_current();
            if (!t || t->popout)
                t=(Tab*)tabs->value();
            if (!t->popout && handle(this,t,e))
                return 1;
        }
        return Fl_Overlay_Window::handle(e);
    }

    int handle(Fl_Overlay_Window *win,Tab *tab,int e) {
//printf("XXXX handle win=%s tab=%s e=%s\n",win->label(),tab->label(),fl_eventnames[e]);

        if (tab!=last_tab) { // tab changed
            clear_selection(std::string("tab change old=")+last_tab->label()+" new="+tab->label());
            action_end();
            last_tab=tab;
            need_redraw_overlay=true;
        }

        int mx=Fl::event_x(),my=Fl::event_y(),button=Fl::event_button();
        if (!tab->xy_inside(win,mx,my)) // click outside tab body
            return 0;

        switch (e) {
        case FL_PUSH: {
            if (button==FL_RIGHT_MOUSE) {
                tab_body_right_click_menu(mx,my);

            } else if (button==FL_LEFT_MOUSE) {

                sx=mx; sy=my;

                // check for click on primary sizing box
                if (xy_inside_widget(mx,my,primary) && mx>=primary->x()+primary->w()-10 && my>=primary->y()+primary->h()-10) {
                    action_start(ACTION_SIZE,tab);
                    break;
                }

            }
            break;
        }
        case FL_RELEASE: {

            if (button==FL_LEFT_MOUSE) {

                if (action==ACTION_SELECT) { // end ACTION_SELECT: add widgets under srect to selection

                    if (srect.w()>0) {
                        auto tab_widgets=tab->get_child_widgets();
                        for (auto &o : tab_widgets) {
                            if (widget_inside_rect(o,srect.x(),srect.y(),srect.w(),srect.h()))
                                selected.insert(o);
                        }
                    }
                    srect={mx,my,mx,my};
                    action_end();
                    need_redraw_overlay=true;
                    break;
                } 
 
                if (action==ACTION_SIZE) { // end ACTION_SIZE
                    action_end();
                    break;
                }

                if (action==ACTION_DRAG) { // end ACTION_DRAG 
                    action_end();
                    break;
                }

                std::vector<Fl_Widget*> under_mouse;
                get_widgets_at(tab,mx,my,under_mouse);
                if (!under_mouse.empty()) {
                    if (Fl::event_shift()) { // add widgets under mouse to selection
                        for (auto &o : under_mouse)
                            selected.insert(o);
                        select_primary(under_mouse[under_mouse.size()-1]);
                    } else {
                        // count how many widgets under mouse are currently selected
                        int last_idx=-1,count=0;
                        for (int i=0;i<(int)under_mouse.size();i++) {
                            if (selected.count(under_mouse[i])) {
                                last_idx=i;
                                count++;
                            }
                        }

                        if (Fl::event_ctrl() && count==1) { // toggle thru z-order
                            if (primary!=under_mouse[last_idx]) 
                                select_primary(under_mouse[last_idx]);
                            else {
                                selected.erase(under_mouse[last_idx]);
                                select_primary(last_idx>0 ? under_mouse[last_idx-1] : NULL);
                            }
                        } else { // select widget on top and deselect the rest
                            if (Fl::event_ctrl()) {
                                for (auto &o : under_mouse)
                                    selected.erase(o);
                            } else
                                clear_selection("release");
                            select_primary(under_mouse[under_mouse.size()-1]);
                        }
                    }
                }
            }
            break;
        }
        case FL_DRAG: {
            if (action==ACTION_SIZE) { // resize primary
                int rx=snapx(mx+snap),ry=snapy(my+snap);
                if (rx>primary->x() && ry>primary->y()) {
                    FactoryInterface *f=widgets.get_factory(primary);

                    if (!primary->as_group() || !Fl::event_shift()) 
                        f->resize(primary,primary->x(),primary->y(),rx-primary->x(),ry-primary->y());
                    else { // dont resize children if shift pressed
                        Fl_Group *p=primary->as_group();
                        struct XYWH { int x,y,w,h; };
                        std::vector<XYWH> dimensions;
                        for (int i=0;i<p->children();i++) {
                            Fl_Widget *o=p->child(i);
                            dimensions.push_back({o->x(),o->y(),o->w(),o->h()});
                        }
                        f->resize(primary,primary->x(),primary->y(),rx-primary->x(),ry-primary->y());
                        
                        for (int i=0;i<p->children();i++) {
                            Fl_Widget *o=p->child(i);
                            auto &d=dimensions[i];
                            o->resize(d.x,d.y,d.w,d.h);
                        }
                    }

                    need_redraw_overlay=true;
                }

            } else if (action==ACTION_SELECT) { // resize selection area
                srect={srect.x1,srect.y1,mx,my};
                need_redraw_overlay=true;

            } else if (action==ACTION_DRAG) { // drag primary
                int xchange=snapx(primary->x()+mx-sx)-primary->x(),ychange=snapy(primary->y()+my-sy)-primary->y();
                if (xchange || ychange) {
                    std::set<Fl_Widget*> selected_without_children=selected;
                    remove_children(selected_without_children);
                    for (auto &o : selected_without_children)
                        o->position(o->x()+xchange,o->y()+ychange);
                    sx=snapx(mx); sy=snapy(my);
                    need_redraw_overlay=true;
                }

            } else if (xy_inside_widget(mx,my,primary)) { // drag on primary selection
                action_start(ACTION_DRAG,tab);

            } else { 

                // drag on selected => make primary and start dragging
                for (auto &o : selected) {
                    if (xy_inside_widget(mx,my,o)) {
                        select_primary(o);
                        action_start(ACTION_DRAG,tab);
                        break;
                    }
                }
                if (action!=ACTION_NONE) break;

                // start of new srect selection
                if (Fl::event_shift()) {
                    std::vector<Fl_Widget*> under_mouse;
                    get_widgets_at(tab,mx,my,under_mouse);
                    for (auto &o : under_mouse)
                        selected.insert(o);
                } else
                    clear_selection("new srect");
                select_primary(NULL);
                srect={mx,my,mx,my};
                action_start(ACTION_SELECT,tab);
            }
            break;
        }
        case FL_FOCUS: break;
        case FL_UNFOCUS: break;
        case FL_KEYDOWN: {
            if (Fl::event_key()==FL_Escape)
                clear_selection("escape");
            if (Fl::event_key()=='u' || (Fl::event_ctrl() && Fl::event_key()=='z'))
               undo();
            if (Fl::event_key()=='r' || (Fl::event_ctrl() && Fl::event_key()=='y'))
               redo();
            if (Fl::event_ctrl() && (Fl::event_key()=='c' || Fl::event_key()=='x')) { // Copy or Cut
               const bool is_cut=Fl::event_key()=='x';
               if (is_cut) action_start(ACTION_CUT,tab);
               auto w=selected;
               remove_children(w);

               std::string out;
               for (auto &o : w) {
                   std::vector<Fl_Widget*> kids;
                   widgets.get_child_widgets(o,kids);
                   out+=widgets.get_properties(o).serialize()+"\n";
                   for (auto &o : kids)
                       out+=widgets.get_properties(o).serialize()+"\n";
                   if (is_cut) delete o;
               }
               Fl::copy(out.data(),out.size(),1);
               if (is_cut) {
                   clear_selection("is cut");
                   action_end();
               }

            }
            if (Fl::event_ctrl() && Fl::event_key()=='v') // Paste
                Fl::paste(*win,1);

            break;
        }
        case FL_PASTE: {
            if (!Fl::event_text()) break;

            std::string str(Fl::event_text(),Fl::event_length());
            std::vector<std::string> wprops=split(str,'\n');

            Fl_Group *g=tab->as_group();
            if (primary && primary->as_group()) g=primary->as_group();

            const int imax=std::numeric_limits<int>::max();
            int minx=imax,miny=imax;
            for (auto &propstr : wprops) {
                PropertyMap props;
                props.deserialize(propstr);
                FactoryInterface *f=factories.get_factory(props["factory"],props["name"]);
                if (!f) continue;

                int x=atoi(props["x"].c_str()),y=atoi(props["y"].c_str());
                if (x<minx) minx=x;
                if (y<miny) miny=y;
            }
            if (minx==imax) break;

            clear_selection("paste");
            action_start(ACTION_PASTE,tab);

            std::map<std::string,std::string> namemap;
            for (auto &propstr : wprops) {
                PropertyMap props;
                props.deserialize(propstr);
                FactoryInterface *f=factories.get_factory(props["factory"],props["name"]);
                if (!f) continue;

                auto oldname=props["name"];
                auto name=oldname;
                if (widgets.get_info(name)) {
                    while (!name.empty() && std::isdigit(name[name.size()-1]))
                        name=name.substr(0,name.size()-1);
                    int i=0;
                    while (widgets.get_info(name+std::to_string(i))) i++;
                    name+=std::to_string(i);
                }
                namemap[oldname]=name;

                int x=atoi(props["x"].c_str())+mx-minx,y=atoi(props["y"].c_str())+my-miny,w=atoi(props["w"].c_str()),h=atoi(props["h"].c_str());
                auto label=props["label"];

                g->begin();
                Fl_Widget *o=f->create(&widgets,name,x,y,w,h,label);
                g->add(o);
                g->end();

                const static std::set<std::string> ignore{ "layout","factory","name","x","y","w","h","label" };
                props["parent"]=namemap[props["parent"]];
                for (auto &nv : props) {
                    if (!ignore.count(nv.first))
                        f->set_property(&widgets,o,nv.first,nv.second);
                }

                selected.insert(o);
            }
            action_end();
            need_redraw_overlay=true;
        }
        default:
            break;
        }
        return 1;
    }

    virtual void draw_overlay() { 
        Tab *tab=tabs_current();
        if (!tab->popout)
            render_overlay(tab);
        else
            tab->popout->redraw_overlay();
    }

    void render_overlay(Tab *tab) {
        auto tab_widgets=tab->get_child_widgets();
        bool has_primary=false;
        for (auto &o : tab_widgets) {
            if (widget_inside_rect(o,srect.x(),srect.y(),srect.w(),srect.h())) {
                fl_color(FL_RED);
                fl_rect(o->x(),o->y(),o->w(),o->h());
            } else if (!widgets.get_factory(o)->border_visible()) {
                fl_color(FL_YELLOW);
                fl_rect(o->x(),o->y(),o->w(),o->h());
            } 
            if (o->parent()->resizable()==o && o->w()>=10 && o->h()>=10) {
                fl_color(FL_GREEN);
                fl_rectf(o->x()+5,o->y()+5,5,5);
            }
            if (o==primary)
                has_primary=true;
        }
        for (auto &o : selected) {
            if (o!=primary) {
                fl_color(FL_RED);
                fl_rect(o->x(),o->y(),o->w(),o->h());
            }
        }
        if (has_primary) {
            fl_color(FL_CYAN);
            fl_rect(primary->x(),primary->y(),primary->w(),primary->h());
            if (primary->w()>10 && primary->h()>10)
                fl_rect(primary->x()+primary->w()-10,primary->y()+primary->h()-10,10,10);
        }
        if (action==ACTION_SELECT) {
            fl_color(FL_BLUE);
            fl_rect(srect.x(),srect.y(),srect.w(),srect.h());
        }
    }

    // cap window & overlay redrawing at max fps
    double max_fps=24.0;
    bool need_draw=false,really_draw=false;
    virtual void draw() { 
        if (really_draw) {
            Fl_Overlay_Window::draw();
            really_draw=false;
        } else
            need_draw=true;
    }
    static void draw_timer_cb(void *userdata) {
        DesignWindow *win=(DesignWindow*)userdata;
        if (win->need_draw) {
            win->need_draw=false;
            win->really_draw=true;
            win->redraw();
        }
        if (win->need_redraw_overlay) {
            win->need_redraw_overlay=false;
            win->redraw_overlay();
            win->properties_window->update_values();
        }
        Fl::repeat_timeout(1.0/win->max_fps,draw_timer_cb,userdata);
    }

    DesignWindow(int x,int y,int w,int h,const std::string &label) : Fl_Overlay_Window(x,y,w,h,"") {
        copy_label(label.c_str());

        menu_height=20;
        menu=new Fl_Menu_Bar(0,0,w,menu_height); 

        tab_label_height=30;
        tabs=new Fl_Tabs(0,menu_height,w,h-menu_height,"tabs");
        tabs->box(FL_PLASTIC_THIN_DOWN_BOX);
        tabs->callback((Fl_Callback*)_tabs_callback,this);
        tabs->when(FL_WHEN_RELEASE|FL_WHEN_CHANGED|FL_WHEN_NOT_CHANGED);
        tabs->align(Fl_Align(FL_ALIGN_CENTER));
        tabs->end();
        resizable(tabs);

//        set_non_modal();
        end();

        menu_add("File/New");
        menu_add("File/Load Layouts...");
        menu_add("File/Load Layouts as Widgets...");
        menu_add("File/Save As...");
        menu_add("File/Quit/Exit");
        menu_add("Edit/Properties Window/Show");
        menu_add("Edit/Properties Window/Hide");

        tabs_add("Layout1");

        properties_window=new MyPropertiesWindow(*this,x+100,y+100);

        Fl::add_timeout(1.0/max_fps,draw_timer_cb,(void*)this); 
    }

    virtual ~DesignWindow() { }

    void menu_clear() { menu->clear(); }
    bool menu_add(const std::string &path,const int shortcut=0,const bool is_active=true) {
        int flags=0;
        if (!is_active) flags|=FL_MENU_INACTIVE;
        if (path.size()>3 && path.substr(path.size()-3)=="---") flags|=FL_MENU_DIVIDER;
        menu->add(path.c_str(),shortcut,_menu_callback,this,flags);
        return true;
    }
    bool menu_remove(const std::string &path) {
        int idx=menu->find_index(path.c_str());
        if (idx>=0) menu->remove(idx);
        return idx>=0;
    }
    void menu_activate(const std::string &path) {
        Fl_Menu_Item *i=(Fl_Menu_Item*)menu->find_item(path.c_str());
        if (i) i->activate();
    }
    void menu_deactivate(const std::string &path) {
        Fl_Menu_Item *i=(Fl_Menu_Item*)menu->find_item(path.c_str());
        if (i) i->deactivate();
    }

    void tabs_clear() { tabs->clear(); }
    Tab *tabs_get(const std::string &label) {
        for (int i=0;i<tabs->children();i++) {
            Fl_Widget *t=tabs->child(i);
            if (t && label==t->label())
                return (Tab*)t; 
        }
        return NULL;
    }
    Tab *tabs_current() {
        if (!last_tab || !last_tab->popout)
            last_tab=(Tab*)tabs->value();
        return last_tab;
    }
    Tab *tabs_add(const std::string &label) {
        Tab *t=tabs_get(label);
        if (!t) {
            t=new Tab(*this,tabs->x(),tabs->y()+tab_label_height,tabs->w(),tabs->h()-tab_label_height);
            t->labelfont(FL_HELVETICA_BOLD);
            t->box(FL_DOWN_BOX);
            t->color(FL_BACKGROUND_COLOR);
            t->end();
            tabs->add(t);
            tabs->resizable(t);
            t->copy_label(label.c_str());
            last_tab=t;
        }
        return t;
    }
    void tabs_rename(Tab *tab,const std::string &name) {
        tab->rename(name);
    }
    bool tabs_remove(const std::string &label) {
        Tab *t=tabs_get(label);
        if (t) {
            tabs->remove(t);
            Fl::delete_widget(t);
            return true;
        }
        return false;
    }

    virtual void tabs_callback() { 
        // tab switched
        clear_selection("tab change cb");
        action_end();
        last_tab=(Tab*)tabs->value();
        need_redraw_overlay=true;
    }
    virtual void menu_callback(const std::string &path) { 
        if (path=="File/New") {
            action_start(ACTION_FILE_NEW);
            clear_selection();
            tabs_clear();
            tabs_add("Layout1");
            action_end();
        } else if (path=="File/Load Layouts...") {
            Fl_File_Chooser chooser(".","*.layout",Fl_File_Chooser::SINGLE,"Load Layouts");
            chooser.show();
            while (chooser.shown()) Fl::wait(); 
            const char *filename=chooser.value();

            if (filename) {
                action_start(ACTION_FILE_LOAD_LAYOUTS);
                auto err=load(filename);
                if (!err.empty()) fl_message("%s",err.c_str());
                action_end();
                need_redraw_overlay=true;
            }
        } else if (path=="File/Load Layouts as Widgets...") {
            Fl_File_Chooser chooser(".","*.layout",Fl_File_Chooser::SINGLE,"Load Widgets");
            chooser.show();
            while (chooser.shown()) Fl::wait(); 
            const char *filename=chooser.value();

            if (filename) {
// TODO - action/undo ?
                auto err=factories.load_layouts_as_widgets(filename);
                if (!err.empty()) fl_message("%s",err.c_str());
            }
        } else if (path=="File/Save As...") {
            Fl_File_Chooser chooser(".","*.layout",Fl_File_Chooser::CREATE,"Save Layouts");
            chooser.show();
            while(chooser.shown()) Fl::wait(); 
            const char *filename=chooser.value();
            FILE *fd=NULL;
            if (filename) {
                fd=fopen(filename,"w");
                if (!fd) fl_message("Could not open file %s for writing",filename);
            }
            if (fd) {
                for (int i=0;i<tabs->children();i++) {
                    Tab *t=(Tab*)tabs->child(i);
                    const std::string layout_name=escape(t->label());

                    auto tab_widgets=t->get_child_widgets();
                    for (auto &o : tab_widgets) {
                        PropertyMap props=widgets.get_properties(o);
                        props["layout"]=layout_name;
                        fprintf(fd,"%s\n",props.serialize().c_str());
                    }
                }
                fclose(fd);
            }
        } else if (path=="File/Quit/Exit") {
            exit(0);
        } else if (path=="Edit/Properties Window/Show") {
            properties_window->show();
        } else if (path=="Edit/Properties Window/Hide") {
            properties_window->hide();
        }
    }

    std::string load(const std::string &filename) {
		auto layouts=load_layout_file(filename);
		if (layouts.empty()) 
			return "Could not read file "+filename;

        std::string err;
        tabs_clear();

        factories.init();
		for (auto &i : layouts) 
            factories.add_layout_widget_factory(i.first,i.second);

        clear_selection();
		for (auto &i : layouts) {
			auto layout=i.first;
            tabs_remove(layout);
			Tab *t=tabs_add(layout);
			err=widgets.load_layout(factories,t->as_group(),filename,layout);
			if (!err.empty()) break;
		}

        if (tabs->children()==0) tabs_add("Layout1");
        return err;
	}

    typedef std::vector<std::string> UndoPoint;
    std::vector<UndoPoint*> undo_points;
    int undo_idx=0;
    void create_undo_point() {
        UndoPoint *u=new UndoPoint;
        for (int i=0;i<tabs->children();i++) {
            Tab *t=(Tab*)tabs->child(i);
            const std::string layout_name=escape(t->label());

            auto tab_widgets=t->get_child_widgets();
            if (tab_widgets.empty()) {
                PropertyMap props={ { "layout", layout_name } };
                u->push_back(props.serialize());
            }
            for (auto &o : tab_widgets) {
                PropertyMap props=widgets.get_properties(o);
                props["layout"]=layout_name;
                props["x"]=std::to_string(atoi(props["x"].c_str())-t->x());
                props["y"]=std::to_string(atoi(props["y"].c_str())-t->y());
                u->push_back(props.serialize());
            }
        }
        
        for (int i=undo_points.size();i>undo_idx;--i)
            delete undo_points[i-1];
        undo_points.resize(undo_idx+1);
        undo_points[undo_idx++]=u;

        if (undo_points.size()>50) {
            delete undo_points[0];
            undo_points.erase(undo_points.begin());
            undo_idx--;
        }
    }

    void do_undo(UndoPoint *u) {
        const std::string current_tab=tabs_current()->label();
        clear_selection();

        std::map<std::string,std::vector<PropertyMap> > layouts;
        for (auto &propstr : *u) {
            PropertyMap props;
            props.deserialize(propstr);
            const auto tab_name=props["layout"];
            layouts[tab_name].push_back(props);
        }

        for (int i=0;i<tabs->children();) {
            Tab *tab=(Tab*)tabs->child(i);
            if (layouts.count(tab->label())==0)
                tabs_remove(tab->label());
            else { 
                tab->clear();
                i++;
            }
        }

        factories.init();
        for (auto &i : layouts)
            factories.add_layout_widget_factory(i.first,i.second);

        for (auto &p : layouts) {
            const auto tab_name=p.first;
            Tab *t=tabs_add(tab_name);
            t->as_group()->begin();

            for (auto &props : p.second) {

                auto &factory=props["factory"];
                if (factory.empty()) continue;

                auto &name=props["name"];
                FactoryInterface *f=factories.get_factory(factory,name);
                int x=t->x()+atoi(props["x"].c_str()),y=t->y()+atoi(props["y"].c_str()),w=atoi(props["w"].c_str()),h=atoi(props["h"].c_str());
                auto label=props["label"];

                Fl_Widget *o=f->create(&widgets,name,x,y,w,h,label);

                const static std::set<std::string> ignore{ "layout","factory","name","x","y","w","h","label" };
                for (auto &nv : props) {
                    if (!ignore.count(nv.first))
                        f->set_property(&widgets,o,nv.first,nv.second);
                }
            }
            t->as_group()->end();
            t->as_group()->redraw();
            init_sizes(t->as_group()->parent());
        }

        if (tabs->children()==0) tabs_add("Layout1");

        Fl_Group *t=tabs_get(current_tab);
        if (t) tabs->value(t);
        last_tab=nullptr;

        need_redraw_overlay=true;
        redraw();
    }

    void undo() {
        if (undo_idx==undo_points.size()) {
           create_undo_point(); // for redo
           undo_idx--;
        }
        if (undo_idx>0) 
            do_undo(undo_points[--undo_idx]);
    }

    void redo() {
        if (undo_idx<undo_points.size()) {
            if (++undo_idx<undo_points.size())
                do_undo(undo_points[undo_idx]);
        }
    }

    void update_layout_widgets(Tab *changed_tab) {

        auto factory_names=factories.get_factory_names(); // all factories
        if (!changed_tab) {
            // make LayoutWidgetFactorys factories match tabs
            for (int i=0;i<tabs->children();i++) {
                Tab *tab=(Tab*)tabs->child(i);
                FactoryInterface *f=factories.get_factory(tab->label());

                std::vector<PropertyMap> layout;
                auto tab_widgets=tab->get_child_widgets();
                for (auto &o : tab_widgets)
                    layout.push_back(widgets.get_properties(o));

                if (!f) {
                    // New Layout added => add new LayoutWidgetFactory
                    factories.add_layout_widget_factory(tab->label(),layout);
                } else {
                    // Update layout for existing factory
                    ((LayoutWidgetFactory*)f)->update_layout(layout);
                }
            }
            // remove any layout widget factories without tabs
            auto factory_names_copy=factory_names;
            for (auto& factory_name : factory_names_copy) {
                FactoryInterface *f=factories.get_factory(factory_name);
                if (f->factory_type!=FactoryInterface::FACTORY_TYPE_LAYOUT_WIDGETS) continue;

                Tab *tab=tabs_get(factory_name);
                if (!tab) {
                    // Layout was deleted => erase factory and widgets
                    auto name2infos=widgets.get_name2infos();
                    for (auto &p : name2infos) {
                        WidgetInfo *info=p.second;
                        if (info->factory==f)
                            delete info->o;
                    }
                    factories.remove_factory(f);
                    factory_names.erase(factory_name);
                    delete f;
                }
            }
        }

        // Layout changes => update existing layout widgets
        std::vector<LayoutWidgetFactory*> lwf;
        for (auto& factory_name : factory_names) {
            FactoryInterface *f=factories.get_factory(factory_name);
            if (f->factory_type==FactoryInterface::FACTORY_TYPE_LAYOUT_WIDGETS) 
                lwf.push_back((LayoutWidgetFactory*)f);
        }
        std::sort(lwf.begin(),lwf.end(),[&](LayoutWidgetFactory *a,LayoutWidgetFactory *b) -> bool
        { 
            return !has_dependency(a->factory_name,b->factory_name);
        });
        for (auto f : lwf) {
            Tab *tab=tabs_get(f->factory_name);
            std::vector<PropertyMap> layout;
            auto tab_widgets=tab->get_child_widgets();
            for (auto &o : tab_widgets)
                layout.push_back(widgets.get_properties(o));

            LayoutWidgetFactory &layout_factory=*(LayoutWidgetFactory*)f;
            layout_factory.update_layout(layout);

            auto name2infos=widgets.get_name2infos();
            for (auto &p : name2infos) {
                WidgetInfo *info=p.second;
                if (info->factory==f) 
                    ((LayoutWidget*)info->o)->update_layout(layout_factory,layout);
            }
        }
    }

    bool has_dependency(const std::string &factory_name1,const std::string &factory_name2) {
        // does name1 layout widget have dependency on name2 layout widget
        FactoryInterface *f=factories.get_factory(factory_name1);
        if (f && f->factory_type==FactoryInterface::FACTORY_TYPE_LAYOUT_WIDGETS) {
            if (factory_name1==factory_name2)
                return true;
            LayoutWidgetFactory &layout_factory=*(LayoutWidgetFactory*)f;
            for (auto &pm : layout_factory.layout) {
                const std::string &factory_name=pm["factory"];
                if (factory_name==factory_name2 || has_dependency(factory_name,factory_name2)) 
                    return true;
            }
        }
        return false;
    }
};

} // namespace fltklayout


