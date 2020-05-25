# fltklayout

# To compile fltklayout on Centos 7:

`sudo yum group install "Development Tools" --setopt=group_package_types=mandatory,default,optional --skip-broken`

`sudo yum install git fltk-devel fltk-fluid fltk-static libXinerama-devel libXcursor-devel libXrender libXcursor libXinerama libXft gnu-free-fonts-common gnu-free-sans-fonts gnu-free-serif-fonts gnu-free-mono-fonts xauth`

`git clone https://github.com/iexg/fltklayout.git`

#// or download and unzip https://github.com/iexg/fltklayout/archive/master.zip into fltklayout/ directory

`cd fltklayout`

`make`

This creates an executable called ***fltklayout_designer***, a library called **libfltklayout.a**, and compiles some example applications ***test*** and ***test2***

# To compile on Windows10 in Mobaxterm local terminal:

`apt-get install fltk-devel libfltk-devel libfltk1.3 gcc-g++ gdb cygwin64-binutils git make` # Note: may take a long time!

`git clone https://github.com/iexg/fltklayout.git`

`cd fltklayout`

`PATH=/usr/x86_64-pc-cygwin/bin:$PATH make`

This creates an executable called ***fltklayout_designer***, a library called **libfltklayout.a**, and compiles some example applications ***test*** and ***test2***
