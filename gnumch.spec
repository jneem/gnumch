Name:           gnumch
Version:        0.2.0
Release:        1%{?dist}
Summary:        Clone of the "Number Munchers" game

Group:          Amusements/Games
License:        GPL
URL:            http://spuzz.net/projects/gnumch/
Source0:        http://spuzz.net/files/gnumch-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: gettext-devel
BuildRequires: SDL-devel, SDL_image-devel, SDL_gfx-devel
BuildRequires: SDL_mixer-devel,SDL_ttf-devel

%description

Gnumch is a clone of the "Number Munchers" game for those of us that
still experience nostalgia for the good old days of Apple ][e. It is an
excellent educational game that teaches simple mathematical concepts
such as primes, factors, multiples, and simple arithmetic.



%prep
%setup -q

# create .desktop file
cat > %name.desktop <<EOF
[Desktop Entry]
Name=Gnumch
Comment=Educational math game
Exec=/usr/bin/gnumch
Type=Application
Terminal=false
Categories=Application;Game;Education
Encoding=UTF-8
X-Desktop-File-Install-Version=0.9
EOF




%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

mkdir $RPM_BUILD_ROOT%{_datadir}/applications/
cp %{name}.desktop $RPM_BUILD_ROOT%{_datadir}/applications/


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/locale/nb/LC_MESSAGES/gnumch.mo


%changelog
* Wed Aug 16 2006 Andrew Ziem <andrewz springsrescuemissionorg> 0.2.0-1
- 0.2.0
* Wed May 10 2006 Andrew Ziem <andrewz springsrescuemissionorg> 0.1.96-1
- 0.1.96
* Thu May 04 2006 Andrew Ziem <andrewz springsrescuemissionorg> 0.1.95-1
- 0.1.95
* Thu May 04 2006 Andrew Ziem <andrewz springsrescuemissionorg> 0.1.94-1
- initial gnumch.spec
