#Module-Specific definitions
%define mod_name mod_diagnostics
%define mod_conf 94_%{mod_name}.conf
%define mod_so %{mod_name}.so

Summary:	DSO module for the apache web server
Name:		apache-%{mod_name}
Version:	0.1
Release:	19
Group:		System/Servers
License:	Apache License
URL:		http://apache.webthing.com/mod_diagnostics/
Source0:	http://apache.webthing.com/svn/apache/filters/mod_diagnostics.c
Source1:	http://apache.webthing.com/mod_diagnostics/index.html
Source2:	%{mod_conf}
Patch0:		mod_diagnostics-0.1-format_not_a_string_literal_and_no_format_arguments.diff
Requires(pre): rpm-helper
Requires(postun): rpm-helper
Requires(pre):	apache-conf >= 2.2.0
Requires(pre):	apache >= 2.2.0
Requires:	apache-conf >= 2.2.0
Requires:	apache >= 2.2.0
BuildRequires:	apache-devel >= 2.2.0
BuildRequires:	file
Epoch:		1

%description
mod_diagnostics is a very simple filter module. It can be inserted anywhere and
in any number of places in the Apache filter chain, and merely listens to and
reports traffic going through. It is  designed as a learning aid and a
debugging tool for Apache filter modules.

%prep

%setup -q -T -c -n %{mod_name}-%{version}
cp %{SOURCE0} .
cp %{SOURCE1} .
cp %{SOURCE2} %{mod_conf}

%patch0 -p0

# strip away annoying ^M
find . -type f|xargs file|grep 'CRLF'|cut -d: -f1|xargs perl -p -i -e 's/\r//'
find . -type f|xargs file|grep 'text'|cut -d: -f1|xargs perl -p -i -e 's/\r//'

%build

%{_bindir}/apxs -c %{mod_name}.c

%install

install -d %{buildroot}%{_libdir}/apache-extramodules
install -d %{buildroot}%{_sysconfdir}/httpd/modules.d

install -m0755 .libs/*.so %{buildroot}%{_libdir}/apache-extramodules/
install -m0644 %{mod_conf} %{buildroot}%{_sysconfdir}/httpd/modules.d/%{mod_conf}

install -d %{buildroot}%{_var}/www/html/addon-modules
ln -s ../../../..%{_docdir}/%{name}-%{version} %{buildroot}%{_var}/www/html/addon-modules/%{name}-%{version}

%post
if [ -f %{_var}/lock/subsys/httpd ]; then
    %{_initrddir}/httpd restart 1>&2;
fi

%postun
if [ "$1" = "0" ]; then
    if [ -f %{_var}/lock/subsys/httpd ]; then
	%{_initrddir}/httpd restart 1>&2
    fi
fi

%clean

%files
%doc index.html
%attr(0644,root,root) %config(noreplace) %{_sysconfdir}/httpd/modules.d/%{mod_conf}
%attr(0755,root,root) %{_libdir}/apache-extramodules/%{mod_so}
%{_var}/www/html/addon-modules/*




%changelog
* Sat Feb 11 2012 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-18mdv2012.0
+ Revision: 772617
- rebuild

* Tue May 24 2011 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-17
+ Revision: 678303
- mass rebuild

* Sun Dec 05 2010 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-16mdv2011.0
+ Revision: 609983
- rebuild
- rebuild

* Mon Mar 08 2010 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-14mdv2010.1
+ Revision: 516089
- rebuilt for apache-2.2.15

* Sat Aug 01 2009 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-13mdv2010.0
+ Revision: 406573
- rebuild

* Wed Jan 07 2009 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-12mdv2009.1
+ Revision: 326697
- fix build with -Werror=format-security

* Mon Jul 14 2008 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-11mdv2009.0
+ Revision: 234929
- rebuild

* Thu Jun 05 2008 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-10mdv2009.0
+ Revision: 215568
- fix rebuild

* Fri Mar 07 2008 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-9mdv2008.1
+ Revision: 181717
- rebuild

* Mon Feb 18 2008 Thierry Vignaud <tv@mandriva.org> 1:0.1-8mdv2008.1
+ Revision: 170717
- rebuild
- fix "foobar is blabla" summary (=> "blabla") so that it looks nice in rpmdrake
- kill re-definition of %%buildroot on Pixel's request

  + Olivier Blin <blino@mandriva.org>
    - restore BuildRoot

* Sat Sep 08 2007 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-7mdv2008.0
+ Revision: 82554
- rebuild


* Sat Mar 10 2007 Oden Eriksson <oeriksson@mandriva.com> 0.1-6mdv2007.1
+ Revision: 140666
- rebuild

* Sat Jan 27 2007 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-5mdv2007.1
+ Revision: 114307
- use the svn version and fix the source url

* Sat Jan 27 2007 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-4mdv2007.1
+ Revision: 114300
- bunzip the sources

* Thu Nov 09 2006 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-3mdv2007.1
+ Revision: 79404
- Import apache-mod_diagnostics

* Mon Aug 07 2006 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-3mdv2007.0
- rebuild

* Wed Dec 14 2005 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-2mdk
- rebuilt against apache-2.2.0

* Mon Nov 28 2005 Oden Eriksson <oeriksson@mandriva.com> 1:0.1-1mdk
- fix versioning

* Sun Jul 31 2005 Oden Eriksson <oeriksson@mandriva.com> 2.0.54_0.1-2mdk
- fix deps

* Fri Jun 03 2005 Oden Eriksson <oeriksson@mandriva.com> 2.0.54_0.1-1mdk
- rename the package
- the conf.d directory is renamed to modules.d
- use new rpm-4.4.x pre,post magic

* Sun Mar 20 2005 Oden Eriksson <oeriksson@mandrakesoft.com> 2.0.53_0.1-5mdk
- use the %1

* Mon Feb 28 2005 Oden Eriksson <oeriksson@mandrakesoft.com> 2.0.53_0.1-4mdk
- fix %%post and %%postun to prevent double restarts
- fix bug #6574

* Wed Feb 16 2005 Oden Eriksson <oeriksson@mandrakesoft.com> 2.0.53_0.1-3mdk
- spec file cleanups, remove the ADVX-build stuff

* Mon Feb 14 2005 Oden Eriksson <oeriksson@mandrakesoft.com> 2.0.53_0.1-2mdk
- new source, same version...
- new url

* Tue Feb 08 2005 Oden Eriksson <oeriksson@mandrakesoft.com> 2.0.53_0.1-1mdk
- rebuilt for apache 2.0.53

* Wed Sep 29 2004 Oden Eriksson <oeriksson@mandrakesoft.com> 2.0.52_0.1-1mdk
- built for apache 2.0.52

* Fri Sep 17 2004 Oden Eriksson <oeriksson@mandrakesoft.com> 2.0.51_0.1-1mdk
- built for apache 2.0.51

* Tue Jul 13 2004 Oden Eriksson <oeriksson@mandrakesoft.com> 2.0.50_0.1-1mdk
- built for apache 2.0.50
- remove redundant provides

* Tue Jun 15 2004 Oden Eriksson <oden.eriksson@kvikkjokk.net> 2.0.49_0.1-1mdk
- built for apache 2.0.49

