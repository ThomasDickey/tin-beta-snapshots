Name: tin
Summary: easy-to-use USENET news reader
Version: 2.6.2
Release: 1
License: BSD
Group: Applications/News
Source: ftp://ftp.tin.org/pub/news/clients/tin/v2.6/%{name}-%{version}.tar.bz2
Buildroot: /var/tmp/%{name}-%{version}-%{release}
Packager: Dirk Nimmich <nimmich@muenster.de>

%description
An easy-to-use USENET news reader for the console using NNTP.
It supports threading, scoring, different charsets, and many other
useful things. It has also support for different languages.

%define prefix /usr
%define confdir /etc/tin

%prep
%setup -q
CFLAGS="$RPM_OPT_FLAGS" ./configure --with-install-prefix=$RPM_BUILD_ROOT \
 --prefix=%{prefix} \
 --mandir=%{_mandir} \
 --sysconfdir=%{confdir} \
 --verbose \
 --disable-echo \
 --enable-prototypes \
 --enable-nntp-only \
 --enable-cancel-locks \
 --enable-long-article-numbers \
 --with-pcre

%build
make build

%install
make install
make install_sysdefs

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%dir %attr(755,root,root) %{confdir}
%config(noreplace) %attr(644,root,root) %{confdir}/*
%attr(755,root,root) %{prefix}/bin/*
%{_datadir}/locale/*/LC_MESSAGES/*.mo
%doc %{_mandir}/man1/*
%doc %{_mandir}/man5/*
%doc doc/CHANGES doc/CHANGES.old doc/INSTALL doc/TODO doc/WHATSNEW
%doc doc/auth.txt doc/filtering doc/good-netkeeping-seal doc/iso2asc.txt
%doc doc/keymap.sample doc/mailcap.sample doc/mime.types doc/tin.defaults
%doc doc/pgp.txt doc/reading-mail.txt
%doc doc/tools.txt doc/umlaute.txt doc/umlauts.txt
%doc doc/wildmat.3 doc/plp_snprintf.3
%doc doc/article.txt doc/art_handling.txt doc/internals.txt doc/rcvars.txt
%doc doc/config-anomalies doc/nov_tests doc/DEBUG_REFS doc/ABOUT-NLS
%doc doc/CREDITS
%doc README

%changelog
* Tue Jul 22 2003 Dirk Nimmich <nimmich@muenster.de>
  Specfile created for tin 1.6.0.
