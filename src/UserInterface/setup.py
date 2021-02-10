import sys
from cx_Freeze import setup, Executable

build_exe_options = {
    'include_files': ['Project/', 'Gui/']
}

base = None
if sys.platform == 'win32':
    base = 'Win32GUI'

setup(name = 'Gears',
      version = '1.0',
      description = 'Gears Stimulus Generator',
      options = { 'build_exe': build_exe_options,
                 'bdist_msi': {} },
      executables = [Executable('Gears1.py', base=base)])