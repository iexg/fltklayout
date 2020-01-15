#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <FL/Fl_Table.H>

namespace fltklayout {

struct StringTable : public Fl_Table {

   struct Header {
       StringTable &table;
       std::string name,label;
       Fl_Align column_align;

       Header(StringTable &table,const bool is_column,const std::string &name,const std::string &label)
       : table(table),name(name),label(label),column_align(FL_ALIGN_CENTER)
       { }
       virtual ~Header() { }

       virtual void set_label(const std::string &_label) { label=_label; }
       virtual void draw(int X,int Y,int W,int H) {
           fl_push_clip(X,Y,W,H);
           fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,table.row_header_color());
           fl_color(FL_BLACK);
           fl_draw(label.c_str(),X,Y,W,H,column_align);
           fl_pop_clip();
       }
   };

   virtual Header *header_factory(const bool is_column,const std::string &name,const std::string &label) {
       return new Header(*this,is_column,name,label);
   }
   virtual void delete_header(Header *h) { delete h; }

   struct Headers {
       std::vector<Header*> headers;
       std::map<std::string,int> name2idx;
       std::vector<int> view;

       int get_idx(const std::string &name) {
           auto i=name2idx.find(name);
           return i==name2idx.end() ? -1 : i->second;
       }
       Header *get_header(const int N) { 
           return N>=0 && N<(int)headers.size() ? headers[N] : NULL;
       }
       int view2idx(const int view_idx) {
           return view_idx>=0 && view_idx<(int)view.size() ? view[view_idx] : -1;
       }
       void set_view(const std::vector<int> &new_view) { view=new_view; }
   };
   Headers column_headers,row_headers;

   void set_column_view(const std::vector<int> &new_view) {
       column_headers.set_view(new_view);
       set_cols();
   }
   void set_row_view(const std::vector<int> &new_view) {
       row_headers.set_view(new_view);
       set_rows();
   }

   void set_cols() { cols(column_headers.view.size()); }
   void set_rows() {
       int old=rows();
       rows(row_headers.view.size());
       if (!old) row_height_all(default_row_height);
   }

   struct Cell {
       Cell *selected_prev=NULL,*selected_next=NULL;
       int is_selected=0x00;

       Cell() { }
       virtual ~Cell() { }

       virtual std::string text()=0;
       virtual void text(const std::string &new_text)=0;
       virtual void draw(StringTable &table,int ROW,int COL,int R,int C,int X,int Y,int W,int H)=0;
    };
    struct TextCell : public Cell {
        std::string _text;
        Header &row_header,&column_header;

        TextCell(StringTable &table,Header &row_header,Header &column_header)
        : row_header(row_header),column_header(column_header)
        { }
        virtual ~TextCell() { }

        virtual std::string text() { return _text; }
        virtual void text(const std::string &new_text) { _text=new_text; }
        virtual void draw(StringTable &table,int ROW,int COL,int R,int C,int X,int Y,int W,int H) {
            fl_push_clip(X,Y,W,H);
            fl_color(is_selected ? FL_CYAN : FL_WHITE); fl_rectf(X,Y,W,H); 
            fl_color(FL_GRAY0); fl_draw(_text.c_str(),X,Y,W,H,column_header.column_align);
            fl_color(table.color()); fl_rect(X,Y,W,H);
            fl_pop_clip();
        }
    };
    virtual Cell *cell_factory(Header &row_header,Header &column_header) {
        return new TextCell(*this,row_header,column_header);
    }
    virtual void delete_cell(Cell *c) {
//        deselect(c);
        delete c;
    }
    
    std::vector<std::vector<Cell*>> data;
    void reserve(const int rows,const int cols) {
        data.reserve(rows);
        if (!data.empty() && cols>(int)data[0].size()) {
            for (auto row : data)
                row.resize(cols);
        }
        row_headers.headers.reserve(rows);
        column_headers.headers.reserve(cols);
    }
    Cell *cell(const int ROW,const int COL) { return data[ROW][COL]; }
    Cell *cell(const std::string &row_name,const std::string &column_name) {
        const int ROW=row_headers.get_idx(row_name);
        if (ROW>=0) {
            const int COL=column_headers.get_idx(column_name);
            if (COL>=0) return data[ROW][COL];
        }
        return NULL;
    }

    void damageCell(const int ROW,const int COL) { redraw_range(ROW,ROW,COL,COL); }
    void damageCell(const std::string &row_name,const std::string &column_name) {
        const int ROW=row_headers.get_idx(row_name);
        if (ROW>=0) {
            const int COL=column_headers.get_idx(column_name);
            if (COL>=0) damageCell(ROW,COL);
        }
    }
    void damageCells(const int ROW1,const int COL1,const int ROW2,const int COL2) {
        damage_zone(ROW1,COL1,ROW2,COL2,ROW2,COL2);
    }

    void setText(const int ROW,const int COL,const std::string &new_text) {
        Cell *c=cell(ROW,COL);
        if (c) {
            c->text(new_text);
            damageCell(ROW,COL);
        }
    }
    void setText(const std::string &row_name,const std::string &column_name,const std::string &new_text) {
        const int ROW=row_headers.get_idx(row_name);
        if (ROW>=0) {
            const int COL=column_headers.get_idx(column_name);
            if (COL>=0) setText(ROW,COL,new_text);
        }
    }

    void clear() {
        clear_rows();
        for (auto h : column_headers.headers) 
            delete_header(h);
        column_headers.headers.clear();
        column_headers.name2idx.clear();
        column_headers.view.clear();
        set_cols();
    }

    void clear_rows() {
//        clear_selection();
        for (int r=0;r<(int)row_headers.headers.size();r++) {
            Header &row_header=*row_headers.headers[r];
            for (int c=0;c<(int)column_headers.headers.size();c++)
                delete_cell(cell(r,c));
            delete_header(&row_header);
        }
        data.clear();
        row_headers.headers.clear();
        row_headers.name2idx.clear();
        row_headers.view.clear();
        set_rows();
    }
    
    int add_column(const std::string &name,const std::string &label,const bool _set_cols=true) {
        int COL=column_headers.get_idx(name);
        if (COL>=0) 
            column_headers.headers[COL]->set_label(label);
        else {
            COL=column_headers.headers.size();
            Header *new_header=header_factory(true,name,label);
            column_headers.headers.push_back(new_header);
            column_headers.view.push_back(COL);
            column_headers.name2idx[name]=COL;
            for (int r=0;r<(int)data.size();r++) {
                data[r].resize(COL+1);
                data[r][COL]=cell_factory(*row_headers.headers[r],*new_header);
            }
        }
        if (_set_cols) set_cols();
        return COL;
    }
    bool remove_column(const std::string &name,const bool _set_cols=true) {
        int COL=column_headers.get_idx(name);
        if (COL<0) return false;

        auto &cv=column_headers.view;
        cv.erase(std::remove(cv.begin(),cv.end(),COL),cv.end());
        for (auto& i : cv) {
            if (i>=COL) --i;
        }
        for (auto i : column_headers.name2idx) {
            if (i.second>=COL) --i.second;
        }

        for (int r=0;r<(int)data.size();r++)
            data[r].erase(data[r].begin()+COL);

        delete_header(column_headers.headers[COL]);
        column_headers.headers.erase(column_headers.headers.begin()+COL);

        if (_set_cols) set_cols();
        return true;
    }
    
    int add_row(const std::string &name,const std::string &label,const bool _set_rows=true) {
        int ROW=row_headers.get_idx(name);
        if (ROW>=0) 
            row_headers.headers[ROW]->set_label(label);
        else {
            ROW=row_headers.headers.size();
            Header *new_header=header_factory(false,name,label);
            row_headers.headers.push_back(new_header);
            row_headers.view.push_back(ROW);
            row_headers.name2idx[name]=ROW;
            data.resize(ROW+1);
            data[ROW].resize(column_headers.headers.size());
            for (int c=0;c<(int)column_headers.headers.size();c++)
                data[ROW][c]=cell_factory(*new_header,*column_headers.headers[c]);
        }
        if (_set_rows) set_rows();
        return ROW;
    }
    bool remove_row(const std::string &name,const bool _set_rows=true) {
        int ROW=row_headers.get_idx(name);
        if (ROW<0) return false;

        auto &rv=row_headers.view;
        rv.erase(std::remove(rv.begin(),rv.end(),ROW),rv.end());
        for (auto& i : rv) {
            if (i>=ROW) --i;
        }
        for (auto i : row_headers.name2idx) {
            if (i.second>=ROW) --i.second;
        }

        for (int r=0;r<(int)data.size();r++)
            data[r].erase(data[r].begin()+ROW);

        delete_header(row_headers.headers[ROW]);
        row_headers.headers.erase(row_headers.headers.begin()+ROW);

        if (_set_rows) set_rows();
        return true;
    }

    virtual void drawColumnHeader(Header &header,int X,int Y,int W,int H) { header.draw(X,Y,W,H); }
    virtual void drawRowHeader(Header &header,int X,int Y,int W,int H) { header.draw(X,Y,W,H); }
    virtual void drawCell(Cell &c,int ROW,int COL,int R,int C,int X,int Y,int W,int H) { c.draw(*this,ROW,COL,R,C,X,Y,W,H); }

    void draw_cell(TableContext context,int R=0,int C=0,int X=0,int Y=0,int W=0,int H=0) {
        int ROW=row_headers.view2idx(R);
        int COL=column_headers.view2idx(C);
        if (ROW<0 || COL<0) return;

        switch(context) {
            case CONTEXT_STARTPAGE: fl_font(default_textfont,default_textsize); break;
            case CONTEXT_COL_HEADER: drawColumnHeader(*column_headers.headers[COL],X,Y,W,H); break;
            case CONTEXT_ROW_HEADER: drawRowHeader(*row_headers.headers[ROW],X,Y,W,H); break;
            case CONTEXT_CELL: drawCell(*data[ROW][COL],ROW,COL,R,C,X,Y,W,H); break;
            case CONTEXT_RC_RESIZE: break;
            default: break;
        }
    }

    int default_row_height=14;
    Fl_Font default_textfont=FL_HELVETICA;
    int default_textsize=12;

    StringTable(int X,int Y,int W,int H,const char *L=0) : Fl_Table(X,Y,W,H) { 
        rows(0);
        row_header(1);
        row_height_all(default_row_height);
        row_resize(1);
        row_header_width(80);

        cols(0);
        col_header(1);
        col_width_all(80);
        col_resize(1);

        end();

        add_column("col1","col1");
        add_column("col2","col2");
        add_column("col3","col3");
        add_column("col4","col4");

        set_column_view({3,2,1,0});
        remove_column("col3");

        add_row("row1","row1");
        add_row("row2","row2");
        add_row("row3","row3");
        add_row("row4","row4");

        set_row_view({3,2,1,0});
        remove_row("row3");
    }

    virtual ~StringTable() { clear(); }

    virtual int handle(int e) {
// TODO selection

        return Fl_Table::handle(e);
    }
};

} // namespace fltklayout

