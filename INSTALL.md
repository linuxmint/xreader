# Building from source
### Option 1 (The easy way - Debian/Ubuntu only):
```
sudo apt install devscripts equivs
git clone https://github.com/linuxmint/xreader.git
cd xreader

# install dependencies (you need to confirm the installation)
sudo mk-build-deps -ir

# build
dpkg-buildpackage

# install
sudo dpkg -i ../*.deb
```
### Option 2 (Everywhere else):
#### The following options can be enabled during build:
- GConf Support
- GTK+ Unix Print
- Keyring Support
 - DBUS Support
 - SM client support
 - Thumbnailer
 - Previewer
 - Gtk-Doc Support
 - Debug mode
 - GObject Introspection

##### Install dependencies (these are subject to change - check the first section of`debian/control` if you seem to be missing anything):
```
apt install git dpkg-dev
apt install gobject-introspection libdjvulibre-dev libgail-3-dev          \
            libgirepository1.0-dev libgtk-3-dev libgxps-dev               \
            libkpathsea-dev libpoppler-glib-dev libsecret-1-dev           \
            libspectre-dev libtiff-dev libwebkit2gtk-4.0-dev libxapp-dev  \
            mate-common meson xsltproc yelp-tools
```
##### Download the source-code to your machine:
```
git clone https://github.com/linuxmint/xreader.git
cd xreader
```
##### Configure the build with Meson:
```
# The following configuration installs all binaries,
# libraries, and shared files into /usr/local, and
# enables all available options:

meson buildir \
  --prefix=/usr/local \
  --buildtype=plain \
  -D deprecated_warnings=false \
  -D djvu=true \
  -D dvi=true \
  -D t1lib=true \
  -D pixbuf=true \
  -D comics=true \
  -D introspection=true
```
##### Build and install (sudo or root is needed for install):
```
ninja -C builddir
sudo ninja -C builddir install
```
##### Run:
```
/usr/local/bin/xreader

# If you want to test the daemon
usr/local/lib/x86_64-linux-gnu/xreaderd

You can enable debugging with the G_MESSAGES_DEBUG environmental     \
variable.
```
##### Uninstall:
```
ninja -C debian/build uninstall
```
##### Tests:
- Dependencies:
    - python-dogtail [ https://fedorahosted.org/dogtail/ ]
    - python-pyatspi2 [ http://download.gnome.org/sources/pyatspi/ ]
```
ninja test -C debian/build/
```
