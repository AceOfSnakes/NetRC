#!/usr/bin/env python3
import os
import re
from datetime import datetime

def get_metadata_from_debian():
    # Sensible defaults
    name = "avrpioremote"
    version = "26.06"
    url = "https://github.com"
    summary = "AVR PIO Remote application"  # Default fallback summary
    has_qt5_package = False

    # Extract Name and Version from changelog
    if os.path.exists('debian/changelog'):
        with open('debian/changelog', 'r') as f:
            first_line = f.readline().strip()
            match = re.search(r'^(\S+)\s+\(([^)]+)\)', first_line)
            if match:
                name = match.group(1)
                version = match.group(2).split('-')[0]

    # Extract Homepage, Summary (Synopsis), and package targets from control file
    if os.path.exists('debian/control'):
        with open('debian/control', 'r') as f:
            for line in f:
                # 1. Read SUMMARY as the first line of the Description: field
                if line.strip().startswith('Description:'):
                    # Strip the marker and grab the rest of that exact line
                    raw_summary = line.replace('Description:', '').strip()
                    if raw_summary:
                        summary = raw_summary
                
                if line.strip().startswith('Homepage:'):
                    url = line.replace('Homepage:', '').strip()
                
                # 2. Check if control file contains 'qt5' in any Package: field
                if line.strip().startswith('Package:'):
                    pkg_name = line.replace('Package:', '').strip()
                    if 'qt5' in pkg_name:
                        has_qt5_package = True
                    
    return name, version, url, summary, has_qt5_package

def convert_debian_changelog_to_rpm(changelog_path="debian/changelog"):
    if not os.path.exists(changelog_path):
        return "%changelog\n* Sun May 31 2026 Ace Of Snakes <AceOfSnakesMain@gmail.com> - 26.06-1\n- Automated packaging split."

    with open(changelog_path, 'r') as f:
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
    name, version, url, summary, has_qt5_package = get_metadata_from_debian()
    pct = "%"
    spec_content = f"""{pct}define _rpmfilename {pct}{pct}{{NAME}}_{pct}{pct}{{VERSION}}_{pct}{pct}{{ARCH}}.rpm
# FIXED: Overrides spec post-install scripts to completely skip doc analysis or compression checks
{pct}define __spec_install_post {pct}{{nil}}
{pct}define __brp_keep_la_files 1
{pct}define _build_id_links none
{pct}undefine _missing_build_ids_terminate_build


Name:           {name}
Version:        {version}
Release:        1{pct}{{?dist}}
Summary:        {summary}

License:        GPLv3+
URL:            {url}

Source0:        {name}_{version}_x86_64.txz
"""

    # Conditionally include Source1 depending on qt5 presence
    if has_qt5_package:
        spec_content += f"Source1:        {name}-qt5_{version}_x86_64.txz\n"

    spec_content += f"""
BuildRequires:  tar

{pct}description
With this software you are able to control your Pioneer receiver from your PC.
This package contains the binary compiled against the Qt6 framework.
"""

    # Conditionally generate Qt5 subpackage definition
    if has_qt5_package:
        spec_content += f"""
# --- SEPARATE RPM PACKAGE: Qt5 VERSION ---
{pct}package -n {name}-qt5
Summary:        {summary} (Qt5)

{pct}description -n {name}-qt5
With this software you are able to control your Pioneer receiver from your PC.
This package contains the binary compiled against the Qt5 framework.
"""

    spec_content += f"""
{pct}prep
rm -rf {pct}{{_builddir}}/{name}-{version}
mkdir -p {pct}{{_builddir}}/{name}-{version}
cd {pct}{{_builddir}}/{name}-{version}

mkdir source-qt6 && tar -xf {pct}{{SOURCE0}} -C source-qt6
"""

    # Conditionally extract Source1 in %prep
    if has_qt5_package:
        spec_content += f"mkdir source-qt5 && tar -xf {pct}{{SOURCE1}} -C source-qt5\n"

    spec_content += f"""
{pct}build
# Pre-compiled binaries require no compiler tasks

{pct}install
cd {pct}{{_builddir}}/{name}-{version}

rm -rf {pct}{{buildroot}}
mkdir -p {pct}{{buildroot}}

cp -a source-qt6/* {pct}{{buildroot}}/
"""

    # Conditionally copy Qt5 files in %install
    if has_qt5_package:
        spec_content += f"cp -a source-qt5/* {pct}{{buildroot}}/\n"

    spec_content += f"""
# FIXED: Explicitly delete the extracted usr/share/doc folders so RPM never processes them
rm -rf {pct}{{buildroot}}/usr/share/doc

# --- Manifest for Package 1: {name} (Qt6) ---
{pct}files
/opt/{name}/
/usr/share/applications/{name}.desktop
"""

    # Conditionally append Qt5 files manifest block
    if has_qt5_package:
        spec_content += f"""
# --- Manifest for Package 2: {name}-qt5 (Qt5) ---
{pct}files -n {name}-qt5
/opt/{name}-qt5/
/usr/share/applications/{name}-qt5.desktop
"""

    spec_content += "\n" + convert_debian_changelog_to_rpm() + "\n"

    with open(f"{name}.spec", "w") as spec_file:
        spec_file.write(spec_content)
    print(f"Successfully generated spec template for {name} (Qt5 subpackage included: {has_qt5_package})")

if __name__ == "__main__":
    generate_spec()
