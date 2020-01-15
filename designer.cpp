#include "designer.h"

int main(int argc,char **argv) {
    int x,y,w,h;

    Fl::screen_xywh(x,y,w,h);
    fltklayout::DesignWindow win(x+w/4,y+h/4,w/2,h/2,"fltklayout designer");
    win.show();

    if (argc==2) {
        win.action_start(fltklayout::DesignWindow::ACTION_FILE_LOAD_LAYOUTS);
        auto err=win.load(argv[1]);
        if (!err.empty()) {
            fprintf(stderr,"ERROR: %s\n",err.c_str());
            exit(1);
        }
        win.action_end();
    }

    Fl::run();
}

