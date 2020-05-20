#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

namespace fltklayout {

// based on erco horizontal resizer bar widget
// http://seriss.com/people/erco/fltk/#ResizerBar
class VerticalResizerBar : public Fl_Box {
    int last_y;
    int min_h;                                                        // min height for widget above us
    void HandleDrag(int diff) {
        Fl_Scroll *grp=(Fl_Scroll*)parent();
        int top=y();
        int bot=y()+h();
        // First pass: find widget directly above us with common edge
        //    Possibly clamp 'diff' if widget would get too small..
        //
        for (int t=0; t<grp->children(); t++) {
            Fl_Widget *w=grp->child(t);
            if (diff<0 && w->y()+w->h() == top) {                           // found widget directly above?
                if (w->h()+diff < min_h) diff=w->h()-min_h;   // clamp
                break;                                                // done with first pass
            } else if (diff>0 && w->y() == bot) {                           // found widget directly below?
                if (w->h()-diff < min_h) diff=w->h()-min_h;   // clamp
                break;                                                // done with first pass
            }
        }
        // Second pass: find widgets below us, move based on clamped diff
        for (int t=0; t<grp->children(); t++) {
            Fl_Widget *w=grp->child(t);
            if (w->y()+w->h() == top)                           // found widget directly above?
                w->resize(w->x(), w->y(), w->w(), w->h()+diff);       // change height
            else if (w->y() == bot)                                    // found widget below us?
                w->resize(w->x(), w->y()+diff, w->w(), w->h()-diff);      // change height
        }
        // Change our position last
        resize(x(),y()+diff,w(),h());
        grp->init_sizes();
        grp->redraw();
    }
public:
    VerticalResizerBar(int X,int Y,int W,int H,const char *l) : Fl_Box(X,Y,W,H,"///////") {
        last_y=0;
        min_h=5;
//        align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
//        labelfont(FL_COURIER);
//        labelsize(H);
        visible_focus(0);
        box(FL_FLAT_BOX);
    }
    void SetMinHeight(int val) { min_h=val; }
    int  GetMinHeight() const { return min_h; }
    int handle(int e) {
        int ret=0;
        int this_y=Fl::event_y_root();
        switch (e) {
            case FL_FOCUS: ret=1; break;
            case FL_ENTER: ret=1; fl_cursor(FL_CURSOR_NS);      break;
            case FL_LEAVE: ret=1; fl_cursor(FL_CURSOR_DEFAULT); break;
            case FL_PUSH:  ret=1; last_y=this_y;              break;
            case FL_DRAG:
                HandleDrag(this_y-last_y);
                last_y=this_y;
                ret=1;
                break;
            default: break;
        }
        return(Fl_Box::handle(e) | ret);
    }
};

class HorizontalResizerBar : public Fl_Box {
    int orig_w;
    int last_x;
    int min_w;                                                        // min height for widget above us
    void HandleDrag(int diff) {
        Fl_Scroll *grp=(Fl_Scroll*)parent();
        int left=x();
        int right=x()+w();
        // First pass: find widget directly above us with common edge
        //    Possibly clamp 'diff' if widget would get too small..
        //
        for (int t=0; t<grp->children(); t++) {
            Fl_Widget *w=grp->child(t);
            if (diff<0 && w->x()+w->w()==left) {                          // found widget directly to left?
                if (w->w()+diff<min_w) diff=w->w()-min_w;   // clamp
                break;                                                // done with first pass
            } else if (diff>0 && w->x()==right) {                          // found widget directly to right?
                if (w->w()-diff<min_w) diff=w->w()-min_w;   // clamp
                break;                                                // done with first pass
            }
        }
        // Second pass: find widgets to right of us, move based on clamped diff
        for (int t=0; t<grp->children(); t++) {
            Fl_Widget *w=grp->child(t);
            if (w->x()+w->w() == left)                          // found widget directly to left?
                w->resize(w->x(), w->y(), w->w()+diff, w->h());       // change width
            else if (w->x() == right)                                   // found widget below us?
                w->resize(w->x()+diff, w->y(), w->w()-diff, w->h());   // change width
        }
        // Change our position last
        resize(x()+diff,y(),w(),h());
        grp->init_sizes();
        grp->redraw();
    }
public:
    HorizontalResizerBar(int X,int Y,int W,int H,const char *l) : Fl_Box(X,Y,W,H,"///////") {
        last_x=0;
        min_w=5;
//        align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
//        labelfont(FL_COURIER);
//        labelsize(H);
        visible_focus(0);
        box(FL_FLAT_BOX);
    }
    void SetMinWidth(int val) { min_w=val; }
    int  GetMinWidth() const { return min_w; }
    int handle(int e) {
        int ret=0;
        int this_x=Fl::event_x_root();
        switch (e) {
            case FL_FOCUS: ret=1; break;
            case FL_ENTER: ret=1; fl_cursor(FL_CURSOR_WE);      break;
            case FL_LEAVE: ret=1; fl_cursor(FL_CURSOR_DEFAULT); break;
            case FL_PUSH:  ret=1; last_x=this_x;              break;
            case FL_DRAG:
                HandleDrag(this_x-last_x);
                last_x=this_x;
                ret=1;
                break;
            default: break;
        }
        return(Fl_Box::handle(e) | ret);
    }
};

} // namespace fltklayout

