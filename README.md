# fltklayout

To compile fltklayout on Centos 7:

`sudo yum group install "Development Tools" --setopt=group_package_types=mandatory,default,optional --skip-broken`

`sudo yum install fltk-devel fltk-fluid fltk-static libXinerama-devel libXcursor-devel libXrender libXcursor libXinerama libXft gnu-free-fonts-common gnu-free-sans-fonts gnu-free-serif-fonts gnu-free-mono-fonts xauth`

`git clone https://github.com/iexg/fltklayout.git`

#// or download and unzip https://github.com/iexg/fltklayout/archive/master.zip into fltklayout/ directory

`cd fltklayout`

`./mak`

This creates an executable called ***fltklayout_designer***, a library called **libfltklayout.a**, and compiles an example application **test.cpp** to ***a.out***


