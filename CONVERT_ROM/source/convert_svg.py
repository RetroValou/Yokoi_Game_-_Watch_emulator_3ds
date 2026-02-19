import os
import shutil
import subprocess
from lxml import etree
import re


################ Convert SVG to PNG ############################################
def extract_translation(transform):
    m = re.search(r'translate\(\s*(-?\d+(?:\.\d+)?)[ ,]+(-?\d+(?:\.\d+)?)\s*\)', transform)
    if m:
        return float(m.group(1)), float(m.group(2))
    return 0.0, 0.0



def _resolve_external_app(app_label: str, configured_path: str) -> str:
    """Resolve an external executable path or command name.

    If the executable cannot be found, raise a FileNotFoundError with a
    user-friendly message pointing at external_apps.py.
    """

    configured_path = str(configured_path).strip()
    if configured_path:
        # If it looks like a path, prefer checking it directly.
        looks_like_path = (
            os.path.isabs(configured_path)
            or ("\\" in configured_path)
            or ("/" in configured_path)
            or configured_path.lower().endswith((".exe", ".com", ".bat", ".cmd"))
        )
        if looks_like_path and os.path.exists(configured_path):
            return configured_path

        resolved = shutil.which(configured_path)
        if resolved:
            return resolved

    raise FileNotFoundError(
        f"external app {app_label} not found (configured as '{configured_path}'); "
        "check CONVERT_ROM/external_apps.py"
    )



def extract_group_segs(svg_path_list, output_dir, INKSCAPE_PATH, export_dpi: int = 50):
    os.makedirs(output_dir, exist_ok=True)

    inkscape_exe = _resolve_external_app("Inkscape", INKSCAPE_PATH)

    try:
        dpi = int(export_dpi)
    except Exception:
        dpi = 50
    if dpi <= 0:
        dpi = 50
    
    # Collect all exports first
    export_commands = []

    for i_path in range(len(svg_path_list)):
        svg_path = svg_path_list[i_path]
        tree = etree.parse(svg_path)
        root = tree.getroot()
        ns = {'svg': 'http://www.w3.org/2000/svg',
              'inkscape': 'http://www.inkscape.org/namespaces/inkscape'}

        segs_group = root.find('.//svg:g[@inkscape:label="segs"]', ns)
        if segs_group is None:
            print("ERROR NOT FIND SEG GROUP IN SVG")
            return
        # Detect problematic matrix transform (e.g., matrix(0.1,0,0,-0.1,0,1053))
        segs_transform = segs_group.attrib.get('transform', '')
        if 'matrix' in segs_transform:
            print(f"Detected matrix transform on segs group in {svg_path}. Moving transform to each segment.")
            # Move the matrix transform to each child element
            for element in segs_group:
                orig = element.attrib.get('transform', '')
                # Prepend the segs_group matrix to the element's transform
                if orig:
                    element.attrib['transform'] = segs_transform + ' ' + orig
                else:
                    element.attrib['transform'] = segs_transform
            segs_group.attrib['transform'] = ''
            tx, ty = 0.0, 0.0
        else:
            tx, ty = extract_translation(segs_transform)

        compteur = 1
        for element in segs_group:
            title_elem = element.find('svg:title', ns)
            if title_elem is not None and title_elem.text:
                nom_base = title_elem.text.strip() + '.' + str(i_path) 
                png_path = os.path.join(output_dir, f'{nom_base}.png')
                if not os.path.exists(png_path):
                    new_svg = etree.Element(root.tag, nsmap=root.nsmap)
                    for attr in ['width', 'height', 'viewBox']:
                        if attr == 'viewBox' and attr in root.attrib:
                            vb = root.attrib[attr].split()
                            min_x, min_y, w, h = map(float, vb)
                            min_x -= tx
                            min_y -= ty
                            new_svg.attrib[attr] = f"{min_x} {min_y} {w} {h}"
                        elif attr in root.attrib:
                            new_svg.attrib[attr] = root.attrib[attr]
                    clone = etree.Element(element.tag, nsmap=element.nsmap)
                    clone.attrib.update(element.attrib)
                    for child in element: 
                        clone.append(child)
                    new_svg.append(clone)
                    svg_temp_path = os.path.join(output_dir, f'{nom_base}.svg')
                    with open(svg_temp_path, 'wb') as f: 
                        f.write(etree.tostring(new_svg, pretty_print=True))
                    export_commands.append((svg_temp_path, png_path))
                    compteur += 1

    # Batch export using Inkscape shell mode
    if export_commands:
        print(f"Exporting {len(export_commands)} segments using Inkscape shell mode...")
        
        try:
            inkscape_process = subprocess.Popen(
                [inkscape_exe, '--shell'],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            
            commands = []
            for svg_path, png_path in export_commands:
                # Inkscape shell command format
                commands.append(f'file-open:{svg_path}; export-filename:{png_path}; export-dpi:{dpi}; export-do; file-close')
            
            commands.append('quit')
            
            stdout, stderr = inkscape_process.communicate('\n'.join(commands))
            
            if inkscape_process.returncode != 0:
                print(f"Inkscape shell mode warning: {stderr}")
            
            print(f" -- Exported {len(export_commands)} segments successfully")
            
        except Exception as e:
            if isinstance(e, FileNotFoundError) or getattr(e, "winerror", None) == 2:
                raise FileNotFoundError(
                    f"external app Inkscape not found (configured as '{INKSCAPE_PATH}'); "
                    "check CONVERT_ROM/external_apps.py"
                ) from e

            print(f"Error using Inkscape shell mode: {e}")
            print("Falling back to individual exports...")
            
            # Fallback to individual exports if shell mode fails
            for svg_path, png_path in export_commands:
                subprocess.run([
                    inkscape_exe, svg_path,
                    '--export-type=png', 
                    f'--export-filename={png_path}', 
                    f'--export-dpi={dpi}'
                ])
                print(f' -- Export : {png_path}')
        
        finally:
            # Clean up temp SVG files
            for svg_path, _ in export_commands:
                try:
                    os.remove(svg_path)
                except:
                    pass
