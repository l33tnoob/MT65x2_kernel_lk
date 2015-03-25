#!/usr/bin/python
# nonNDK issues protect module - Jan 11 2012
# This module will parse all the module header files and check mediatek code change,
# apply the protect machnism if mediatek's chang with nonNDK issues.
# By Liwen Tan, liwen.tan@mediatek.com
# License: MediaTek

import os 
import sys
import re
import lex
import datetime
import fileinput
import shutil
import relpath

sys.path = ["../"] + sys.path
import CppHeaderParser

t_NONNDK_MODULES = r'nonNdk.modules'
t_NONNDK_OFF      = r'0'       
t_ANDROID_DEFAULT = r'1'
t_MEDIATEK_WRAPPER = r'2'
t_ANDROID_DEFAULT_CODE = r'ANDROID_DEFAULT_CODE'


nonndk_debug = False

class nonNdkClass:
    def __init__(self, srcs, incs, localDir, outDir):
        self.cppHeader = ""
        self.nonNdkIssue = False
        self.MACRO = t_ANDROID_DEFAULT_CODE
        self.LOCAL_SRC_FILES = srcs 
        self.LOCAL_C_INCLUDES = incs
        self.LOCAL_PATH = localDir
        self.OUT_PATH = outDir
        
        self.totalNdkClass = []
        self.hasGlobalContent = {}

        self.modules = re.split('\W*.cpp', self.LOCAL_SRC_FILES)
        for i in range(len(self.modules)):
            self.modules[i] = re.sub('\s+','',self.modules[i])
            if('/' in self.modules[i]):
                m = re.split('/',self.modules[i])
                m = m[len(m)-1]
                self.modules[i] = m

        self.modules.remove('') 

        # Begin to process the header and cpp files
        self.process()
    
    # -----------------------------------------------------------------------------
    # parseHeaderFile() - Call CppHeaderParser to parse the cpp header files
    # parameter:
    #     filename - cpp header file name
    # -----------------------------------------------------------------------------
    def parseHeaderFile(self,filename):
        try:
            self.headerfile  = filename;
            self.cppHeader = CppHeaderParser.CppHeader(filename)
        except CppHeaderParser.CppParseError,  e:
            if nonndk_debug: print e
            sys.exit(1)
    
    # -----------------------------------------------------------------------------
    # moduleToSrc() - Get module name from LOCAL_SRC_FILES string
    # parameter:
    #     module - module name
    # -----------------------------------------------------------------------------
    def moduleToSrc(self, module):
        if len(self.LOCAL_SRC_FILES) == 0 or module == "":
            return ""

        srcs = self.LOCAL_SRC_FILES.split()
        cpp  = module+".cpp"
        for src in srcs:
            if cpp in src:
                return src
        return ""

    # -----------------------------------------------------------------------------
    # bypassDefinition () - Bypass the class or struct definition with no nonndk issue
    # in a file
    # parameter:
    #     filePath - File need be process 
    # -----------------------------------------------------------------------------
    def bypassDefinition(self, filePath):
        if not os.path.exists(filePath):
            return
        
        fcontents = open(filePath).read()
        re1 = r'(class|struct)\s+(?P<classname1>([a-zA-Z_0-9]*))\s*;'
        re2 = r'(class|struct)\s+(?P<classname2>([a-zA-Z_0-9]*))[^\{]*\{.*[^\n+\};\n+]*\n+\};\n+'
        
        ree = []
        ree.append(re1)
        ree.append(re2)
        ree = '|'.join(ree)
        fre = re.compile(ree, re.DOTALL)

        ms = fre.finditer(fcontents)

        for m in ms:
            cn1 = ""
            cn2 = ""
            if m.group('classname1'):
                cn1 = m.group('classname1')
                if nonndk_debug: print cn1
            if m.group('classname2'):
                cn2 = m.group('classname2')
                if nonndk_debug: print cn2

            if not self.nonNdkClass.get(cn1):
                fcontents.replace(m.group(0),'')

            if not self.nonNdkClass.get(cn2):
                fcontents.replace(m.group(0),'')

        f = open(filePath,"w")
        f.write(fcontents)
        f.close()

    # -----------------------------------------------------------------------------
    # bypassImplement () - Bypass the function implementation which no nonndk issue
    # in a file
    # parameter:
    #      filePath - File need be process
    # -----------------------------------------------------------------------------
    def bypassImplement(self, filePath):
        if not os.path.exists(filePath):
            return

        fcontents = open(filePath).read()
        ire = re.compile(r'\n+(?P<classstr>([a-zA-Z_0-9\s*&<>]*))(::[~a-zA-Z_0-9]+)+\s*\([^\)]*\)\s*(const)*[^\{]*\{.*?[^\n+\}\n+]*\n+\}\n', re.DOTALL)
        
        ms = ire.finditer(fcontents)
        for m in ms:
            if nonndk_debug: print m.group(0)
            if m.group('classstr'):
                classstr = m.group('classstr').split()
                l = len(classstr)
                classname = classstr[l-1]
                if not self.nonNdkClass.get(classname):
                    fcontents.replace(m.group(0),'')

        f = open(filePath, "w")
        f.write(fcontents)
        f.close()
   
    # -----------------------------------------------------------------------------
    # classGlobalContent () - A class global content 
    # parameter:
    #      className - The class's name 
    # -----------------------------------------------------------------------------
    def classGlobalContent(self, className):
        if len(self.hasGlobalContent):
            if self.hasGlobalContent.get(className):
                return ""
        string = "#include <utils/KeyedVector.h> \n"
        string = string + "#include <utils/threads.h> \n"
        string = string + "static KeyedVector<int, %s_mtk*> mtk%sObjList; \n" %(className, className)
        string = string + "static Mutex mtk%sObjListLock; \n\n" %(className)

        self.hasGlobalContent[className] = True
        return string
   
    # -----------------------------------------------------------------------------
    # createFileSoftLink () - Create a file soft link for a header file 
    # parameter:
    #      name - The link name
    #      target - Link target file
    # -----------------------------------------------------------------------------
    def createFileSoftLink(self, name, target):
        if os.path.exists(name) or os.path.islink(name):
            os.remove(name)
        if os.path.exists(target):
            # Create soft link with absolute path
            t_relpath = relpath.relpath(os.path.dirname(name), os.path.dirname(target), '/')
            t = t_relpath + "/" + os.path.basename(target)
            os.system("ln -s %s %s" %(t, name))
        return

    # -----------------------------------------------------------------------------
    # createCflagsStr() - Create a cflags string for redeine class xxxx to xxxx_mtk,
    # and add it into TARGET_GLOBAL_CFLAGS
    # parameter:
    #     None
    # -----------------------------------------------------------------------------
    def createCflagsStr(self):
        cflags = "TARGET_GLOBAL_CFLAGS +="
        for c in self.totalNdkClass:
            cflags += "-D%s=%s_mtk " %(c,c)

        cflagsfile = self.OUT_PATH + '/' + 'cflags.cfg'
        f = file(cflagsfile, "w")
        f.write(cflags)
        f.close()

    # -----------------------------------------------------------------------------
    # createNonndkHeaderFile() - Create nonndk.h file to keep the #define xxx xxx_mtk
    # and add it into TARGET_GLOBAL_CFLAGS.
    # - (TARGET_GLOBAL_CFLAGS += -include $(TOPDIR)meditek/source/external/nonNDK/nonndk.h
    # parameter:
    #     None
    # -----------------------------------------------------------------------------
    def createNonndkHeaderFile(self):
        #path = os.path.dirname(os.path.realpath(__file__))
        #if not path: return
        
        path = self.OUT_PATH + "/" + "nonndk.h"
        
        if not os.path.exists(path):
            f = file(path,"w") 
            if not f: 
                print "Create file %s failed!!!" %(path)
                return
            f.close()
       
        headerfile = file(path,"r+")
        lines = headerfile.readlines()

        defines = []
        for c in self.totalNdkClass:
            defines.append("#define %s %s_mtk \n" %(c,c))
        if defines != lines:
            headerfile.seek(0)
            headerfile.truncate(0)
            for define in defines:
                headerfile.write(define)

        headerfile.close()

        configfile = file("system/core/include/arch/mtkNonNDKConfig.h", "r+")

        sap = os.getcwd() + "/" + "system/core/include/arch/"
        if path.startswith('/'):
            tap = path
        else:
            tap = os.getcwd() + "/" + path
        
        rp = relpath.relpath(os.path.dirname(sap), os.path.dirname(tap), "/")
        rp = rp + "/" + os.path.basename(path)
        
        string = r'#include "'
        #string += "../../../../" + str(path)
        string += rp
        string += r'"'
        string += "\n"
        lines = configfile.readlines()
        if not string in lines:
            configfile.write(string)
        configfile.close()

    # -----------------------------------------------------------------------------
    # createModulesFile() - Create nonNdk.modules file and set default value in it 
    # if this file is not exist
    # parameter:
    #     None
    # -----------------------------------------------------------------------------
    def createModulesFile(self, path):
        
        srcs = self.LOCAL_SRC_FILES.split()
        f = file(path,"w")
        if not f: 
            print "Open file %s for write failed!" %(path)
            return

        f.write("#Modules maybe with non-NDK issues, need be protected\n")
        f.write("#Module Name		                                        Protected Mode(0: nonndk off, 1: Android default, 2: MTK wrapper)\n")

        for src in srcs:
            if not re.search('mediatek',src):
                string = src
                string += ' ' * (60 - len(src))
                string += '2'  # default use Mediatek wrapper
                string += '\n'
                f.write(string)
                
        f.close()

    # -----------------------------------------------------------------------------
    # constructorMtkTrans() - Construct constructor for MTK wrapper transfer,
    # parameter:
    #     className: class name
    #     param: parameters for class constructor
    # -----------------------------------------------------------------------------  
    def constructorMtkTrans(self, className, param, isBlank):
        string = ""
        if not isBlank:
            string = "    %s_mtk* mtkInst = new %s_mtk(%s); \n" %(className, className, param)
            string = string + "    mtk%sObjListLock.lock(); \n" %(className)
            string = string + "    mtk%sObjList.add((int)this, mtkInst); \n" %(className)
            string = string + "    mtk%sObjListLock.unlock(); \n" %(className)
        return string 

    # -----------------------------------------------------------------------------
    # deconstructorMtkTrans(className, isPublic) - Construct deconstructor for MTK wrapper transfer,
    # parameter:
    #     className: class name
    #     isPublic:  Is the deconstructor function is publi type
    # -----------------------------------------------------------------------------  
    def deconstructorMtkTrans(self, className, isPublic):
        string = "    %s_mtk *inst = NULL; \n" %(className)
        string = string + "    mtk%sObjListLock.lock(); \n" %(className)
        string = string + "    inst = mtk%sObjList.valueFor((int)this); \n" %(className)
        string = string + "    mtk%sObjList.removeItem((int)this); \n" %(className)
        string = string + "    mtk%sObjListLock.unlock(); \n" %(className)
        ########## Need cover the cases of deconstructor function is private or protected #######
        if isPublic: string = string + "    if(inst) delete inst; \n"  
        return string

    # -----------------------------------------------------------------------------
    # commonMethodMtkTrans() - Construct common method for MTK wrapper transfer,
    # parameter:
    #     className: class name
    #     rtnType: return type for this method
    #     function: function name and parameters information string
    # -----------------------------------------------------------------------------  
    def commonMethodMtkTrans(self, className, rtnType, function, isStatic):
        if isStatic:
            string = "    return %s_mtk::%s; \n" %(className, function)
        else:
            string = "    %s_mtk *inst = NULL; \n" %(className)
            string = string + "    mtk%sObjListLock.lock(); \n" %(className)
            string = string + "    inst = mtk%sObjList.valueFor((int)this); \n" %(className)
            string = string + "    mtk%sObjListLock.unlock(); \n" %(className)
            string = string + "    return (%s)inst->%s; \n" %(rtnType, function)
        return string 
   
    # -----------------------------------------------------------------------------
    # staticMethodMtkTrans() - Construct static method for MTK wrapper transfer,
    # parameter:
    #     className: class name
    #     param: function string with parametiers information
    #     cstRtntype: Return type of this method
    #     cstParam: constructor parameters information
    # -----------------------------------------------------------------------------  
    def staticMethodMtkTrans(self, className, param, cstRtntype, cstParam):
        string = ""
        string += "    %s    rtn; \n" %(cstRtntype)
        consparams = ""
        i = 0
        for p in cstParam:
            paramname = 'a' + str(i)
            if p.endswith('&'):
                string += "    %s %s = NULL;\n" %(p,paramname)
            else:
                string += "    %s %s;\n" %(p,paramname)
            consparams += paramname + ","
            i += 1
        if consparams.endswith(','): consparams = consparams[:-1]

        if className in cstRtntype:
            string += "    %s obj = new %s(%s); \n" %(cstRtntype, className, consparams)
            tempstr = cstRtntype.replace(className,className+'_mtk')
            string += "    %s mtkInst = %s_mtk::%s; \n" %(tempstr, className, param)
        else:
            string += "    %s    *obj = new %s(%s); \n" %(className, className, consparams)
            string += "    %s_mtk *mtkInst = %s_mtk::%s; \n" %(className, className, param)
        
        string += "    if (mtkInst == NULL){ \n"
        if className in cstRtntype and re.search(r'^sp( )*<', cstRtntype.lstrip()):
            string += "        if(obj.get()) obj.clear();\n"    
        else:
            string += "        if(obj != NULL) delete obj;\n"
        
        string += "        return NULL;\n"
        string += "    }\n"
        string += "    mtk%sObjListLock.lock(); \n" %(className)
        
        if className in cstRtntype and re.search(r'^sp( )*<', cstRtntype.lstrip()):
            string += "    mtk%sObjList.add((int)obj.get(), (%s_mtk*)mtkInst.get()); \n" %(className, className)
        else:
            string += "    mtk%sObjList.add((int)obj, mtkInst); \n" %(className)
        string += "    mtk%sObjListLock.unlock(); \n" %(className)
        string += "    return (%s)obj; \n" %(cstRtntype)
        return string

    
    # -----------------------------------------------------------------------------
    # createMtkwrpHeadfileDef() - Create content for MTK wrapper header file definition
    # parameter:
    #     None
    # -----------------------------------------------------------------------------
    def createMtkwrpHeaderFile(self, oldContents):
        string = ""
        for key, value in self.nonNdkClass.items():
            tempstr = ""
            if value:
                c = self.cppHeader.classes[key]

                #write class name and inherits
                tempstr += c["type"] + " " + c["name"] + " "
                if c["inherits"]: 
                    tempstr += ": "
                    for inherit in c["inherits"]:
                        tempstr += inherit["access"] + " "
                        tempstr += inherit["class"]
                        tempstr += ","
                if tempstr [-1:] == ",": tempstr = tempstr[:-1]
                tempstr += " {\n"
 
                #write enums
                enums = c["enums"]["public"]
                if len(enums): tempstr += "public:\n"
                for enum in enums:
                    ere = re.compile(r'enum( )*(\\n)*%s( |\\n)*\{[^\}]*\};' %(enum["name"]), re.DOTALL)
                    m = ere.search(oldContents)
                    if m: 
                        tempstr += "\n    " + m.group(0) + "\n"
                    else:
                        if enum["values"]:
                            tempstr += "    " + "enum %s {\n" %(enum["name"])
                            for e in enum["values"]:
                                tempstr += "        " + e["name"]
                                if e.get("value"): 
                                    tempstr += " "*(32 - len(e["name"]))
                                    tempstr += "= " + e["value"]
                                tempstr += ",\n"
                        tempstr += "    };\n"

                #write function
                for key,value in sorted(c["methods"].items(),reverse=True):
                    if value: tempstr += key + ":\n"
                    for method in value:
                        if method["mtkChange"]: continue
                        if key == "protected" or key == "private":
                            if method["name"] != c["name"] and method["name"] != ("~"+c["name"]):
                                if "virtual" not in method["rtnType"]:
                                    continue 
                        tempstr += "    "
                        

                        if len(method["rtnType"]):
                            stdre = re.compile(r'\s*:\s*:\s*')
                            stdm = stdre.search(method["rtnType"])
                            if stdm:
                                tempstr += method["rtnType"].replace(stdm.group(0),"::")
                            else:
                                tempstr += method["rtnType"] 
                            tempstr += " " 
                        tempstr += method["name"]
                        tempstr += "("
                        for param in method["parameters"]:
                            tempstr += param["type"] + " " + param["name"]
                            if param.has_key("defaltValue"):
                                tempstr += " = %s" %(''.join(param["defaltValue"].split()))
                            tempstr += ","
                        if tempstr[-1:] == ",": tempstr = tempstr[:-1]
                        if method["const"]:
                            tempstr += ") const"
                        else:
                            tempstr += ")"

                        if method["pure_virtual"]:
                            tempstr += " = 0;\n"
                        else:
                            tempstr += ";\n"

            if tempstr: string += tempstr

        return string
    
    # -----------------------------------------------------------------------------
    # createMtkTransContent() - Create content for MTK wrapper transfer,
    # parameter:
    #     None
    # -----------------------------------------------------------------------------  
    def createMtkTransContent(self):
        string = ""
        for key,value in self.nonNdkClass.items():
            if value:
                c = self.cppHeader.classes[key]
                public_methods = c["methods"]["public"]
                string = string + self.genMethodContent(c["name"], public_methods, False)
                   
                protected_methods = c["methods"]["protected"]
                string = string + self.genMethodContent(c["name"], protected_methods, True)

                private_methods = c["methods"]["private"]
                string = string + self.genMethodContent(c["name"], private_methods, True)
                

        return string

    # -----------------------------------------------------------------------------
    # isNamespaceAndr() - Judge whether encounter the android namespace string,
    # parameter:
    #     line: line need parse
    # -----------------------------------------------------------------------------  
    def isNamespaceAndr(self, line):
        string = line.split()
        if "namespace" in string and "android" in string and "{" in string:
            return True
        return False

    # -----------------------------------------------------------------------------
    # isOperatorMethod() - Judge whether it is an operator method
    # parameter:
    #     name: method name
    # -----------------------------------------------------------------------------  
    def isOperatorMethod(self, name):
        if "operator" in name: return True
        return False

    # -----------------------------------------------------------------------------
    # parseConstructor
    # parameter:
    #      
    # -----------------------------------------------------------------------------  
    def parseConstructor(self, classname, inherits, consstr):
        hf = os.path.basename(self.headerfile)
        cppfile = self.OUT_PATH + "/" + hf[:-2] + "_def.cpp"

        if not os.path.exists(cppfile):
            print "Not exist file %s " %(cppfile)
            return (False,'')

        f = open(cppfile)
        cre = re.compile(r'%s::%s\([^\{]*\{' %(classname,classname), re.DOTALL)
        sre = re.compile(r'%s::%s\([^\)]*\)' %(classname, classname), re.DOTALL | re.VERBOSE)

        fcontents = f.read()
        str1 = consstr[consstr.index('(')+1:]
        str1 = str1.split(',')
        s1 = ""
        for s in str1:
            s1 += ''.join(s.split(' ')[:-1])
            m = re.search(r'^[\*|\&]+', ''.join(s.split(' ')[-1:]))
            if m: s1 += m.group(0)
        s1 = ''.join(s1.split())
        
        cms = cre.findall(fcontents)
        for cm in cms:
            ms = sre.findall(cm)
            for m in ms:
                m = m[m.index('(')+1:]
                m = m.split(',')
                
                s2 = ""
                for s in m:
                    s2 += ''.join(s.split(' ')[:-1])
                    s2m = re.search(r'^[\*|\&]+', ''.join(s.split(' ')[-1:]))
                    if s2m: s2 += s2m.group(0)

                s2 = ''.join(s2.split())
               
                if s1 == s2: 
                    instr = ""
                    for inherit in inherits:
                        ire = re.compile(r'%s\([^\)]*\)' %(inherit['class']), re.DOTALL)
                        ims = ire.search(cm)
                        if ims: instr += ims.group(0) + ','
                    if instr.endswith(','): instr = instr[:-1]
                    return (True,instr)

        if f: f.close()
        return (False,'')
  
    # -----------------------------------------------------------------------------
    # handleSpecialMethodRtntype() - handle special class method return type
    # parameter:
    #   classname : class name
    #   rtntype : class method return type
    # -----------------------------------------------------------------------------
    def handleSpecialMethodRtntype(self, classname, rtntype):
        stdre = re.compile(r'\s*:\s*:\s*')
        stdm = stdre.search(rtntype)
        if stdm:    rtntype = rtntype.replace(stdm.group(0),"::")
        
        if self.cppfile:
            clsre = re.compile(r'(?P<clsstr>[a-zA-Z_0-9]+::)%s\s*%s' %(rtntype, classname))
            clsm = clsre.search(open(self.cppfile).read())
            if clsm: rtntype = clsm.group("clsstr") + rtntype

        return rtntype
     
    
    # -----------------------------------------------------------------------------
    # genMethodContent() - Generate the method content for MTK wrapper
    # parameter:
    #     className: class name
    #     methods: methods set for property: public, protected or privated
    #     isBlank: True-> generate blank content for non-public method, 
    #              False-> common generate
    # -----------------------------------------------------------------------------  
    def genMethodContent(self, className, methods, isBlank):
        string = ""
        hasGenGlobal = False
        pureClass = False
        for m in methods:
            if m["pure_virtual"]: 
                pureClass = True
                break

        for m in methods:
            if m["mtkChange"]: continue
            if m["pure_virtual"]: continue
            if "operator" in m["name"]: continue
            if "DISALLOW_EVIL_CONSTRUCTORS" == m["name"]: continue
            if "typedef" in m["rtnType"]: continue
            if isBlank and m["name"] != className and m["name"][1:] != className: 
                if "virtual" not in m["rtnType"]: continue

            function = ""
            funcstr = ""
            t = m["rtnType"]
            if "virtual" in t: t = t[7:].lstrip()
            if "static" in t:  t = t[7:].lstrip()
            if "inline" in t:  t = t[7:].lstrip()


            function = "%s::%s(" %(className, m["name"])
            funcstr  = "%s(" %(m["name"])
           
            num = len(m["parameters"])
            for param in m["parameters"]:
                if param == "[]": break
                num = num - 1
                function = function + "%s %s" %(param["type"], param["name"])
                if not param["name"].startswith('<'):
                    #Check whether param type is class enum type
                    enums = self.cppHeader.classes[className]["enums"]["public"]
                    enumtype = ""
                    for enum in enums:
                        if param["type"] == enum["name"]:
                            enumtype = "(" + className + "_mtk::" + param["type"] + ")"
                            break
                    funcstr  = funcstr + "%s" %(enumtype + param["name"])
                    if num: funcstr = funcstr + ","
                if num: 
                    function = function + ","

            function = function + ")"
            funcstr  = funcstr  + ")"

            if m["const"]:
                function += "  const"

            if m["name"] == className:
                (f,c) = self.parseConstructor(className, self.cppHeader.classes[className]["inherits"], function)
                if not f: continue
                if c: function += "\n:" + c
                string = string + function
                string = string + "\n{ \n"
                #paramstr = funcstr[(funcstr.index('(')+1):funcstr.index(')')]
                paramstr = funcstr[(funcstr.index('(')+1):-1]
                if not pureClass: string = string + self.constructorMtkTrans(className, paramstr, isBlank)
            elif m["name"][1:] == className:
                string = string + function
                string = string + "\n{ \n"
                if isBlank:
                    string = string + self.deconstructorMtkTrans(className, False)
                else:
                    string = string + self.deconstructorMtkTrans(className, True)
            else:
                t = self.handleSpecialMethodRtntype(className,t)

                if t:
                    if t.split()[0] in self.cppHeader.classes[className]['subclass']: 
                        string = string + "%s::%s    " %(className, t)
                    else:    
                        string = string + "%s    " %(t)
                string = string + function
                string = string + "\n{ \n"
                if isBlank:
                    if t == "void": string = string + "    return; \n"
                    else: string = string + "    %s rtn; \n    return rtn; \n" %(t)
                else:
                    if "static" in m["rtnType"] and className in m["rtnType"] and not pureClass:
                        pri_method = self.cppHeader.classes[className]["methods"]["private"]
                        paramnum = 0
                        paramlist = []
                        for method in pri_method:
                            if method['name'] == className:
                                for param in method['parameters']:
                                    paramlist.append(param['type'])
                                break
                
                        string = string + self.staticMethodMtkTrans(className, funcstr, t, paramlist)
                    else:
                        isStatic = False
                        if "static" in m["rtnType"]: isStatic = True 
                        string = string + self.commonMethodMtkTrans(className, t, funcstr, isStatic)
                    
            string = string + "} \n \n"
        return string
    
    # -----------------------------------------------------------------------------
    # getHeaderFileByCpp() - Get the cpp header file by cpp source file
    # parameter:
    #     cpp: cpp file
    # ----------------------------------------------------------------------------- 
    def getHeaderFileByCpp(self, cpp):
        if len(self.LOCAL_C_INCLUDES) == 0 or not cpp.find(".cpp"):
            return ""
        
        src = self.LOCAL_PATH + '/' + cpp
        headerfile = ""
        syspath = False
        
        if os.path.isfile(src):
            fcpp = file(src)
            h = os.path.basename(src)[:-4] + ".h"
            line = ''
            
            while True:
                line = fcpp.readline()
                if len(line) == 0:
                    break
                if line.lstrip().startswith("#include"):
                    if re.search('"%s"'%(h), line) or re.search('/%s'%(h),line):
                        if("<" in line and ">" in line):
                            syspath = True
                            temps = re.split(r'.*?<(.*?)>', line)
                        else:
                            syspath = False
                            temps = re.split(r'.*?"(.*?)"', line) 
                        for temp in temps:
                            if len(temp) > 2:
                                headerfile = temp
                                break
                        break
            fcpp.close()
        else:
            return ""   
        
        if headerfile == '':
            return ''
        
        incs = self.LOCAL_C_INCLUDES.split() 
        #Search in local paths
        if(not syspath):
            hf = self.LOCAL_PATH + '/' + headerfile
            if os.path.isfile(hf):
                return hf
            else:
                return ""
        
        i  = 0
        hf = ''
        #Search in system paths
        while i < len(incs):
            if(os.path.exists(incs[i]) == False):
                i += 1
                continue
            hf = incs[i] + '/' + headerfile
            if os.path.isfile(hf):
                return hf
            else:
                i += 1
                continue

        return ""

    # -----------------------------------------------------------------------------
    # needRebuild() - Check source file and target file date, judge whether need 
    # rebuild
    # parameter:
    #     None
    # ----------------------------------------------------------------------------- 
    def needRebuild(self, src, target):
        if not os.path.exists(src):
            print "Source file %s is nost exist!!!" %(src)
            return False

        defstr = self.OUT_PATH + '/' + target.replace('.cpp', '_def.cpp') 
        wrpstr = self.OUT_PATH + '/' + target.replace('.cpp', '_mtkwrp.cpp')

        if os.path.exists(defstr) or os.path.exists(wrpstr):
            src_statinfo = os.stat(src)
            tartget_statinfo = ""
            if os.path.exists(defstr):
                target_statinfo = os.stat(defstr)
            else:
                target_statinfo = os.stat(wrpstr)
            src_mtime = src_statinfo.st_mtime
            target_mtime = target_statinfo.st_mtime
            if src_mtime > target_mtime:
                return True
            else:
                return False
        else:
            return True


    # -----------------------------------------------------------------------------
    # mtkCheck() - Check whether a module with MediaTek change non-ndk issue
    # parameter:
    #     None
    # -----------------------------------------------------------------------------  
    def mtkCheck(self):
        self.nonNdkIssue = False
        self.nonNdkClass = {}
        for name in self.cppHeader.classes: 
            c = self.cppHeader.classes[name]

            #Bypass new class add by MediaTek
            if c["mtkChange"]:
                if nonndk_debug: print "@@@Class %s is new add by MediaTek" %(c["name"])
                continue
            

            if nonndk_debug: print "\t%s::Method\t"%name
            functype = "virtual"
            self.nonNdkClass[c["name"]] = False
            
            for pub in c["methods"]["public"]:
                if(functype in pub["rtnType"] and pub["mtkChange"]):
                    if nonndk_debug: print "Public Mediatek virtual methods %s " %(pub["name"])
                    self.nonNdkIssue = True
                    self.nonNdkClass[c["name"]] = True
    
            for pro in c["methods"]["protected"]:
                if(functype in pro["rtnType"] and pro["mtkChange"]):
                    if nonndk_debug: print "Protected Mediatek virtual methods %s " %(pro["name"])
                    self.nonNdkIssue = True
                    self.nonNdkClass[c["name"]] = True

            for pri in c["methods"]["private"]:
                if(functype in pri["rtnType"] and pri["mtkChange"]):
                    if nonndk_debug: print "Private Mediatek virtual methods %s " %(pri["name"])
                    self.nonNdkIssue = True
                    self.nonNdkClass[c["name"]] = True

    
            if nonndk_debug: print "\t%s::Property\t"%name
            protype = "static"
            for pub in c["properties"]["public"]:
                if(protype not in pub["type"] and pub["mtkChange"]):
                    if nonndk_debug: print "Non static public Mediatek properties %s " %(pub["name"])
                    self.nonNdkIssue = True
                    self.nonNdkClass[c["name"]] = True

            for pro in c["properties"]["protected"]:
                if(protype not in pro["type"] and pro["mtkChange"]):
                    if nonndk_debug: print "Non static protected Mediatek properties %s " %(pro["name"])
                    self.nonNdkIssue = True
                    self.nonNdkClass[c["name"]] = True
    
            for pri in c["properties"]["private"]:
                if(protype not in pri["type"] and pri["mtkChange"]):
                    if nonndk_debug: print "Non static private Mediatek properties %s " %(pri["name"])
                    self.nonNdkIssue = True
                    self.nonNdkClass[c["name"]] = True

            # Keep all non ndk issue class to the list  self.totalNdkClass
            if self.nonNdkClass[c["name"]]: self.totalNdkClass.append(c["name"])
    
    # -----------------------------------------------------------------------------
    # outputHeaderFile() - Output cpp header file which just with Android default code
    # parameter:
    #     None
    # -----------------------------------------------------------------------------  
    def outputHeaderFile(self):
        if self.nonNdkIssue:
            deffile = self.OUT_PATH + '/' + os.path.basename(self.headerfile)[:-2] + '_def.h'
            fout = file(deffile, 'w')
            fin  = file(self.headerfile)

            bypass = False
            classbypass = False
            defs = ""
            precompcnt = 0
            ctrlmacro = ""
            while True:
                line = fin.readline()
                if len(line) == 0:
                    break
                
                llstrip = line.lstrip()
                if(llstrip.startswith("#ifndef") or llstrip.startswith("#ifdef") or llstrip.startswith("#if")):
                    if self.MACRO in line:
                        if not bypass:
                            if llstrip.startswith("#ifndef" or llstrip.startswith("ifdef")):
                                defs = llstrip[:7].split()
                            else:    
                                if "!defined" in line:
                                    defs = '#ifndef'
                                else:
                                    defs = '#ifdef'

                            if("#ifndef" in defs): 
                                bypass = True
                            else:
                                bypass = False
                            continue
                    if(defs != ""): precompcnt = precompcnt + 1
                elif(not precompcnt and llstrip.startswith("#else")):
                    if("#ifndef" in defs and bypass == True):
                        bypass = False
                        continue
                    elif("#ifdef" in defs and bypass == False):
                        bypass = True
                        continue
                elif(llstrip.startswith("#endif")):
                    if(precompcnt > 0): 
                        precompcnt = precompcnt - 1
                        continue
                    elif("#ifndef" in defs or "#ifdef" in defs): 
                        bypass = False
                        defs = ""
                        precompilecnt = 0
                        continue
                
                if self.isNamespaceAndr(line):
                    filename =  os.path.basename(self.headerfile)[:-2]
                    for key,value in self.nonNdkClass.items():
                        if value:
                            #string =  '#include "%s.h" \n'  %(filename)
                            string = '#ifdef %s \n' %(key)
                            string += '#undef %s \n' %(key)
                            string += '#endif \n'
                            string += '#define %s    %s \n\n' %(key, key)
                    fout.write(string)

                if ctrlmacro == "":
                    ctrlre = re.compile(r'\s*\#ifndef\s+(?P<ctrlmacro>([a-zA-Z_0-9]*))')
                    m = ctrlre.match(line)
                    if m: ctrlmacro = m.group('ctrlmacro')

                if ctrlmacro and ctrlmacro in line:
                    line = line.replace(ctrlmacro,ctrlmacro+'_DEF')

                if not bypass: fout.write(line)

            fout.close()
            fin.close()

            #bypass the class definition with no nonndk issue
            self.bypassDefinition(deffile)

            linkname = os.path.dirname(self.headerfile) + '/' + os.path.basename(deffile)
            self.createFileSoftLink(linkname, deffile)

    
    # -----------------------------------------------------------------------------
    # outputAndrDefCppFile() - Output cpp source file just with Android default code
    # parameter:
    #     None
    # -----------------------------------------------------------------------------  
    def outputAndrDefCppFile(self):
        if self.nonNdkIssue:

            oldcppfile = ""
            newcppfile = ""
            cpp = os.path.basename(self.headerfile)[:-2] + ".cpp"
            hf  = os.path.basename(self.headerfile)

            prog = re.compile(cpp)
            srcs = self.LOCAL_SRC_FILES.split()
            for src in srcs:
                if(prog.search(src) != None):
                    oldcppfile = self.LOCAL_PATH + "/" + src
                    newcppfile = self.OUT_PATH + "/" + cpp[:-4] + "_def.cpp"
                    break

            if(oldcppfile == "" or newcppfile == ""):
                print "****** Can't find cpp file ******"
                return
            
            fcin  = file(oldcppfile)
            fcout = file(newcppfile,"w")
            if not fcin: print "Open file %s for read failed!"%(self.headerfile[:-2]+".cpp")
            if not fcout: print "Open file %s for write failed!"%self.headerfile[:-2]+"_def.cpp"
            
            defs = ""
            bypass = False
            classbypass = False
            classname = ""
            precompcnt = 0
            
            strings = ""
            incstr = ""
            while True:
                line = fcin.readline()
                if len(line) == 0:
                    break
                if line.lstrip().startswith("#include"):
                    if re.search('"%s"'%(hf), line) or re.search('/%s'%(hf),line):
                        name = hf[:-2]
                        fcout.write(line) 
                        """ Add new include header file classname_def.h """
                        incstr = line.replace(name, name+"_def")
                        continue
                llstrip = line.lstrip()
                if(llstrip.startswith("#ifndef") or llstrip.startswith("#ifdef") or llstrip.startswith("#if")):
                    if(self.MACRO in line):
                        if not bypass: 
                            if llstrip.startswith("#ifndef" or llstrip.startswith("#ifdef")):
                                defs = llstrip[:7].split()
                            else:    
                                if line.find("!defined"):
                                    defs = '#ifndef'
                                else:
                                    defs = '#ifdef'

                            if("#ifndef" in defs): 
                                bypass = True
                            else:
                                bypass = False
                            continue    
                    if(defs != ""):
                        precompcnt = precompcnt + 1
                elif(not precompcnt and llstrip.startswith("#else")):
                    if("#ifndef" in defs and bypass == True):
                        bypass = False
                        continue
                    elif("#ifdef" in defs and bypass == False):
                        bypass = True
                        continue
                elif(llstrip.startswith("#endif")):
                    if(precompcnt > 0): 
                        precompcnt = precompcnt - 1
                        continue
                    elif("#ifndef" in defs or "#ifdef" in defs): 
                        bypass = False
                        defs = ""
                        precompcnt = 0 
                        continue

                if self.isNamespaceAndr(line):
                    for key,value in self.nonNdkClass.items():
                        if value:
                            if incstr: string = incstr
                    fcout.write(string)

                if not bypass: fcout.write(line)

            fcin.close()
            fcout.close()

            
            #bypass the class definition with no nonndk issue
            self.bypassDefinition(newcppfile)

            #bypass the class implementation with no nonndk issue
            self.bypassImplement(newcppfile)
            
    # -----------------------------------------------------------------------------
    # outputMtkTransHeaderFile() - Output MTK wrapper header file  
    # parameter:
    #     None
    # -----------------------------------------------------------------------------  
    def outputMtkTransHeaderFile(self):
        if(self.nonNdkIssue):
            oldfile = os.path.basename(self.headerfile)
            oldfile = self.OUT_PATH + "/" + oldfile[:-2] + "_def.h"

            if not os.path.exists(oldfile):
                print "Not exist file %s " %(oldfile)
                return
            
            newfile = oldfile.replace("def", "mtkwrp")

            fin = file(oldfile)
            fout = file(newfile, "w")
            if not fin: print "Open file %s for read failed!" %(oldfile)
            if not fout: print "Open file %s for write failed!" %(newfile)

            definestr = ""
            while True:
                line = fin.readline()
                if len(line) == 0:
                    break

                if line.startswith("#ifndef") and definestr == "":
                    definestr = line.split()[1]

                if self.isNamespaceAndr(line):
                    fout.write(line + '\n')
                    fout.write(self.createMtkwrpHeaderFile(fin.read()))
                    fout.write("\n};\n")
                    namespacestr = "\n}  // namespace android \n"
                    fout.write(namespacestr)
                    if definestr: fout.write("\n#endif //" + definestr)
                    break
                else:
                    fout.write(line)

            fin.close()
            fout.close()

            linkname = os.path.dirname(self.headerfile) + '/' + os.path.basename(oldfile)
            if os.path.islink(linkname):
                os.system("rm -rf %s" %(linkname))
                os.remove(oldfile)
                linkname = linkname.replace("_def","_mtkwrp")
                self.createFileSoftLink(linkname, newfile)





    # -----------------------------------------------------------------------------
    # outputMtkTransCppFile() - Output MTK wrapper cpp source file  
    # parameter:
    #     None
    # -----------------------------------------------------------------------------  
    def outputMtkTransCppFile(self):
        if(self.nonNdkIssue): 
            headerfile = os.path.basename(self.headerfile)

            oldcppfile = self.OUT_PATH + "/" + headerfile[:-2] + "_def.cpp"
            self.cppfile = oldcppfile
            if not os.path.exists(oldcppfile):
                print "Not exist file %s " %(oldcppfile)
                return
            
            newcppfile = oldcppfile.replace("def","mtkwrp")
 
            fcin  = file(oldcppfile)
            fcout = file(newcppfile,"w")
            if not fcin: print "Open file %s for read failed!" %(oldcpfile)
            if not fcout: print "Open file %s for write failed!" %(newcppfile)
            
            while True:
                line = fcin.readline()
                if len(line) == 0:
                    break

                if headerfile[:-2]+'_def.h' in line:
                    fcout.write("using namespace android; \n")
                    string = ""
                    for key,value in self.nonNdkClass.items():
                        if value:
                            c = self.cppHeader.classes[key]
                            string += self.classGlobalContent(c["name"])
                    if string: fcout.write(string)
                    line = line.replace("_def","_mtkwrp")
                    fcout.write(line + '\n')
                    fcout.write("namespace android { \n\n")
                    fcout.write(self.createMtkTransContent())
                    namespacestr = "\n}  // namespace android \n"
                    fcout.write(namespacestr)
                    break
                else:
                    fcout.write(line)
            fcin.close()
            fcout.close()
            
            #remove file classname_def.cpp
            os.remove(oldcppfile)
 
    def process(self):
        
        path = self.LOCAL_PATH + '/' + t_NONNDK_MODULES
        if not os.path.exists(path):
            self.createModulesFile(path)
        
        f = file(path)
        if not f: 
            print "Open file %s for read failed!, Create new default one" %(path)

        pre_modules = {}
        pro = []
        while True:
            line = f.readline()
            if(len(line) == 0):
                break
            if(line.lstrip().startswith('#')):
                continue
            pro = line.split()
            if(len(pro) < 2):
                break
            if pro[0][-4:] == ".cpp": pro[0] = pro[0][:-4]
            pre_modules[pro[0]] = pro[1]
        
        for m in self.modules: 
            if(pre_modules.has_key(m) and pre_modules[m] != t_NONNDK_OFF):
                if nonndk_debug: print "Module %s in pre-protected modlues list" %(m)
                
                cpp = self.moduleToSrc(m)
                if cpp == '':
                    continue
                
                hf  = self.getHeaderFileByCpp(cpp)
                if hf == '':
                    continue
                
                if nonndk_debug: print "*** header file: %s ***" %(hf)
                #parse header file for class information
                self.parseHeaderFile(hf) 
               
                #Check mediatek modification with non-ndk issue
                self.mtkCheck()
                 
                if self.nonNdkIssue and self.needRebuild(hf, cpp):
                    if nonndk_debug: print "\t@@@ Head file %s with non-Ndk issue!!!" %(hf)
                    #output header and cpp files
                    self.outputHeaderFile()
                    self.outputAndrDefCppFile()
                    
                    if pre_modules[m] == t_MEDIATEK_WRAPPER:
                        self.outputMtkTransCppFile()
                        self.outputMtkTransHeaderFile()
        
        #Create cflags string for redefine class xxx to xxx_mtk into global CFLAGS
        #self.createCflagsStr()
        self.createNonndkHeaderFile()
        f.close()
  

def main(argv):
    srcs = ''
    incs = ''
    out = ''
    if(len(argv)<4):
        print '#error, parameter is not enough'
        sys.exit()
    srcs = argv[1][0:]
    if nonndk_debug: print 'srouce files list:'
    if nonndk_debug: print srcs

    incs = argv[2][0:]
    if nonndk_debug: print 'include files list:'
    if nonndk_debug: print incs

    local = argv[3][0:]
    if nonndk_debug: print 'local path:'
    if nonndk_debug: print local

    out = argv[4][0:]
    if nonndk_debug: print 'output directory:'
    if nonndk_debug: print out

    nonNdkClass(srcs, incs, local, out)

if __name__ == '__main__':
    t1 = datetime.datetime.now()
    main(sys.argv)
    t2 = datetime.datetime.now()
    print "Took:", t2 - t1
