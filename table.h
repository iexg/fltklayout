#pragma once

#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <FL/Fl_Table.H>
#include <FL/fl_draw.H>

namespace fltklayout {

struct StringTable : public Fl_Table {

   struct Header {
       StringTable &table;
       std::string name,label;
       Fl_Align column_header_align;
       Fl_Align column_data_align;

       Header(StringTable &table,const bool is_column,const std::string &name,const std::string &label)
       : table(table),name(name),label(label),column_header_align(FL_ALIGN_CENTER),column_data_align(FL_ALIGN_CENTER)
       { }
       virtual ~Header() { }

       virtual void set_label(const std::string &_label) { label=_label; }
       virtual void draw(int X,int Y,int W,int H) {
           fl_push_clip(X,Y,W,H);
           fl_draw_box(FL_THIN_UP_BOX,X,Y,W,H,table.row_header_color());
           fl_color(FL_BLACK);
           fl_draw(label.c_str(),X,Y,W,H,column_header_align);
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
       std::vector<int> view,reverse_view;

       int get_idx(const std::string &name) { // idx in data[]
           auto i=name2idx.find(name);
           return i==name2idx.end() ? -1 : i->second;
       }
       int get_view(const std::string &name) { // idx on screen
           int idx=get_idx(name);
           return idx>=0 ? idx2view(idx) : -1;
       }
       Header *get_header(const int N) { 
           return N>=0 && N<(int)headers.size() ? headers[N] : NULL;
       }
       int view2idx(const int I) { // convert screen idx to data[] idx
           return I>=0 && I<(int)view.size() ? view[I] : -1;
       }
       int idx2view(const int DI) { // convert data[] idx to screen idx
           return DI>=0 && DI<(int)reverse_view.size() ? reverse_view[DI] : -1;
       }
       void set_reverse_view() {
           reverse_view.resize(headers.size());
           std::fill(reverse_view.begin(),reverse_view.end(),-1);
           for (int i=0;i<(int)view.size();i++)
               reverse_view[view[i]]=i;
       }
       void set_view(const std::vector<int> &new_view) { 
           view=new_view; 
           set_reverse_view();
       }
       void view_remove(const int DI) {
		   int VI=reverse_view[DI];
		   if (VI>=0) {
			   view.erase(view.begin()+VI);
			   for (auto &i : view) {
				   if (i>=DI) i--;
			   }
			   reverse_view.erase(reverse_view.begin()+DI);
			   for (int c=DI;c<(int)reverse_view.size();c++) {
				   if (reverse_view[c]>=VI) reverse_view[c]--;
			   }
		   }
       }
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
       Cell() { }
       virtual ~Cell() { }

       virtual std::string text()=0;
       virtual void text(const std::string &new_text)=0;
       virtual void draw(StringTable &table,int DR,int DC,int R,int C,int X,int Y,int W,int H)=0;
    };
    struct TextCell : public Cell {
        std::string _text;
        Header &row_header,&column_header;

        TextCell(StringTable &table,Header &row_header,Header &column_header)
        : row_header(row_header),column_header(column_header)
        { }
        virtual ~TextCell() { }

        virtual std::string text() override { return _text; }
        virtual void text(const std::string &new_text) override { _text=new_text; }
        virtual void draw(StringTable &table,int DR,int DC,int R,int C,int X,int Y,int W,int H) override {
            fl_push_clip(X,Y,W,H);
            int R1,C1,R2,C2;
            table.get_selection(R1,C1,R2,C2);
            Fl_Color bg= (R>=R1 && C>=C1 && R<=R2 && C<=C2) ? FL_CYAN : FL_WHITE;
            fl_color(bg); fl_rectf(X,Y,W,H); 
            fl_color(FL_GRAY0); fl_draw(_text.c_str(),X,Y,W,H,column_header.column_data_align);
            fl_color(table.color()); fl_rect(X,Y,W,H);
            fl_pop_clip();
        }
    };
    virtual Cell *cell_factory(Header &row_header,Header &column_header) {
        return new TextCell(*this,row_header,column_header);
    }
    virtual void delete_cell(Cell *c) { delete c; }
    std::deque<std::vector<Cell*>> data;

    Cell *cell(const int DR,const int DC) { return data[DR][DC]; }
    Cell *cell(const std::string &row_name,const std::string &column_name) {
        const int DR=row_headers.get_idx(row_name);
        if (DR>=0) {
            const int DC=column_headers.get_idx(column_name);
            if (DC>=0) return data[DR][DC];
        }
        return NULL;
    }

    void damageCell(const int DR,const int DC) { 
        int R=row_headers.idx2view(DR),C=column_headers.idx2view(DC);
        if (R>=0 && C>=0) redraw_range(R,R,C,C);
    }
    void damageCell(const std::string &row_name,const std::string &column_name) {
        const int R=row_headers.get_view(row_name);
        if (R>=0) {
            const int C=column_headers.get_view(column_name);
            if (C>=0) damageCell(R,C);
        }
    }
    void damageCells(const int R1,const int C1,const int R2,const int C2) {
        damage_zone(R1,C1,R2,C2,R2,C2);
    }

    void setText(const int DR,const int DC,const std::string &new_text) {
        Cell *c=cell(DR,DC);
        if (c) {
            c->text(new_text);
            damageCell(DR,DC);
        }
    }
    void setText(const std::string &row_name,const std::string &column_name,const std::string &new_text) {
        const int DR=row_headers.get_idx(row_name);
        if (DR>=0) {
            const int DC=column_headers.get_idx(column_name);
            if (DC>=0) setText(DR,DC,new_text);
        }
    }

    void clear() {
        clear_rows();
        for (auto h : column_headers.headers) 
            delete_header(h);
        column_headers.headers.clear();
        column_headers.name2idx.clear();
        column_headers.view.clear();
        column_headers.reverse_view.clear();
        set_cols();
    }

    void clear_rows() {
        set_selection(-1,-1,-1,-1);
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
        row_headers.reverse_view.clear();
        set_rows();
    }
    
    int add_column(const std::string &name,const std::string &label,const int width=0,const Fl_Align data_align=FL_ALIGN_CENTER,const bool _set_cols=true) {
        int DC=column_headers.get_idx(name);
        if (DC>=0) 
            column_headers.headers[DC]->set_label(label);
        else {
            DC=column_headers.headers.size();
            Header *new_header=header_factory(true,name,label);
            column_headers.headers.push_back(new_header);
            column_headers.view.push_back(DC);
            column_headers.reverse_view.push_back(column_headers.view.size()-1);
            column_headers.name2idx[name]=DC;
            new_header->column_data_align=data_align;
            for (int r=0;r<(int)row_headers.headers.size();r++) {
                data[r].resize(DC+1);
                data[r][DC]=cell_factory(*row_headers.headers[r],*new_header);
            }
        }
        if (width) {
            int C=column_headers.idx2view(DC);
            if (C>=0) col_width(C,width);
        }
        if (_set_cols) set_cols();
        return DC;
    }
    bool remove_column(const std::string &name,const bool _set_cols=true) {
        int DC=column_headers.get_idx(name);
        if (DC<0) return false;
         
        column_headers.view_remove(DC);
        set_selection(-1,-1,-1,-1);
        for (int r=0;r<(int)row_headers.headers.size();r++) {
            delete_cell(data[r][DC]);
            data[r].erase(data[r].begin()+DC);
        }
        for (auto &i : column_headers.name2idx) {
            if (i.second>=DC) --i.second;
        }

        Header *h=column_headers.headers[DC];
        column_headers.headers.erase(column_headers.headers.begin()+DC);
        delete_header(h);

        if (_set_cols) set_cols();
        return true;
    }
    
    int add_row(const std::string &name,const std::string &label,const bool _set_rows=true) {
        int DR=row_headers.get_idx(name);
        if (DR>=0) 
            row_headers.headers[DR]->set_label(label);
        else {
            DR=row_headers.headers.size();
            Header *new_header=header_factory(false,name,label);
            row_headers.headers.push_back(new_header);
            row_headers.view.push_back(DR);
            row_headers.reverse_view.push_back(row_headers.view.size()-1);
            row_headers.name2idx[name]=DR;
            data.resize(DR+1);
            data[DR].resize(column_headers.headers.size());
            for (int c=0;c<(int)column_headers.headers.size();c++)
                data[DR][c]=cell_factory(*new_header,*column_headers.headers[c]);
        }
        if (_set_rows) set_rows();
        return DR;
    }
    bool remove_row(const std::string &name,const bool _set_rows=true) {
        int DR=row_headers.get_idx(name);
        if (DR<0) return false;

        row_headers.view_remove(DR);
        set_selection(-1,-1,-1,-1);
        for (int c=0;c<(int)data[DR].size();c++)
            delete_cell(data[DR][c]);
        data.erase(data.begin()+DR);
        for (auto &i : row_headers.name2idx) {
            if (i.second>=DR) --i.second;
        }

        Header *h=row_headers.headers[DR];
        row_headers.headers.erase(row_headers.headers.begin()+DR);
        delete_header(h);

        if (_set_rows) set_rows();
        return true;
    }

    virtual void draw_column_header(Header &header,int X,int Y,int W,int H) { header.draw(X,Y,W,H); }
    virtual void draw_row_header(Header &header,int X,int Y,int W,int H) { header.draw(X,Y,W,H); }
    virtual void draw_cell(int DR,int DC,int R,int C,int X,int Y,int W,int H) { cell(DR,DC)->draw(*this,DR,DC,R,C,X,Y,W,H); }

    void draw_cell(TableContext context,int R=0,int C=0,int X=0,int Y=0,int W=0,int H=0) {
        int DR=row_headers.view2idx(R);
        int DC=column_headers.view2idx(C);
        if (DR<0 || DC<0) return;

        switch(context) {
            case CONTEXT_STARTPAGE: fl_font(default_textfont,default_textsize); break;
            case CONTEXT_COL_HEADER: draw_column_header(*column_headers.headers[DC],X,Y,W,H); break;
            case CONTEXT_ROW_HEADER: draw_row_header(*row_headers.headers[DR],X,Y,W,H); break;
            case CONTEXT_CELL: draw_cell(DR,DC,R,C,X,Y,W,H); break;
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
};

} // namespace fltklayout

