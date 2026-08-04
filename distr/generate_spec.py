#!/usr/bin/env python3
import os
import re
from datetime import datetime

def get_metadata_from_debian():
    """
    Extracts name, version, url, summary, and subpackage availability
    dynamically from standard Debian build infrastructure configuration files.
    """
    # Safe universal fallbacks if files are missing
    name = "avrpioremote"
    version = "26.06"
    url = ""
    summary = "Network Remote application Suite"
    has_qt5_package = False

    # 1. Parse Name and Version cleanly from debian/changelog
    if os.path.exists('debian/changelog'):
        with open('debian/changelog', 'r', encoding='utf-8') as f:
            first_line = f.readline().strip()
            match = re.search(r'^(\S+)\s+\(([^)]+)\)', first_line)
            if match:
                name = match.group(1)
                # Strip out any debian revision suffix (e.g., "1.0.0-1" -> "1.0.0")
                version = match.group(2).split('-')[0]

    # 2. Parse Homepage, Summary, and Package targets from debian/control
    if os.path.exists('debian/control'):
        with open('debian/control', 'r', encoding='utf-8') as f:
            for line in f:
                if line.strip().startswith('Description:'):
                    raw_summary = line.replace('Description:', '').strip()
                    if raw_summary:
                        summary = raw_summary
                elif line.strip().startswith('Homepage:'):
                    url = line.replace('Homepage:', '').strip()
                elif line.strip().startswith('Package:'):
                    pkg_name = line.replace('Package:', '').strip()
                    if 'qt5' in pkg_name:
                        has_qt5_package = True

    return name, version, url, summary, has_qt5_package

def convert_debian_changelog_to_rpm(changelog_path="debian/changelog"):
    """
    Parses a standard Debian syntax changelog file and transforms it into
    a strictly-compliant, macro-friendly RPM %changelog section.
    """
    if not os.path.exists(changelog_path):
        return "%changelog\n* Sun May 31 2026 Maintainer <maintainer@example.com> - 1.0.0-1\n- Automated packaging template initialization."

    with open(changelog_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
        
    header_re = re.compile(r'^(\S+)\s+\(([^)]+)\)\s+([^;]+);')
    footer_re = re.compile(r'^ \-\- (.+?)\s+<(.+?)>\s+(.+)$')
    entries = []
    current_entry = None
    changes = []
    
    for line in lines:
        header_match = header_re.match(line)
        footer_match = footer_re.match(line)
        if header_match:
            current_entry = {'version': header_match.group(2).split('-')[0]}
        elif footer_match and current_entry:
            current_entry['maintainer'] = footer_match.group(1)
            current_entry['email'] = footer_match.group(2)
            deb_date_str = footer_match.group(3).strip()
            try:
                # Strip out the weekday prefix and time zone layout fluff
                clean_date = re.sub(r'^[A-Za-z]{3},\s+', '', deb_date_str)
                clean_date = re.sub(r'\s+[\+\-]\d{4}$', '', clean_date)
                dt = datetime.strptime(clean_date, "%d %b %Y %H:%M:%S")
                rpm_date = dt.strftime("%a %b %d %Y")
            except Exception:
                rpm_date = "Sun May 31 2026"
            current_entry['date'] = rpm_date
            current_entry['changes'] = changes
            entries.append(current_entry)
            current_entry = None
            changes = []
        else:
            stripped = line.strip()
            if stripped and not stripped.startswith('--') and stripped != '*':
                item = re.sub(r'^\*\s*', '', stripped)
                changes.append(item)
                
    rpm_changelog = "%changelog\n"
    for entry in entries:
        rpm_changelog += f"* {entry['date']} {entry['maintainer']} <{entry['email']}> - {entry['version']}-1\n"
        for change in entry['changes']:
            rpm_changelog += f"- {change}\n"
        rpm_changelog += "\n"
        
    return rpm_changelog.strip()

def generate_spec():
    # 1. Fetch data dynamically
    name, version, url, summary, has_qt5_package = get_metadata_from_debian()
    pct = "%"
    
    # 2. TRANSFORM ANY NAME TO A DYNAMIC CASE-INSENSITIVE RPM WILDCARD
    # Strips typical package naming variants, then converts "netrc" to "[Nn][Ee][Tt][Rr][Cc]*.png"
    clean_base_string = name.replace("-qt5", "").replace("-qt6", "")
    icon_wildcard = "".join([f"[{c.upper()}{c.lower()}]" for c in clean_base_string]) + "*.png"
    
    # EXCLUSION MAPPING: Forces the base package to reject any file ending in a 5 right before the .png extension
    qt6_icon_filter = icon_wildcard.replace(".png", "[!5].png")
    
    # Generate the string layout
    spec_content = f"""{pct}define _rpmfilename {pct}{pct}{{NAME}}_{pct}{pct}{{VERSION}}_{pct}{pct}{{ARCH}}.rpm
{pct}define __spec_install_post {pct}{{nil}}
{pct}define __brp_keep_la_files 1
{pct}define _build_id_links none
{pct}undefine _missing_build_ids_terminate_build

Name: {name}
Version: {version}
Release: 1{pct}{{?dist}}
Summary: {summary}
License: GPLv3+
URL: {url}
Source0: {name}_{version}_x86_64.txz
"""

    if has_qt5_package:
        spec_content += f"Source1: {name}-qt5_{version}_x86_64.txz\n"

    spec_content += f"""
BuildRequires: tar

{pct}description
Application suite generated automatically from source tree configuration bindings.
This package contains the binary compiled against the Qt6 framework.
"""

    if has_qt5_package:
        spec_content += f"""
# --- SEPARATE RPM PACKAGE: Qt5 VERSION ---
{pct}package -n {name}-qt5
Summary: {summary} (Qt5)

{pct}description -n {name}-qt5
Application suite generated automatically from source tree configuration bindings.
This package contains the binary compiled against the Qt5 framework.
"""

    # --- SCRIPTLETS FOR RUNTIME CACHE RELOADS ---
    spec_content += f"""
# --- Scriptlets for Base Package ({name}) ---
{pct}post
/usr/bin/touch --no-create {pct}{{_datadir}}/icons/hicolor &>/dev/null || :
/usr/bin/update-desktop-database &>/dev/null || :

{pct}postun
if [ $1 -eq 0 ] ; then
    /usr/bin/gtk-update-icon-cache {pct}{{_datadir}}/icons/hicolor &>/dev/null || :
    /usr/bin/update-desktop-database &>/dev/null || :
fi
"""

    if has_qt5_package:
        spec_content += f"""
# --- Scriptlets for Qt5 Subpackage ({name}-qt5) ---
{pct}post -n {name}-qt5
/usr/bin/touch --no-create {pct}{{_datadir}}/icons/hicolor &>/dev/null || :
/usr/bin/update-desktop-database &>/dev/null || :

{pct}postun -n {name}-qt5
if [ $1 -eq 0 ] ; then
    /usr/bin/gtk-update-icon-cache {pct}{{_datadir}}/icons/hicolor &>/dev/null || :
    /usr/bin/update-desktop-database &>/dev/null || :
fi
"""

    spec_content += f"""
{pct}prep
rm -rf {pct}{{_builddir}}/{name}-{version}
mkdir -p {pct}{{_builddir}}/{name}-{version}
cd {pct}{{_builddir}}/{name}-{version}
mkdir source-qt6 && tar -xf {pct}{{SOURCE0}} -C source-qt6
"""

    if has_qt5_package:
        spec_content += f"mkdir source-qt5 && tar -xf {pct}{{SOURCE1}} -C source-qt5\n"

    spec_content += f"""
{pct}build
# Pre-compiled production bundles require no local compilation tasks

{pct}install
cd {pct}{{_builddir}}/{name}-{version}
rm -rf {pct}{{buildroot}}
mkdir -p {pct}{{buildroot}}
cp -a source-qt6/* {pct}{{buildroot}}/
"""

    if has_qt5_package:
        spec_content += f"cp -a source-qt5/* {pct}{{buildroot}}/\n"

    spec_content += f"""
rm -rf {pct}{{buildroot}}/usr/share/doc

# --- Manifest for Package 1: {name} (Qt6) ---
{pct}files
/opt/{name}/
{pct}{{_datadir}}/applications/{name}.desktop
# GENERIC EXCLUSION WILDCARD: Matches your icon, but ignores any matching -Qt5 variants
{pct}{{_datadir}}/icons/hicolor/*/*/{qt6_icon_filter}
"""

    if has_qt5_package:
        # Appends '-Qt5' identifier explicitly before the filename extension matching pattern
        qt5_icon_wildcard = icon_wildcard.replace(".png", "-Qt5.png")
        
        spec_content += f"""
# --- Manifest for Package 2: {name}-qt5 (Qt5) ---
{pct}files -n {name}-qt5
/opt/{name}-qt5/
{pct}{{_datadir}}/applications/{name}-qt5.desktop
# GENERIC INCLUSION WILDCARD: Matches only the explicit Qt5 variant icon file
{pct}{{_datadir}}/icons/hicolor/*/*/{qt5_icon_wildcard}
"""

    # 3. Inject parsed changelog blocks cleanly
    spec_content += "\n" + convert_debian_changelog_to_rpm() + "\n"

    # 4. Flush file content directly to disk
    output_filename = f"{name}.spec"
    with open(output_filename, "w", encoding='utf-8') as spec_file:
        spec_file.write(spec_content)

    print(f"Successfully generated 100% generic spec asset file: {output_filename}")
    print(f" -> Tracked Base Icon Pattern: {qt6_icon_filter}")
    
    # SAFE BLOCK: Only attempts to read and print the Qt5 pattern if the subpackage exists
    if has_qt5_package:
        print(f" -> Tracked Qt5  Icon Pattern: {qt5_icon_wildcard}")
        
    print(f" -> Qt5 Subpackage Detected: {has_qt5_package}")

if __name__ == "__main__":
    generate_spec()


