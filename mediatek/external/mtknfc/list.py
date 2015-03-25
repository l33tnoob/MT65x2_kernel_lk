import os
import stat,sys,re

def walktree (top = ".", depthfirst = True):
    names = os.listdir(top)
    if not depthfirst:
        yield top, names
    for name in names:
        try:
            st = os.lstat(os.path.join(top, name))
        except os.error:
            continue
        if stat.S_ISDIR(st.st_mode):
            for (newtop, children) in walktree (os.path.join(top, name), depthfirst):
                yield newtop, children
    if depthfirst:
        yield top, names

if len(sys.argv)<2:
  print "usage: list.py <dir root>"
  exit(0)

fHdr = open('header.mk','w')
fSrc = open('source.mk', 'w')
# dump all folders with header files
print >> fHdr, 'lib_common_c_includes := \\'
for (basepath, children) in walktree(sys.argv[1],False):
	basepath = basepath.replace(sys.argv[1],'$(LOCAL_PATH)')
	for child in children:
		if child.endswith(".h"):
			print >> fHdr, '    ',basepath,' \\'
			break
			
# dump the lists for source files	
print >> fSrc, 'lib_common_c_files := \\'
for (basepath, children) in walktree(sys.argv[1],False):
#	basepath = basepath.replace(sys.argv[1],'$(LOCAL_PATH)')
	basepath = basepath.replace(sys.argv[1],'.')	
	for child in children:
		if child.endswith(".c"):
			print >> fSrc, '    ',os.path.join(basepath, child), ' \\'

