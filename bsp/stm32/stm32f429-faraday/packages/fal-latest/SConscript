
from building import *
import rtconfig

cwd     = GetCurrentDir()
src     = Glob('src/*.c')
CPPPATH = [cwd + '/inc']
LOCAL_CCFLAGS = ''

group = DefineGroup('fal', src, depend = ['PKG_USING_FAL'], CPPPATH = CPPPATH, LOCAL_CCFLAGS = LOCAL_CCFLAGS)

Return('group')
