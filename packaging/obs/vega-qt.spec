Name:           vega-qt
Version:        0
Release:        1%{?dist}
Summary:        Centro de controle Vega para KDE Plasma
License:        GPL-3.0-only
URL:            https://github.com/lyra-os-linux/vega-qt
Source0:        vega-qt-src-%{version}.tar

BuildRequires:  cmake >= 3.24
BuildRequires:  ninja
BuildRequires:  gcc-c++
BuildRequires:  qt6-core-devel >= 6.6
BuildRequires:  qt6-qml-devel >= 6.6
BuildRequires:  qt6-dbus-devel >= 6.6
BuildRequires:  qt6-network-devel >= 6.6
BuildRequires:  kf6-kirigami-devel
Requires:       vegad
Requires:       secret-tool
Requires:       qt6-declarative-imports
Requires:       kf6-kirigami-imports
Recommends:     restic
Recommends:     systemsettings6

%description
Interface Qt 6 e Kirigami do centro de controle Vega para KDE Plasma.
Conecta-se ao serviço de sistema vegad e pode coexistir com as interfaces
vega-gtk e vega-xfce.

%prep
%setup -q -n vega-qt-src-%{version}

%build
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=%{_prefix} \
  -DCMAKE_INSTALL_BINDIR=bin \
  -DCMAKE_INSTALL_DATADIR=share
cmake --build build --parallel %{_smp_build_ncpus}

%install
DESTDIR=%{buildroot} cmake --install build

%files
%license LICENSE
%doc README.md
%{_bindir}/vega-qt
%{_datadir}/applications/org.lyraos.Vega.Qt.desktop
%{_datadir}/icons/hicolor/scalable/apps/vega.svg

%changelog
