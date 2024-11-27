Name:       ohm-plugin-ruleengine

Summary:    A prolog-based OHM rule engine plugin
Version:    1.1.13
Release:    1
License:    LGPLv2+
URL:        https://github.com/sailfishos/ohm-rule-engine
Source0:    %{name}-%{version}.tar.gz
Requires:   swi-prolog-library-core >= 7.0
Requires:   ohm
Requires:   prolog-resourcepolicy-extensions
BuildRequires:  automake
BuildRequires:  make
BuildRequires:  pkgconfig(libprolog) >= 1.2.0
BuildRequires:  pkgconfig(libohmplugin) >= 1.2.0
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(swipl) >= 7.0

%description
A prolog-based OHM rule engine plugin.


%package -n prolog-resourcepolicy-extensions
Summary:    Prolog extensions for the resource policy framework
Requires:   %{name} = %{version}-%{release}

%description -n prolog-resourcepolicy-extensions
A set of prolog extensions needed by the resource policy framework
(and/or its prototype implementation).



%prep
%autosetup -n %{name}-%{version}


%build
echo -n "%{version}" > .tarball-version
%autogen --disable-static
%configure --disable-static \
    --enable-extra-warnings

%make_build


%install
%make_install

%files
%license COPYING
%{_libdir}/ohm/libohm_rule_engine.so
%config %{_sysconfdir}/ohm/plugins.d/rule_engine.ini

%files -n prolog-resourcepolicy-extensions
%dir %{_libdir}/prolog
%dir %{_libdir}/prolog/extensions
%{_libdir}/prolog/extensions/*
