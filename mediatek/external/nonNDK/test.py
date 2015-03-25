#!/usr/bin/python
# nonNDK issues protect module - Jan 11 2012
# This module will parse all the module header files and check mediatek code change,
# apply the protect machnism if mediatek's chang with nonNDK issues.
# By Liwen Tan, liwen.tan@mediatek.com
# License: MediaTek

import os
import sys
import re
import datetime

sys.path = ["../"] + sys.path
import CppHeaderParser

t_ANDROID_DEFAULT_CODE = r'ANDROID_DEFAULT_CODE'

def headerfile_search(base=".", circle=True):
    if base == ".":  
        base = os.getcwd()
        
    flist = []  
    if not os.path.exists(base) or os.path.islink(base): return flist
    if "out/target/product" in base or "out/host/" in base: return flist
    if "abi/" in base or "bionic/" in base or "bootable/" in base or "build/" in base: return flist
    if "dalvik/" in base or "packages" in base or "kernel/" in base or "vendor" in base: return flist

    cur_list = os.listdir(base)
    for item in cur_list:  
        path = os.path.join(base, item)
        if os.path.isfile(path) and "frameworks/base" in path:  
            if path.endswith('.h') or path.endswith('.hpp'):
                if t_ANDROID_DEFAULT_CODE in open(path).read():
                    flist.append(path)  
        else:
            if os.path.isdir(path): flist += headerfile_search(path)  
    return flist  


def parseHeaderFile(filename):
    cppHeader = CppHeaderParser.CppHeader(filename)
    return cppHeader

def nonNdkIssueCheck(filename):
    nonNdkIssue = False
    cppHeader = parseHeaderFile(filename)
    for name in cppHeader.classes: 
        c = cppHeader.classes[name]

        #Bypass new class add by MediaTek
        if c["mtkChange"]:
              continue
            
        #print "\t%s::Method\t"%name
        functype = "virtual"    
        for pub in c["methods"]["public"]:
            if(functype in pub["rtnType"] and pub["mtkChange"]):
                #print "Public Mediatek virtual methods %s " %(pub["name"])
                nonNdkIssue = True
    
        for pro in c["methods"]["protected"]:
            if(functype in pro["rtnType"] and pro["mtkChange"]):
                nonNdkIssue = True

        for pri in c["methods"]["private"]:
            if(functype in pri["rtnType"] and pri["mtkChange"]):
                #print "Private Mediatek virtual methods %s " %(pri["name"])
                nonNdkIssue = True
    
        #print "\t%s::Property\t"%name
        protype = "static"
        for pub in c["properties"]["public"]:
            if(protype not in pub["type"] and pub["mtkChange"]):
                #print "Non static public Mediatek properties %s " %(pub["name"])
                nonNdkIssue = True

        for pro in c["properties"]["protected"]:
            if(protype not in pro["type"] and pro["mtkChange"]):
                #print "Non static protected Mediatek properties %s " %(pro["name"])
                nonNdkIssue = True
    
        for pri in c["properties"]["private"]:
            if(protype not in pri["type"] and pri["mtkChange"]):
                #print "Non static private Mediatek properties %s " %(pri["name"])
                nonNdkIssue = True
        
    return nonNdkIssue

def main(argv):
    search_path = "../../../../../"
    headerfiles = headerfile_search(search_path)
    for h in headerfiles:
        if nonNdkIssueCheck(h):
            print h

if __name__ == '__main__':
    t1 = datetime.datetime.now()
    main(sys.argv)
    t2 = datetime.datetime.now()
    print "Took:", t2 - t1
