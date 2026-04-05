Name:           netrc
Version:        26.04
Release:        1%{?dist}
Summary:        NetRC Qt application

URL:            https://github.com/AceOfSnakes/NetRC

BuildArch:      x86_64
License:        GPLv3

%description
NetRC also known as Network remote control. The base idea of this project is to provide access to control your devices via local network. All devices should be configurable without changes in source code. All you need is to create device configuration file. Load it into application and control your device via network. Based on ideas and code from this project https://sourceforge.net/projects/avrpioremote/

%define _rpmfilename %%{NAME}_%%{VERSION}_%%{ARCH}.rpm

%clean
# %{__rm} -rf $RPM_BUILD_ROOT
# %{__rm} -rf $RPM_BUILD_DIR/*

%prep
# Do NOT assume any directory; extract tarball manually
#%setup -q -T
#tar -xzf %{SOURCE0} -C .
echo =============================================
#%{__rm} -rf %{name}-%{version}
#%{__mkdir} -p %{name}-%{version}

%build
# Nothing to build; we are packaging a prebuilt binary
echo ======== $(pwd)
echo PWD: $(pwd)
cd rpm/SOURCES
qmake6 NetRC.pro
%{__make}

mkdir -p %{buildroot}

#cp -a settings/* %{buildroot}/
#cp -a NetRC %{buildroot}/

%install
#rm -rf %{buildroot}
# Copy all files from tarball into build root
#cp -a * %{buildroot}/

# mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}/opt/NetRC
mkdir -p %{buildroot}/usr/share/applications/

# make install INSTALL_ROOT=%{buildroot}
# install -m 755 NetRC %{buildroot}%{_bindir}/NetRC

mkdir -p %{buildroot}
echo =========== Install Filder %{__pwd}

cp -r rpm/SOURCES/settings %{buildroot}/opt/NetRC/settings/
cp -r rpm/SOURCES/style %{buildroot}/opt/NetRC/style/
cp -a rpm/SOURCES/NetRC %{buildroot}/opt/NetRC
cp -a rpm/SOURCES/src/images/NetRC.png %{buildroot}/opt/NetRC/NetRC.png
cp -a rpm/SOURCES/NetRC.desktop %{buildroot}/usr/share/applications/NetRC.desktop

%files
/opt/NetRC/NetRC
/opt/NetRC/NetRC.png
/opt/NetRC/settings
/opt/NetRC/style
/usr/share/applications/NetRC.desktop


%changelog
* Wed Apr 01 2026 Ace Of Snakes <AceOfSnakes@gmail.com> - 26.04-1
- Add wheel button

* Sun Mar 01 2026 Ace Of Snakes <AceOfSnakes@gmail.com> - 26.03-1
- Initial RPM release
