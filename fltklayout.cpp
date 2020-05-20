#include "fltklayout.h"

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_File_Chooser.H>
//#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Tree.H>
#include <FL/names.h>

#include "table.h"
#include "resizebar.h"

#include <limits>

namespace fltklayout {

std::string escape(const std::string &in) {
    static char hex[]="0123456789ABCDEF";
    std::string out;
    for (unsigned char *c=(unsigned char*)in.c_str();*c;c++) {
        if (*c>=' ' && *c<='~' && *c!='%' && *c!=',' && *c!='=')
            out+=*c;
        else {
            out+='%';
            out+=hex[*c>>4];
            out+=hex[*c&0xf];
        }
    }
    return out;
}
std::string unescape(const std::string &in) {
    static char hex[]="0123456789ABCDEF";
    std::string out;
    for (unsigned char *c=(unsigned char*)in.c_str();*c;c++) {
        if (*c=='%' && c[1] && c[2]) {
            char *h1=strchr(hex,c[1]),*h2=strchr(hex,c[2]);
            if (h1 && h2) {
                out+=(char)(((h1-hex)<<4)+(h2-hex));
                c+=2;
                continue;
            }
        }
        out+=*c;
    }
    return out;
}

void PropertyMap::deserialize(const std::string &props) {
    clear();
    const char *s=props.c_str();
    while (*s>=' ') {
        const char *sep=s;
        while (*sep>' ' && *sep!='=') sep++;
        if (*sep!='=') continue;
        const char *e=sep+1;
        while (*e>=' ' && *e!=',') e++;

        std::string pname(unescape(std::string(s,sep-s))),value(unescape(std::string(sep+1,e-(sep+1))));
        (*this)[pname]=value;

        s=e;
        if (*s==',') s++;
    }
}
std::string PropertyMap::serialize() {
    std::string out;
    for (auto &p : *this) {
        if (!out.empty()) out+=',';
        out+=escape(p.first)+'='+escape(p.second);
    }
    return out;
}

bool WidgetFactoryBase::set_property(Widgets *widgets,Fl_Widget *o,const std::string &key,const std::string &val) {
    const int v=atoi(val.c_str());
    if (key=="parent") {
        Fl_Widget *p=widgets->get_widget(val);
        if (!p || !p->as_group() || p==o) 
            return false;
        p->as_group()->add(o);
        return true;
    }
    if (key=="name")
        return widgets->rename(o,val);
    if (key=="label") { o->copy_label(val.c_str()); return true;}
    if (key=="resizable") { 
        if (!o->parent()) return false;
        if (v) o->parent()->resizable(o);
        return true;
    }
    if (key=="active") { 
        if (v) 
            o->activate();
        else
            o->deactivate();
        return true;
    }
    if (key=="box") { 
        Fl_Boxtype b=Fl_Boxtype(v);
        switch(v) {
            case _FL_SHADOW_BOX: b=Fl_Boxtype(FL_SHADOW_BOX); break;
            case _FL_SHADOW_FRAME: b=Fl_Boxtype(FL_SHADOW_FRAME); break;
            case _FL_ROUNDED_BOX: b=Fl_Boxtype(FL_ROUNDED_BOX); break;
            case _FL_RSHADOW_BOX: b=Fl_Boxtype(FL_RSHADOW_BOX); break;
            case _FL_ROUNDED_FRAME: b=Fl_Boxtype(FL_ROUNDED_FRAME); break;
            case _FL_RFLAT_BOX: b=Fl_Boxtype(FL_RFLAT_BOX); break;
            case _FL_ROUND_UP_BOX: b=Fl_Boxtype(FL_ROUND_UP_BOX); break;
            case _FL_ROUND_DOWN_BOX: b=Fl_Boxtype(FL_ROUND_DOWN_BOX); break;
            case _FL_DIAMOND_UP_BOX: b=Fl_Boxtype(FL_DIAMOND_UP_BOX); break;
            case _FL_DIAMOND_DOWN_BOX: b=Fl_Boxtype(FL_DIAMOND_DOWN_BOX); break;
            case _FL_OVAL_BOX: b=Fl_Boxtype(FL_OVAL_BOX); break;
            case _FL_OSHADOW_BOX: b=Fl_Boxtype(FL_OSHADOW_BOX); break;
            case _FL_OVAL_FRAME: b=Fl_Boxtype(FL_OVAL_FRAME); break;
            case _FL_OFLAT_BOX: b=Fl_Boxtype(FL_OFLAT_BOX); break;
            case _FL_PLASTIC_UP_BOX: b=Fl_Boxtype(FL_PLASTIC_UP_BOX); break;
            case _FL_PLASTIC_DOWN_BOX: b=Fl_Boxtype(FL_PLASTIC_DOWN_BOX); break;
            case _FL_PLASTIC_UP_FRAME: b=Fl_Boxtype(FL_PLASTIC_UP_FRAME); break;
            case _FL_PLASTIC_DOWN_FRAME: b=Fl_Boxtype(FL_PLASTIC_DOWN_FRAME); break;
            case _FL_PLASTIC_THIN_UP_BOX: b=Fl_Boxtype(FL_PLASTIC_THIN_UP_BOX); break;
            case _FL_PLASTIC_THIN_DOWN_BOX: b=Fl_Boxtype(FL_PLASTIC_THIN_DOWN_BOX); break;
            case _FL_PLASTIC_ROUND_UP_BOX: b=Fl_Boxtype(FL_PLASTIC_ROUND_UP_BOX); break;
            case _FL_PLASTIC_ROUND_DOWN_BOX: b=Fl_Boxtype(FL_PLASTIC_ROUND_DOWN_BOX); break;
            case _FL_GTK_UP_BOX: b=Fl_Boxtype(FL_GTK_UP_BOX); break;
            case _FL_GTK_DOWN_BOX: b=Fl_Boxtype(FL_GTK_DOWN_BOX); break;
            case _FL_GTK_UP_FRAME: b=Fl_Boxtype(FL_GTK_UP_FRAME); break;
            case _FL_GTK_DOWN_FRAME: b=Fl_Boxtype(FL_GTK_DOWN_FRAME); break;
            case _FL_GTK_THIN_UP_BOX: b=Fl_Boxtype(FL_GTK_THIN_UP_BOX); break;
            case _FL_GTK_THIN_DOWN_BOX: b=Fl_Boxtype(FL_GTK_THIN_DOWN_BOX); break;
            case _FL_GTK_THIN_UP_FRAME: b=Fl_Boxtype(FL_GTK_THIN_UP_FRAME); break;
            case _FL_GTK_THIN_DOWN_FRAME: b=Fl_Boxtype(FL_GTK_THIN_DOWN_FRAME); break;
            case _FL_GTK_ROUND_UP_BOX: b=Fl_Boxtype(FL_GTK_ROUND_UP_BOX); break;
            case _FL_GTK_ROUND_DOWN_BOX: b=Fl_Boxtype(FL_GTK_ROUND_DOWN_BOX); break;
            case _FL_GLEAM_UP_BOX: b=Fl_Boxtype(FL_GLEAM_UP_BOX); break;
            case _FL_GLEAM_DOWN_BOX: b=Fl_Boxtype(FL_GLEAM_DOWN_BOX); break;
            case _FL_GLEAM_UP_FRAME: b=Fl_Boxtype(FL_GLEAM_UP_FRAME); break;
            case _FL_GLEAM_DOWN_FRAME: b=Fl_Boxtype(FL_GLEAM_DOWN_FRAME); break;
            case _FL_GLEAM_THIN_UP_BOX: b=Fl_Boxtype(FL_GLEAM_THIN_UP_BOX); break;
            case _FL_GLEAM_THIN_DOWN_BOX: b=Fl_Boxtype(FL_GLEAM_THIN_DOWN_BOX); break;
            case _FL_GLEAM_ROUND_UP_BOX: b=Fl_Boxtype(FL_GLEAM_ROUND_UP_BOX); break;
            case _FL_GLEAM_ROUND_DOWN_BOX: b=Fl_Boxtype(FL_GLEAM_ROUND_DOWN_BOX); break;
        }
        o->box(b);
        return true;
    }
    if (key=="color") { o->color(v); return true; }
    if (key=="align") { o->align(v); return true; }
    if (key=="labelfont") { o->labelfont(v); return true; }
    if (key=="labelsize") { o->labelsize(v); return true; }
    if (key=="labeltype") { 
        if (v==_FL_SHADOW_LABEL) o->labeltype(FL_SHADOW_LABEL);
        else if (v==_FL_ENGRAVED_LABEL) o->labeltype(FL_ENGRAVED_LABEL);
        else if (v==_FL_EMBOSSED_LABEL) o->labeltype(FL_EMBOSSED_LABEL);
        else o->labeltype(Fl_Labeltype(v));
        return true;
    }
    if (key=="labelcolor") { o->labelcolor(v); return true; }
    if (key=="selection_color") { o->selection_color(v); return true; }
    if (key=="output") { 
        if (v)
            o->set_output();
        else
            o->clear_output();
        return true;
    }
    if (key=="x") { resize(o,v,o->y(),o->w(),o->h()); return true; }
    if (key=="y") { resize(o,o->x(),v,o->w(),o->h()); return true; }
    if (key=="w") { resize(o,o->x(),o->y(),v,o->h()); return true; }
    if (key=="h") { resize(o,o->x(),o->y(),o->w(),v); return true; }
    return false;
}
std::string WidgetFactoryBase::get_property(Widgets *widgets,Fl_Widget *o,const std::string &key) {
    if (key=="factory") return widgets->get_factory_name(o);
    if (key=="parent") {
        Fl_Widget *p=o->parent();
        while (p && !widgets->is_managed(p)) p=p->parent();
        return p ? widgets->get_name(p) : "";
    }
    if (key=="name") return widgets->get_name(o);
    if (key=="label") return o->label();
    if (key=="resizable") return o->parent() && o->parent()->resizable()==o ? "1" : "0";
    if (key=="active") return o->active() ? "1" : "0";
    if (key=="box") return std::to_string((int)o->box());
    if (key=="color") return std::to_string((int)o->color());
    if (key=="align") return std::to_string((int)o->align());
    if (key=="labelfont") return std::to_string((int)o->labelfont());
    if (key=="labelsize") return std::to_string((int)o->labelsize());
    if (key=="labeltype") return std::to_string((int)o->labeltype());
    if (key=="labelcolor") return std::to_string((int)o->labelcolor());
    if (key=="selection_color") return std::to_string((int)o->selection_color());
    if (key=="output") return o->output() ? "1" : "0";
    if (key=="x") return std::to_string(o->x());
    if (key=="y") return std::to_string(o->y());
    if (key=="w") return std::to_string(o->w());
    if (key=="h") return std::to_string(o->h());
    return "";
}

std::vector<std::string> WidgetFactoryBase::get_property_names() {
    std::vector<std::string> property_names;
    auto property_info=get_property_info();
    for (auto &p : property_info)
        property_names.push_back(p.first);
    return property_names;
}

PropertyMap WidgetFactoryBase::get_property_info() { 
    static PropertyMap property_info{
        { "factory","readonly" },
            { "parent","string" },
            { "name","string" },
            { "label","string" },
            { "resizable","bool" },
            { "active","bool" },
            { "color","color" },
            { "labelcolor","color" },
            { "align","bitmask{TOP=1,BOTTOM=2,LEFT=4,RIGHT=8,INSIDE=16,OVER_IMAGE=32,CLIP=64,WRAP=128,IMAGE_RIGHT=256,BACKDROP=512}" },
            { "labelfont","enum{HELVETICA=0,HELVETICA_BOLD=1,HELVETICA_ITALIC=2,HELVETICA_BOLD_ITALIC=3,COURIER=4,COURIER_BOLD=5,COURIER_ITALIC=6,COURIER_BOLD_ITALIC=7,TIMES=8,TIMES_BOLD=9,TIMES_ITALIC=10,TIMES_BOLD_ITALIC=11,SYMBOL=12,SCREEN=13,SCREEN_BOLD=14,ZAPF_DINGBATS=15}" },
            { "labelsize","int" },
            { "labeltype","enum{NORMAL=0,NO_LABEL=1,SHADOW=2,ENGRAVED=3,EMBOSSED=4}" },
            { "selection_color","color" },
            { "output","bool" },
            { "box","enum{NO,FLAT,UP,DOWN,UP_FRAME,DOWN_FRAME,THIN_UP,THIN_DOWN,THIN_UP_FRAME,THIN_DOWN_FRAME,ENGRAVED,EMBOSSED,ENGRAVED_FRAME,EMBOSSED_FRAME,BORDER,SHADOW,BORDER_FRAME,SHADOW_FRAME,ROUNDED,RSHADOW,ROUNDED_FRAME,RFLAT,ROUND_UP,ROUND_DOWN,DIAMOND_UP,DIAMOND_DOWN,OVAL,OSHADOW,OVAL_FRAME,OFLAT,PLASTIC_UP,PLASTIC_DOWN,PLASTIC_UP_FRAME,PLASTIC_DOWN_FRAME,PLASTIC_THIN_UP,PLASTIC_THIN_DOWN,PLASTIC_ROUND_UP,PLASTIC_ROUND_DOWN,GTK_UP,GTK_DOWN,GTK_UP_FRAME,GTK_DOWN_FRAME,GTK_THIN_UP,GTK_THIN_DOWN,GTK_THIN_UP_FRAME,GTK_THIN_DOWN_FRAME,GTK_ROUND_UP,GTK_ROUND_DOWN,GLEAM_UP,GLEAM_DOWN,GLEAM_UP_FRAME,GLEAM_DOWN_FRAME,GLEAM_THIN_UP,GLEAM_THIN_DOWN,GLEAM_ROUND_UP,GLEAM_ROUND_DOWN}" },
            { "x","int" },
            { "y","int" },
            { "w","int" },
            { "h","int" }
    };
    return property_info;
}

struct PackFactory : public SimpleWidgetFactory<Fl_Pack,false,true> {
    using BASE=SimpleWidgetFactory<Fl_Pack,false,true>;

    PackFactory(Factories *factories,const std::string &factory_name) : BASE(factories,factory_name) { }
    virtual ~PackFactory() { }

    virtual bool set_property(Widgets *widgets,Fl_Widget *o,const std::string &key,const std::string &val) {
        if (key=="type") {
            ((Fl_Pack*)o)->type(atoi(val.c_str()));
            Fl_Group *g=o->as_group();
            g->init_sizes();
            return true;
        }
        return BASE::set_property(widgets,o,key,val);
    }
    virtual std::string get_property(Widgets *widgets,Fl_Widget *o,const std::string &key) {
        if (key=="type") return std::to_string((int)o->type());
        return BASE::get_property(widgets,o,key);
    }
	virtual PropertyMap get_property_info() {
        static PropertyMap pm=combine_maps(BASE::get_property_info(),{ 
            { "type", "enum{Fl_Pack::VERTICAL=0,Fl_Pack::HORIZONTAL=1}" } 
        });
        return pm;
	}

    virtual void resize(Fl_Widget *o,int x,int y,int w,int h) { 
        Fl_Group *g=o->as_group();
        Fl_Widget *r=g->resizable();
        if (!r && g->children()) {
            g->resizable(g->child(g->children()-1));
            o->resize(x,y,w,h);
            g->resizable(NULL);
        } else
            o->resize(x,y,w,h);
        g->init_sizes();
    }
};

Factories::Factories() { init(); }
Factories::~Factories() {
    for (auto &p : factories)
        delete p.second;
}
void Factories::init() {
    for (auto &p : factories)
        delete p.second;
    factories.clear();

    add_factory(new SimpleWidgetFactory<Fl_Group,false,true>(this,"Fl_Group"));
    add_factory(new SimpleWidgetFactory<Fl_Scroll,false,true>(this,"Fl_Scroll"));
    add_factory(new SimpleWidgetFactory<Fl_Box,false,false>(this,"Fl_Box"));
    add_factory(new SimpleWidgetFactory<Fl_Button,true,false>(this,"Fl_Button"));
    add_factory(new SimpleWidgetFactory<Fl_Check_Button,false,false>(this,"Fl_Check_Button"));
    add_factory(new SimpleWidgetFactory<Fl_Input,true,false>(this,"Fl_Input"));
    add_factory(new SimpleWidgetFactory<Fl_Multiline_Input,true,false>(this,"Fl_Multiline_Input"));
    add_factory(new SimpleWidgetFactory<Fl_Output,true,false>(this,"Fl_Output"));
    add_factory(new SimpleWidgetFactory<Fl_Multiline_Output,true,false>(this,"Fl_Multiline_Output"));
    add_factory(new PackFactory(this,"Fl_Pack"));
    add_factory(new SimpleWidgetFactory<Fl_Menu_Bar,true,false>(this,"Fl_MenuBar"));
    add_factory(new SimpleWidgetFactory<StringTable,false,false>(this,"StringTable"));
    add_factory(new SimpleWidgetFactory<VerticalResizerBar,false,false>(this,"VerticalResizerBar"));
    add_factory(new SimpleWidgetFactory<HorizontalResizerBar,false,false>(this,"HorizontalResizerBar"));
}

std::map<std::string,std::vector<PropertyMap> > load_layout_file(const std::string &filename) {
    std::map<std::string,std::vector<PropertyMap> > layouts;

    if (filename.find("\n")!=std::string::npos) {
        // filename is the data
        const char *line=filename.c_str()+6,*end=line+filename.size()-6;
        while (line<end) {
            const char *line_start=line,*line_end=line_start;
            while (line_end<end && *line_end!='\n') line_end++;
            line=line_end+1;
            while (line_start<line_end && (*line_start==' ' || *line_start=='\t')) 
                line_start++;
            while (line_end>line_start && *(line_end-1)<' ') line_end--;
            if (line_start==line_end || *line_start=='#') continue;

            PropertyMap props;
            const char *s=line_start;
            while (s<line_end && *s>=' ') {
                const char *sep=s;
                while (sep<line_end && *sep>' ' && *sep!='=') sep++;
                if (sep==line_end || *sep!='=') continue;
                const char *e=sep+1;
                while (e<line_end && *e>=' ' && *e!=',') e++;

                std::string pname(unescape(std::string(s,sep-s))),value(unescape(std::string(sep+1,e-(sep+1))));
                props[pname]=value;

                s=e;
                if (*s==',') s++;
            }
            props["line"]=std::string(line_start,line_end-line_start);
            layouts[props["layout"]].push_back(props);
        }

    } else {
        // load data from file
        FILE *fd=fopen(filename.c_str(),"r");
        if (!fd) return layouts;
        char line[65536];
        while (fgets(line,sizeof(line),fd)) {
            char *s=line,*e=s+strlen(s);
            while (*s==' ' || *s=='\t') s++;
            if (*s=='#') continue;
            while (e>s && *(e-1)<' ') e--;
            *e=0;

            PropertyMap props;
            while (*s>=' ') {
                char *sep=s;
                while (*sep>' ' && *sep!='=') sep++;
                if (*sep!='=') continue;
                char *e=sep+1;
                while (*e>=' ' && *e!=',') e++;

                std::string pname(unescape(std::string(s,sep-s))),value(unescape(std::string(sep+1,e-(sep+1))));
                props[pname]=value;

                s=e;
                if (*s==',') s++;
            }
            props["line"]=line;
            layouts[props["layout"]].push_back(props);
        }
        fclose(fd);
    }
    return layouts;
}

std::string Widgets::load_layout(Factories &factories,Fl_Group *grp,const std::string &filename,const std::string &layout_name,const std::string &prefix,const bool resize_group,const bool zero_xy,const int at_x,const int at_y) {
    auto layouts=load_layout_file(filename);
    if (layouts.empty())
        return "could not open file for reading";

    auto layout=layouts[layout_name];
    if (layout.empty())
        return "layout not found in file";

    const int imax=std::numeric_limits<int>::max();
    int minx=zero_xy ? imax:0,miny=zero_xy ? imax:0;
    int maxx=0,maxy=0;
    for (auto &props : layout) {
        Fl_Widget *o=get_widget(prefix+props["name"]);
        if (o) return "Widget with name="+props["name"]+" already exists";

        int x=atoi(props["x"].c_str()),y=atoi(props["y"].c_str()),w=atoi(props["w"].c_str()),h=atoi(props["h"].c_str());
        if (x<minx) minx=x;
        if (y<miny) miny=y;
        if (x+w>maxx) maxx=x+w;
        if (y+h>maxy) maxy=y+h;
    }

    if (resize_group) {
        int rightx=at_x+maxx-minx,bottomy=at_y+maxy-miny;
        grp->size(rightx-grp->x(),bottomy-grp->y());
    }

    for (auto &props : layout) {
        auto line=props["line"];

        auto factory=props["factory"];
        if (factory.empty()) return "malformed line (missing factory=): "+line;
        auto name=props["name"];
        FactoryInterface *f=factories.get_factory(factory,name);
        if (!f) return "unknown factory ("+factory+"): "+line;

        int x=atoi(props["x"].c_str()),y=atoi(props["y"].c_str()),w=atoi(props["w"].c_str()),h=atoi(props["h"].c_str());
        auto label=props["label"];

        Fl_Widget *o=f->create(this,prefix+name,at_x+x-minx,at_y+y-miny,w,h,label);
        if (!o) return "factory failed to create widget (factory="+factory+",name="+name+"): "+line;

        auto parent=props["parent"];
        Fl_Group *g=grp;
        if (!parent.empty()) {
            Fl_Widget *p=get_widget(prefix+parent);
            if (!p) return "parent="+parent+" not found: "+line;
            if (!get_factory(p)->is_group()) return "parent="+parent+" is not a group: "+line;
            g=p->as_group();
        }
        g->begin(); g->add(o); g->end();

        const static std::set<std::string> ignore{ "line","layout","factory","name","x","y","w","h","label","parent" };
        for (auto &nv : props) {
            if (!ignore.count(nv.first))
                if (!f->set_property(this,o,nv.first,nv.second))
                    return "failed to set property (factory="+factory+","+nv.first+"="+nv.second+"): "+line;
        }
    }

    grp->init_sizes();
    return ""; // success
}

Fl_Widget *LayoutWidgetFactory::create(Widgets *widgets,const std::string &widget_name,int cx,int cy,int cw,int ch,const std::string &clabel) {
    auto layout_widget=new LayoutWidget(0,0,1,1,"");
    layout_widget->update_layout(*this,layout);

    ((Fl_Widget*)layout_widget)->resize(cx,cy,cw,ch);
    layout_widget->copy_label(clabel.c_str());

    WidgetInfo *info=new WidgetInfo();
    info->init(widgets,widget_name,this,layout_widget);
    widgets->add_widget(info);

    layout_widget->callback(callback_helper,widgets);
    return layout_widget;
}

void LayoutWidget::update_layout(LayoutWidgetFactory &factory,std::vector<PropertyMap> &layout) {

    // destroy any previous widgets
    clear();

    // work out layout dimensions
    int minx,miny;
    int num_top_level;
    const int imax=std::numeric_limits<int>::max();
    minx=imax,miny=imax;
    num_top_level=0;
    int maxx=0,maxy=0;
    for (auto &props : layout) {
        int x=atoi(props["x"].c_str()),y=atoi(props["y"].c_str()),w=atoi(props["w"].c_str()),h=atoi(props["h"].c_str());
        if (x<minx) minx=x;
        if (x+w>maxx) maxx=x+w;
        if (y<miny) miny=y;
        if (y+h>maxy) maxy=y+h;

        if (props["parent"].empty())
            num_top_level++;
    }
    int lw=maxx>minx ? maxx-minx : 0;
    int lh=maxy>miny ? maxy-miny : 0;

    int cw=w(),ch=h();
    size(lw,lh);

    begin();
    for (auto &props : layout) {
        auto factory_name=props["factory"];
        auto name=props["name"];
        FactoryInterface *f=factory.factories->get_factory(factory_name,name);
        if (!f) continue; // unknown factory

        int ox=atoi(props["x"].c_str()),oy=atoi(props["y"].c_str()),ow=atoi(props["w"].c_str()),oh=atoi(props["h"].c_str());
        auto label=props["label"];

        Fl_Widget *o=f->create(&widgets,name,x()+ox-minx,y()+oy-miny,ow,oh,label);
        if (!o) continue; // create failed

        auto parent=props["parent"];
        Fl_Group *g=as_group();
        if (!parent.empty()) {
            Fl_Widget *p=widgets.get_widget(parent);
            if (!p || !widgets.get_factory(p)->is_group()) continue; // cant find parent widget
            g=p->as_group();
        }
        g->begin(); g->add(o); g->end();

        const static std::set<std::string> ignore{ "line","layout","factory","name","x","y","w","h","label","parent" };
        for (auto &nv : props) {
            if (!ignore.count(nv.first))
                f->set_property(&widgets,o,nv.first,nv.second); // note: ignoring return code
        }
    }

    end();
    size(cw,ch);
}

std::string Factories::load_layouts_as_widgets(const std::string &filename) {
    auto layouts=load_layout_file(filename);
    if (layouts.empty())
        return "could not open file "+filename;

    for (auto &p : layouts) {
        auto &layout_name=p.first;
        auto &layout=p.second;

        if (!layout.empty())
			add_factory(new LayoutWidgetFactory(this,layout_name,layout));
    }
    return ""; // success
}

std::string Factories::add_layout_widget_factory(const std::string &factory_name,const std::vector<PropertyMap> &layout) {
    add_factory(new LayoutWidgetFactory(this,factory_name,layout));
    return ""; // success
}

void callback_helper(Fl_Widget *o,void *obj) {
    Widgets *widgets=(Widgets*)obj;
    auto info=widgets->get_info(o);
    if (info && info->callback)
        info->callback(o,info);
}

} // namespace fltklayout
