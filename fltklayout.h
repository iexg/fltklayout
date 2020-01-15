#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Group.H>

#include <string>
#include <map>
#include <set>
#include <vector>
#include <functional>

namespace fltklayout {

std::string escape(const std::string &in);
std::string unescape(const std::string &in);

struct PropertyMap : public std::map<std::string,std::string> {
    PropertyMap() = default;
    PropertyMap(std::initializer_list<value_type> init) : std::map<std::string,std::string>(init) { }
    void deserialize(const std::string &props);
    std::string serialize();
};

template <typename T>
inline T combine_maps(const T &m1,const T &m2) {
    T m(m1);
    m.insert(m2.begin(),m2.end());
    return m;
}
template <typename T>
inline std::vector<typename T::key_type> map_keys_to_vector(const T &m) {
    std::vector<typename T::key_type> keys;
    for (auto &p : m) keys.push_back(p.first);
    return keys;
}
inline std::vector<std::string> split(std::string str,const char separator) {
    std::vector<std::string> out;
    while (!str.empty()) {
        const size_t idx=str.find(separator);
        if (idx!=std::string::npos){
            out.push_back(str.substr(0,idx));
            str=str.substr(idx+1);
            if (str.empty()) out.push_back("");
        } else {
            out.push_back(str);
            str.clear();
        }
    }
    return out;
}

struct FactoryInterface;
struct Widgets;
struct Factories;

struct WidgetInfo { // extra info stored for each named widget
    Widgets *widgets=nullptr;              // widget collection this named widget belongs to
    std::string name;                      // widget name
    FactoryInterface *factory=nullptr;     // factory that created this widget
    Fl_Widget_Tracker *tracker=nullptr;    // used to track if FLTK deletes this widget
    Fl_Widget *o=nullptr;                  // the actual widget itself
    std::function<void(Fl_Widget*,WidgetInfo*)> callback; // callback

    WidgetInfo() = default;
    virtual ~WidgetInfo() { delete tracker; }

    WidgetInfo *init(Widgets *widgets,const std::string &name,FactoryInterface *factory,Fl_Widget *o) { 
        this->widgets=widgets;
        this->name=name;
        this->factory=factory;
        this->tracker=new Fl_Widget_Tracker(o);
        this->o=o;
        return this;
    }
    bool exists() { return tracker->exists(); }
};
struct FactoryInterface { // a factory that can manufacture widgets, and also get/set properties on existing widgets
    Factories *factories;     // factory collection that this factory belongs to
    std::string factory_name; // factory name

    static const int FACTORY_TYPE_NONE=0;
    static const int FACTORY_TYPE_LAYOUT_WIDGETS=1;
    int factory_type=FACTORY_TYPE_NONE;

    FactoryInterface(Factories *factories=NULL,const std::string &factory_name="")
    : factories(factories),factory_name(factory_name)
    { }
    virtual ~FactoryInterface() { }

    virtual std::string name() const { return factory_name; }

    virtual bool border_visible()=0; // used by designer to highlight invisible widgets
    virtual bool is_group()=0;       // used by designer to tell if widget should be treated as a group 

    // virtual methods provided by real factories...
    virtual Fl_Widget *create(Widgets *widgets,const std::string &name,int x,int y,int w,int h,const std::string &label)=0;
    virtual bool set_property(Widgets *widgets,Fl_Widget *o,const std::string &key,const std::string &val)=0;
    virtual std::string get_property(Widgets *widgets,Fl_Widget *o,const std::string &key)=0;
    virtual std::vector<std::string> get_property_names()=0;
    virtual PropertyMap get_property_info()=0;
    virtual void resize(Fl_Widget *o,int x,int y,int w,int h)=0;
};

// load file from disk into STL data structure
std::map<std::string,std::vector<PropertyMap> > load_layout_file(const std::string &filename);

class Factories { // a collection of widget factories
    typedef std::map<std::string,FactoryInterface*> FactoryMap;
    FactoryMap factories; // map "factory_name" => factory, or "name=widget_name" => factory

public:
    Factories();
    virtual ~Factories();

    virtual void init();

    virtual void add_factory(FactoryInterface *factory,std::string widget_name="") {
        const size_t sep=widget_name.find_first_of(".:0123456789");
        if (sep!=std::string::npos) 
            widget_name=widget_name.substr(0,sep);
        if (!widget_name.empty())
            factories["name="+widget_name]=factory;
        else
            factories[factory->name()]=factory;
    }

    virtual FactoryInterface *get_factory(const std::string &factory_name,std::string widget_name=std::string()) {
        const size_t sep=widget_name.find_first_of(".:0123456789");
        if (sep!=std::string::npos) 
            widget_name=widget_name.substr(0,sep);
        auto i=factories.find("name="+widget_name);
        if (i!=factories.end()) return i->second;
        i=factories.find(factory_name);
        return i!=factories.end() ? i->second : NULL;
    }

    virtual bool rename_factory(FactoryInterface *factory,const std::string new_name) {
        if (factories.count(new_name)) return false; // already exists
        factories.erase(factory->factory_name);
        factory->factory_name=new_name;
        factories[new_name]=factory;
        return true;
    }

    virtual void remove_factory(FactoryInterface *f) {
        if (f) factories.erase(f->factory_name);
    }

    virtual std::set<std::string> get_factory_names() { // returns names of all available factories
        std::set<std::string> names;
        for (auto &p : factories) {
            if (p.first.size()<5 || p.first.compare(0,5,"name=")!=0)
                names.insert(p.first);
        }
        return names;
    }

    std::string load_layouts_as_widgets(const std::string &filename);
    std::string add_layout_widget_factory(const std::string &factory_name,const std::vector<PropertyMap> &layout);
};

class Widgets { // a collection of named widgets

public:
    typedef std::map<std::string,WidgetInfo*> Name2InfoMap;
    typedef std::map<Fl_Widget*,WidgetInfo*> Widget2InfoMap;

private:
    Name2InfoMap name2info;     // map   name => WidgetInfo
    Widget2InfoMap widget2info; // map widget => WidgetInfo

    WidgetInfo *check_exists(WidgetInfo *winfo) {
        if (winfo->exists())
            return winfo;
        name2info.erase(winfo->name);
        widget2info.erase(winfo->o);
        delete winfo;
        return NULL;
    }

public:
    Widgets() { }
    virtual ~Widgets() { 
        for (auto &p : name2info)
            delete p.second;
    }

    std::string load_layout(Factories &factories,
                            Fl_Group *grp,
                            const std::string &filename,
                            const std::string &layout_name,
                            const std::string &prefix="",
                            const bool resize_group=false,
                            const bool zero_xy=false,
                            const int at_x=0,
                            const int at_y=0); 

    bool is_managed(Fl_Widget *o) { return get_info(o); }

    Fl_Widget *get_widget(const std::string &name) { 
        WidgetInfo *winfo=get_info(name);
        return winfo ? winfo->o : NULL;
    }
    std::string get_name(Fl_Widget *o) {
        WidgetInfo *winfo=get_info(o);
        return winfo ? winfo->name : "";
    }
    FactoryInterface *get_factory(Fl_Widget *o) {
        WidgetInfo *winfo=get_info(o);
        return winfo ? winfo->factory : NULL;
    }
    std::string get_factory_name(Fl_Widget *o) {
        WidgetInfo *winfo=get_info(o);
        return winfo ? winfo->factory->name() : "";
    }
    WidgetInfo *get_info(Fl_Widget *o) {
        auto i=widget2info.find(o);
        return i!=widget2info.end() ? check_exists(i->second) : NULL;
    }
    WidgetInfo *get_info(const std::string &name) {
        auto i=name2info.find(name);
        return i!=name2info.end() ? check_exists(i->second) : NULL;
    }
    bool rename(Fl_Widget *o,const std::string &new_name) { 
        WidgetInfo *winfo=get_info(o);
        if (!winfo) return false;

        auto name=get_name(o);
        if (name!=new_name) {
            if (get_widget(new_name)) return false; // exists
            name2info.erase(name);
            winfo->name=new_name;
            name2info[new_name]=winfo;
        }
        return true;
    }
    bool rename(const std::string &name,const std::string &new_name) {
        return rename(get_widget(name),new_name);
    }
    std::set<std::string> get_names() { // get names of all managed widgets
        std::set<std::string> names;
        for (auto &p : name2info) {
            if (p.second->exists())
                names.insert(p.first);
        }
        return names;
    }

    const Name2InfoMap& get_name2infos() { // gets name=>info map of all managed widgets
        for (auto i=name2info.begin();i!=name2info.end();) {
            if (i->second->exists())
                ++i;
            else {
                auto next=i;
                ++next;
                widget2info.erase(i->second->o);
                name2info.erase(i);
                i=next;
            }
        }
        return name2info;
    }
    const Widget2InfoMap& get_widget2infos() { // gets widget=>info map of all managed widgets
        for (auto i=widget2info.begin();i!=widget2info.end();) {
            if (i->second->exists())
                ++i;
            else {
                auto next=i;
                ++next;
                name2info.erase(i->second->name);
                widget2info.erase(i);
                i=next;
            }
        }
        return widget2info;
    }

    bool remove(Fl_Widget *o) {
        WidgetInfo *winfo=get_info(o);
        if (!winfo) return false; // not found
        name2info.erase(winfo->name);
        widget2info.erase(o);
        delete winfo;
        return true;
    }
    void remove(const std::string &name) { remove(get_widget(name)); }

    void add_widget(WidgetInfo *winfo) {
        name2info[winfo->name]=winfo;
        widget2info[winfo->o]=winfo;
    }
    std::string get_unique_name() {
        for (int i=0;;i++) {
            std::string name="w"+std::to_string(i);
            if (!get_widget(name)) 
                return name;
        }
    }
    void get_child_widgets(Fl_Widget *o,std::vector<Fl_Widget*> &child_widgets) {
        // note: start widget can be managed or unmanaged
        Fl_Group *g=o ? o->as_group() : NULL;
        for (int i=0;g && i<g->children();i++) {
            Fl_Widget *o=g->child(i);
            if (!is_managed(o)) continue;

            child_widgets.push_back(o);
            if (o->as_group()) get_child_widgets(o,child_widgets);
        }
    }

    Fl_Widget *get_parent(Fl_Widget *o) { // gets managed parent
        while (o) {
            if (is_managed(o->parent()))
                return o->parent();
            o=o->parent();
        }
        return NULL;
    }

    PropertyMap get_properties(Fl_Widget *o) {
        PropertyMap props;
        FactoryInterface *f=get_factory(o);
        if (f) {
            for (auto &name : f->get_property_names())
                props[name]=f->get_property(this,o,name);
        }
        return props;
    }
};

struct WidgetFactoryBase : public FactoryInterface {
    WidgetFactoryBase(Factories *factories,const std::string &factory_name)
    : FactoryInterface(factories,factory_name)
    { }
    virtual ~WidgetFactoryBase() { }

    virtual bool border_visible() { return true; }
    virtual bool is_group() { return false; }

    virtual Fl_Widget *create(Widgets *widgets,const std::string &name,int x,int y,int w,int h,const std::string &label)=0;

    virtual bool set_property(Widgets *widgets,Fl_Widget *o,const std::string &key,const std::string &val);
    virtual std::string get_property(Widgets *widgets,Fl_Widget *o,const std::string &key);
    virtual std::vector<std::string> get_property_names();
    virtual PropertyMap get_property_info();
    virtual void resize(Fl_Widget *o,int x,int y,int w,int h) { o->resize(x,y,w,h); }

    PropertyMap get_widget_properties(Widgets *widgets,Fl_Widget *o) {
        PropertyMap props;
        for (auto &name : get_property_names())
            props[name]=get_property(widgets,o,name);
        return props;
    }
};

struct LayoutWidgetFactory : public WidgetFactoryBase {
    std::vector<PropertyMap> layout;

    virtual bool border_visible() { return false; }
    virtual bool is_group() { return false; }

    LayoutWidgetFactory(Factories *factories,const std::string &factory_name,const std::vector<PropertyMap> &sub_widget_props)
    : WidgetFactoryBase(factories,factory_name),layout(sub_widget_props)
    {
        factory_type=FACTORY_TYPE_LAYOUT_WIDGETS;
    }

    virtual ~LayoutWidgetFactory() { }

    virtual Fl_Widget *create(Widgets *widgets,const std::string &widget_name,int cx,int cy,int cw,int ch,const std::string &clabel);

    void update_layout(const std::vector<PropertyMap> &new_layout) { layout=new_layout; }
};

struct LayoutWidget : public Fl_Group {
    Widgets widgets;

    LayoutWidget(int x,int y,int w,int h,const char *label) : Fl_Group(x,y,w,h,label) { }
    virtual ~LayoutWidget() { }

    void update_layout(LayoutWidgetFactory &factory,std::vector<PropertyMap> &layout);
};

inline Fl_Widget *create_widget(Widgets &widgets,Factories &factories,const std::string &factory,const std::string &name,int x,int y,int w,int h,const std::string &label="") {
    if (name.empty()) return NULL; // invalid name
    if (widgets.get_widget(name)) return NULL; // widget with that name already exists

    FactoryInterface *f=factories.get_factory(factory,name);
    if (!f) return NULL; // unknown factory

    Fl_Widget *o=f->create(&widgets,name,x,y,w,h,label);
    return o;
}

void callback_helper(Fl_Widget *o,void *obj); // redirects FLTK C callback to WidgetInfo::callback

} // namespace fltklayout

