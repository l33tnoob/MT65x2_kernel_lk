#!/usr/bin/env python

"""
Usage: ale-as-extract.py [-o output_file] [input_files...]

-d turn on debug option
-h to display this usage message and exit.
-o set output file
-r reserve log message content with output
"""

import base64,ctypes,ctypes.util,sys,getopt,re,hashlib

ale_rec_label_fmt_arg_max = 15

ale_rec_label_pattern = "(.*____ale_rec____.*):$"
ale_rec_label_tag_pattern = "^\t.word\t(\.\w+|0)$"
ale_rec_label_fmt_pattern = "^\t.word\t((\.\w+)|([\w\._]*__FUNCTION__[\w\._]*))$"
ale_rec_label_filename_pattern = "^\t.word\t(\.\w+)$"

xlog_ale_rec_label_pattern = "(.*____xlog_ale_rec____.*):$"
xlogk_ale_rec_label_pattern = "(.*____xlogk_ale_rec____.*):$"
xlog_ale_rec_tag_level_pattern = "^\t.word\t(\d+)$"
xlog_tag_level_enum = {
    "0": "unknown",
    "2": "verbose",
    "3": "debug",
    "4": "info",
    "5": "warn",
    "6": "error",
    "7": "fatal"
};
ale_rec_label_hash_template = "\t.word\t0x%s\n"
ale_rec_zero_word_template = "\t.word\t0\n"
ale_rec_label_fmt_args_template = "\t.ascii\t\"%s\\000\"\n"
ale_rec_label_fmt_args_space_template = "\t.space\t%d\n"

ale_string_label_pattern = "((\.LC\d+)|([\w\._]*__FUNCTION__[\w\._]*)):$"
ale_string_used_pattern = "\.LC\d+|[\w\._]*__FUNCTION__[\w\._]*"
ale_string_content_pattern = "\t.ascii\t\"(.*)\"$"

# refrence http://en.wikipedia.org/wiki/Printf
printf_template_p = re.compile("%(?:[0-9]+\$)?[0-9+\-#]*(?:\.[0-9]+|\.\*)?((?:(?:h|hh|l|ll|z|j|t)?[diuxXo]|[scp]|L?[fFeEgG]))")

# set true to turn debug log
opt_debug_enabled = False
# Set true to not remove fmt string
opt_no_remove_string = False
 
def ale_die(msg):
    print "%s\n" % msg
    sys.exit(1)

def ale_warning(msg):
    sys.stderr.write("%s\n" % msg)

class ALERawRec:
    __slot__ = ["layer", "level", "tag_id", "fmt_id", "filename_id", "lineno"]

    def __init__(self, layer, level, tag_id, fmt_id, filename_id):
        self.layer = layer
        self.level = level
        self.tag_id = tag_id
        self.fmt_id = fmt_id
        self.filename_id = filename_id

class ALERecExtractor:
    _raw_records = {}
    _messages = {}
    _records = {}

    _message_id_used = set();
    _message_id_removed = set();

    _rec_label_p = None
    _rec_label_tag_p = None
    _rec_label_fmt_p = None
    _rec_label_filename_p = None

    _string_label_p = None
    _string_used_p = None
    _string_content_p = None

    _libc = None

    def __init__(self):
        self._rec_label_p = re.compile(ale_rec_label_pattern)
        self._rec_label_tag_p = re.compile(ale_rec_label_tag_pattern)
        self._rec_label_fmt_p = re.compile(ale_rec_label_fmt_pattern)
        self._rec_label_filename_p = re.compile(ale_rec_label_filename_pattern)

        self._xlog_rec_label_p = re.compile(xlog_ale_rec_label_pattern)
        self._xlogk_rec_label_p = re.compile(xlogk_ale_rec_label_pattern)
        self._xlog_rec_tag_level_p = re.compile(xlog_ale_rec_tag_level_pattern)

        self._string_label_p = re.compile(ale_string_label_pattern)
        self._string_used_p = re.compile(ale_string_used_pattern);
        self._string_content_p = re.compile(ale_string_content_pattern)

        # 
        cpath = ctypes.util.find_library("c")
        self._libc = ctypes.CDLL(cpath)

    def parse_printf_format(self, fmt):
        arg_string = ""

        args = (ctypes.c_int * ale_rec_label_fmt_arg_max)(0)
        argcount = self._libc.parse_printf_format(fmt, ale_rec_label_fmt_arg_max, ctypes.byref(args))
        if (argcount > ale_rec_label_fmt_arg_max):
            return None

        string_args = printf_template_p.findall(fmt)
        if argcount != len(string_args):
            return None
        
        for i in range(0, argcount):
             flags = args[i] & 0xff00
             arg = args[i] & 0x00ff

             if arg == 0:   # 32 bit integer
                 if string_args[i][0:2] != "ll":
                     arg_string += "d"
                 else:
                     arg_string += "l"
             elif arg == 1:   # 32 bit integer, cast to char
                 arg_string += "c"
             elif arg == 3: # C string
                 arg_string += "s"
             elif arg == 5: # C pointer
                 arg_string += "p"
             elif arg == 7: # 64 bit double
                 arg_string += "e"
             else:
                 ale_die("printf parser return Unknown code %d with string '%s'" % (arg, fmt));

        return arg_string

    def define_label_found(self, it, label, layer):
        line = it.next()
        tag_result = self._rec_label_tag_p.match(line)
        if tag_result == None:
            ale_die("ALE record '%s' tag string parse failed \"%s\"" % (label, line))

        line = it.next()
        fmt_result = self._rec_label_fmt_p.match(line)

        if fmt_result == None:
            ale_die("ALE record '%s' fmt parse failed \"%s\"" % (label, line))

        line = it.next()
        filename_result = self._rec_label_filename_p.match(line)
        if filename_result == None:
            ale_die("ALE record '%s' filename parse failed" % label)
        
        # The level/lineno field is shared
        line = it.next()
        taglevel_result = self._xlog_rec_tag_level_p.match(line)
        if taglevel_result == None:
            ale_die("ALE record tag level parse failed %s" % taglevel_result)
        taglevel_str = taglevel_result.group(1)
        if taglevel_str not in xlog_tag_level_enum:
            ale_die("ALE record encounter invalid tag level %s\n" % taglevel_str)
        
        self._raw_records[label] = ALERawRec(layer, xlog_tag_level_enum[taglevel_str], tag_result.group(1), fmt_result.group(1), filename_result.group(1))

        # Sometime, programmer call log and assign tag/fmt same string
        if tag_result.group(1) != "0":
            self._message_id_used.add(tag_result.group(1))

    def define_string_found(self, it, label):
        fmt_string = ""
        line = it.next()
        result = self._string_content_p.match(line)
        while result != None:
            fmt_string = fmt_string + result.group(1)
            line = it.next()
            result = self._string_content_p.match(line)
        self._messages[label] = fmt_string
        return line

    def readfile(self, lines):
        it = iter(lines)
        try:
            next_line = None
            while True:
                if next_line == None:
                    line = it.next()
                else:
                    line = next_line
                    next_line = None
                if not line:
                    break

                # Process ALE record
                result = self._rec_label_p.match(line)
                if result != None:
                    self.define_label_found(it, result.group(1), "log-native")
                else:
                    result = self._xlog_rec_label_p.match(line)
                    if result != None:
                        self.define_label_found(it, result.group(1), "xlog-native")
                    else:
                        result = self._xlogk_rec_label_p.match(line)
                        if result != None:
                            self.define_label_found(it, result.group(1), "xlog-kernel")
                        else:
                            # Record all assemble strings
                            result = self._string_label_p.match(line)
                            if result != None:
                                next_line = self.define_string_found(it, result.group(1))
                            else:
                            #Check if string is reference by non-ale used
                                for string_id in self._string_used_p.findall(line):
                                    self._message_id_used.add(string_id)

        except StopIteration:
            pass

        for rec_id, v in self._raw_records.iteritems():
            try:
                if v.tag_id != "0":
                    tag = self._messages[v.tag_id]
                else:
                    tag = ""
                fmt = self._messages[v.fmt_id]
                fmt_args = self.parse_printf_format(fmt)
                
                filename = self._messages[v.filename_id]
                
                if fmt_args != None:
                    h = hashlib.md5()
                    h.update(tag)
                    h.update(fmt)

                    if (v.fmt_id not in self._message_id_used) and (not opt_no_remove_string):
                        self._message_id_removed.add(v.fmt_id)
                    # self._message_id_removed.add(v.filename_id)
                    self._records[rec_id] = h.hexdigest(), tag, fmt, fmt_args, v.level, v.layer, filename
                else:
                    ale_warning("'%s' has too many argument in message string '%s'" % (rec_id, fmt))
            except KeyError:
                ale_die("ALE record '%s' can't find tag/fmt" % rec_id);
        
    def rec_exist(self, rec_id):
        return rec_id in self._records

    def get_rec_hash(self, rec_id):
        message = self._records[rec_id]
        return message[0][0:8]

    def get_rec_fmt_args(self, rec_id):
        message = self._records[rec_id]
        return message[3]

    def is_message_to_be_removed(self, string_id):
        return string_id in self._message_id_removed

    def debug_dump(self):
        sys.stderr.write("ALE converted: %d, Removed string:%d\n" % (len(self._records), len(self._message_id_removed)));

class ALERecUpdator:
    _extractor = None
    _output_f = None
    def __init__(self, extractor):
        self._extractor = extractor


    def define_rec_found(self, it, label):
        self._output_f.write("%s:\n" % label)
        if not self._extractor.rec_exist(label):
            return

        tag_line = it.next()
        tag_result = self._extractor._rec_label_tag_p.match(tag_line)
        if tag_result != None:
            fmt_line = it.next()
            fmt_result = self._extractor._rec_label_fmt_p.match(fmt_line)
            if fmt_result != None:
                self._output_f.write(tag_line)
                
                if not(opt_no_remove_string):
                    self._output_f.write(ale_rec_zero_word_template)
                else:
                    self._output_f.write(fmt_line)

                # filename
                line = it.next()
                self._output_f.write(ale_rec_zero_word_template)
                # lineno + level
                line = it.next()
                self._output_f.write(ale_rec_zero_word_template)
                    
                # hash
                line = it.next()
                self._output_f.write(ale_rec_label_hash_template % self._extractor.get_rec_hash(label))

                # embed printf format
                fmt_args = self._extractor.get_rec_fmt_args(label)

                # grok following lines
                #   .ascii "..."
                #   .space #n
                line = it.next()
                line = it.next()

                self._output_f.write(ale_rec_label_fmt_args_template % fmt_args)
                if ale_rec_label_fmt_arg_max != len(fmt_args):
                    self._output_f.write(ale_rec_label_fmt_args_space_template % (ale_rec_label_fmt_arg_max - len(fmt_args)))
            else:
                self._output_f.write(tag_line)
                self._output_f.write(fmt_line)
        else:
            self._output_f.write(tag_line)

    def define_string_found(self, it, label):
        removed = self._extractor.is_message_to_be_removed(label);
        if not removed:
            self._output_f.write("%s:\n" % label) 

        line = it.next()
        result = self._extractor._string_content_p.match(line)
        while result != None:
            if not removed:
                self._output_f.write(line)
            line = it.next()
            result = self._extractor._string_content_p.match(line)
        return line

    def write(self, of, lines):
        self._output_f = of
        it = iter(lines)
        try:
            next_line = None
            while True:
                if next_line == None:
                    line = it.next()
                else:
                    line = next_line
                    next_line = None
                if not line:
                    break

                # Process log/xlog ALE record
                result = self._extractor._rec_label_p.match(line);
                if result == None:
                    result = self._extractor._xlog_rec_label_p.match(line)
                if result == None:
                    result = self._extractor._xlogk_rec_label_p.match(line)
                    
                if result != None:
                    self.define_rec_found(it, result.group(1))
                else:
                    # Process ALE tag/fmt string
                    result = self._extractor._string_label_p.match(line);
                    if result != None:
                        next_line = self.define_string_found(it, result.group(1))
                    else:
                        self._output_f.write(line)

            # Write out ALE section
        except StopIteration:
            pass

        self._output_f.write("\t.section\t.ale_database,\"\",%progbits\n")
        for m,v in self._extractor._records.iteritems():
            self._output_f.write("\t.ascii\t\"%s,%s,%s,%s,%s,%s,%s\\000\"\n" % (v[0], v[5],v[4], base64.b64encode(v[1]), base64.b64encode(v[2]), v[3], v[6]))


if __name__ == "__main__":
    argv = sys.argv
    input_path = None
    output_path = None
    try:
        opts, args = getopt.getopt(argv[1:], "drho:", [])
    except getopt.GetoptError:
        usage(argv[0])
        sys.exit(2)
        
    for opt, arg in opts:
        if opt == "-d":
            opt_debug_enabled = True
        elif opt == "-o":
            output_path = arg
        elif opt == "-r":
            opt_no_remove_string = True
        elif opt == "-h":
            print __doc__
            sys.exit(2)

    if len(args) == 0:
        input_f = sys.stdin
    elif len(args) == 1:
        if args[0] != "-":
            input_f = open(args[0], 'r')
        else:
            input_f = sys.stdin;

    lines = input_f.readlines()
    input_f.close()

    extractor = ALERecExtractor()
    extractor.readfile(lines)

    if opt_debug_enabled:
        extractor.debug_dump()
    
    if output_path != None:
        if output_path != "-":
            output_f = open(output_path, 'w')
        else:
            output_f = sys.stdout

        updator = ALERecUpdator(extractor)
        updator.write(output_f, lines)
        output_f.close()

    sys.exit(0)
