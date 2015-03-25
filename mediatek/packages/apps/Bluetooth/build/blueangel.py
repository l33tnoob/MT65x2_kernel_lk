import sys, os, fnmatch, re

print "executing blueangel python script..."

###################################################
# Global Constants
###################################################
PROJECT_ROOT_DIR = "mediatek/packages/apps/Bluetooth"
PROJECT_OUT_DIR = sys.argv[1]
PROJECT_BUILD_DIR = PROJECT_ROOT_DIR + "/build"
MANIFEST_TEMPLATE = "AndroidManifest.tpl"
MANIFEST_FILENAME = "AndroidManifest.xml"
PLACEHOLDER_TEXT = "<!-- BLUEANGEL::PLACEHOLDER -->"
#MODULE_BEG_PATTERN = re.compile(".*BLUEANGEL::IF\s*(\S+)\s*=\s*(\S+).*")
#MODULE_END_PATTERN = re.compile(".*BLUEANGEL::FI.*")
MODULE_BEG_PATTERN = re.compile(".*BLUEANGEL::IF\s*(\S+)\s*.*")
MODULE_END_PATTERN = re.compile(".*BLUEANGEL::FI.*")

###################################################
# Function
###################################################

# sarch file(s) under given directory (e.g. search for all "AndroidManifest.xml")
def find_files(directory, pattern):
	for root, dirs, files in os.walk(directory):
		for basename in files:
			if fnmatch.fnmatch(basename, pattern):
				filename = os.path.join(root, basename)
				yield filename

# parsing module manifest xml file according feature-option and return activated content
def parse_module_file(filename):
	print "parsing module file:", filename
	moduleFile = open(filename, "r")
	moduleFileLines = moduleFile.readlines()
	moduleFile.close()
	moduleContent = []
	isInserting = False
	for line in moduleFileLines:
		# check if inserting mode end
		if isInserting and line.lstrip().startswith("<!--"):
			end = MODULE_END_PATTERN.match(line)
			if end:
				isInserting = False
		# inserting and not to end
		if isInserting:
			moduleContent.append(line)
			continue
		# check if inserting mode beg
		if line.lstrip().startswith("<!--"):
			beg = MODULE_BEG_PATTERN.match(line)
			if beg:
                               isInserting = True
			#	if featureOptions.get(beg.group(1)) == beg.group(2):
			#	if os.environ[beg.group(1)] == beg.group(2):		
	return ''.join(moduleContent)

###################################################
# Main Script
###################################################
# 1. load feature options
# Notes: direct reference with enviroment variable
#from mtkPythonPkg import featureConfig
#featureOptions = featureConfig.getFeatureConfig(sys.argv[1])

#featureOptions = {}
#featureOptions["MTK_BT_SUPPORT"] = "yes"
#featureOptions["MTK_BT_PROFILE_OPP"] = "yes"
#featureOptions["MTK_BT_PROFILE_FTP"] = "no"
#for fo in featureOptions.iteritems():
#	print "[%s]=[%s]" % (fo[0],fo[1])

# 2. compose all modules' AndroidManifest.xml content according to feature option
moduleContent = []
for moduleFile in find_files(PROJECT_ROOT_DIR, MANIFEST_FILENAME):
	moduleContent.append(parse_module_file(moduleFile))

# 3. read project manifest template (AndroidManifest.tpl)
templateFile = open(os.path.join(PROJECT_BUILD_DIR, MANIFEST_TEMPLATE), "r")
templateContent = templateFile.read()
templateFile.close()

# 4. compose project manifest content
manifestContent = templateContent.replace(PLACEHOLDER_TEXT, "".join(moduleContent))

# 5. write out project manifest file (./build/AndroidManifest.xml)
manifestFile = open(os.path.join(PROJECT_OUT_DIR, MANIFEST_FILENAME), "w")
manifestFile.write(manifestContent)
manifestFile.close()

#print "blueangel manifest:", manifestFile

###################################################
# End of File
###################################################

#targetDir = os.path.join(".", TARGET_DIR)
#if not os.path.exists(os.path.join(".", TARGET_DIR)):
#	print "mkdir:", targetDir
#	os.makedirs(targetDir)