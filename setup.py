# ==============================================================================
# Copyright 2021 The channel sdk
# This file is part of the channel sdk library.
#
# The channel sdk library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The channel sdk library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with the channel sdk library. If not, see <http://www.gnu.org/licenses/>.
# =============================================================================="

"""channel sdk is a network library"""

import glob
from posixpath import dirname
import shutil
import os
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
import sys
import setuptools
from distutils import sysconfig

dir_name = "python"
sub_dir_name = "channel_sdk"
save_lib_dir = dir_name + "/" + sub_dir_name + "/"

so_libs = glob.glob('build/lib/lib*.so')
for file_name in so_libs:
    shutil.copy(file_name, save_lib_dir)

# gmssl
# if 'SSL_TYPE' in os.environ and os.environ['SSL_TYPE'] == '2':
#     print("start copy gmssl libraries.")
#     gmssl_so_libs = glob.glob('third_party/gmssl/lib/lib*.so.*')
#     for file_name in gmssl_so_libs:
#         shutil.copy(file_name, save_lib_dir)

print("start copy gmssl libraries.")
gmssl_so_libs = glob.glob('third_party/gmssl/lib/lib*.so.*')
for file_name in gmssl_so_libs:
    shutil.copy(file_name, save_lib_dir)

cc_module_name = "grpc"
build_ext_target = sub_dir_name + "/" + cc_module_name
DOCLINES = __doc__.split('\n')
__version__ = '1.0.0'

build_use_alone = False
if 'USE_ALONE' in os.environ and os.environ['USE_ALONE'] == 'ON':
    build_use_alone = True

root_dir = os.path.dirname(os.path.abspath(__file__))
include_dirs = []
include_dirs.append(root_dir)
include_dirs.append('.')
include_dirs.append('include')

# mpc includes
ccdir = root_dir
include_dirs.append(ccdir+"/core/include")
include_dirs.append(ccdir+"/core/src")

# thirdparty includes
include_dirs.append(ccdir+"/third_party/rapidjson/include")
include_dirs.append(ccdir+"/third_party/pybind11/include")

# libraries search path
library_dirs = ['.']
library_dirs.append(save_lib_dir)

# compile flags and definitions
extra_cflags = []
if build_use_alone:
    extra_cflags.append('-DUSE_ALONE=1')  # USE_ALONE

extra_cflags.append('-fPIC')  # general
extra_cflags.append('-Wno-unused-function')  # general
extra_cflags.append('-Wno-sign-compare')
extra_cflags.append('-std=c++11')  # temp c++11


extra_lflags = []
link_rpath = "$ORIGIN"
extra_lflags.append('-Wl,-rpath={}'.format(link_rpath))

print('extra_lflags', extra_lflags)
print('extra_cflags', extra_cflags)
print('library_dirs', library_dirs)
print('include_dirs', include_dirs)


ext_modules = [
    Extension(
        build_ext_target,
        ['core/pybind11/io_channel.cc'],
        # cc_files,
        include_dirs=include_dirs,
        libraries=['extio'],
        library_dirs=library_dirs,
        extra_compile_args=extra_cflags,
        extra_link_args=extra_lflags,
        language='c++'
    ),
    # others here
]

# disable debug
if sys.platform == 'linux' or sys.platform == "darwin":  # remove -g flags
    for k in sysconfig._config_vars.keys():
        if isinstance(sysconfig._config_vars[k], str):
            sysconfig._config_vars[k] = sysconfig._config_vars[k].replace(
                '-g ', ' ')

setup(
    name='channel-sdk',
    version=__version__,
    author='channel sdk',
    author_email='channel-sdk@juzix.cn',
    url='https://www.platon.network/',
    download_url='http://192.168.9.66/RosettaFlow/channel-sdk',
    description=DOCLINES[0],
    long_description='\n'.join(DOCLINES[2:]),
    package_dir={'': 'python'},  # where to find package
    packages=find_packages(
        'python', exclude=['build']),
    ext_modules=ext_modules,
    # Add in any packaged data.
    include_package_data=True,
    # install_requires=['numpy', 'pandas', 'sklearn'],
    setup_requires=['pybind11>=2.4'],
    zip_safe=False,
    # PyPI package information.
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'Intended Audience :: Education',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Topic :: Scientific/Engineering',
        'Topic :: Scientific/Engineering :: Mathematics',
        'Topic :: Scientific/Engineering :: Artificial Intelligence',
        'Topic :: Software Development',
        'Topic :: Software Development :: Libraries',
        'Topic :: Software Development :: Libraries :: Python Modules',
    ],
    license='LGPLv3',
    keywords='channel sdk',
)
