#!/usr/bin/env python

"""
Usage: merge-xlog-filter-tags.py [-o output_file] [input_files...]

Merge together zero or more xlog-filter-tags files to produce a single
output file, stripped of comments.  Checks that no tag numbers conflict
and fails if they do.

-d turn on debug option
-f output filter list
-l setting system default level when generate system config files (default level is verbose)
-h to display this usage message and exit
-t output tag list 
"""

import cStringIO
import getopt
import re
import struct
import sys

tag_level_enum = ["verbose", "debug", "info", "warn", "error", "fatal", "on", "off", "default"]

class XlogTag(object):
  __slots__ = ["tagname", "taglevel", "description", "filename", "linenum"]

  def __init__(self, tagname, taglevel, description, filename, linenum):
    self.tagname = tagname
    self.taglevel = taglevel
    self.description = description
    self.filename = filename
    self.linenum = linenum


class XlogTagFile(object):
  """Read an input xlog-filter-tags file."""
  def AddError(self, msg, linenum=None):
    if linenum is None:
      linenum = self.linenum
    self.errors.append((self.filename, linenum, msg))

  def AddWarning(self, msg, linenum=None):
    if linenum is None:
      linenum = self.linenum
    self.warnings.append((self.filename, linenum, msg))

  def AddInfo(self, msg, linenum=None):
    if linenum is None:
      linenum = self.linenum
    self.infos.append((self.filename, linenum, msg))

  def __init__(self, filename, file_object=None):
    """'filename' is the name of the file (included in any error
    messages).  If 'file_object' is None, 'filename' will be opened
    for reading."""

    self.errors = []
    self.warnings = []
    self.infos = []
    self.tags = []

    self.filename = filename
    self.linenum = 0

    if file_object is None:
      try:
        file_object = open(filename, "rb")
      except (IOError, OSError), e:
        self.AddError(str(e))
        return

    try:
      for self.linenum, line in enumerate(file_object):
        self.linenum += 1

        line = line.strip()
        if not line or line[0] == '#': continue
        parts = re.split(r"\s+", line, 2)

        if len(parts) < 2:
          self.AddError("failed to parse \"%s\"" % (line,))
          continue

        tagname = parts[0]
        taglevel = parts[1]
        try:
          tag_level_enum.index(taglevel)
          if len(parts) == 3:
            description = parts[2]
          else:
            description = None
            
          self.tags.append(XlogTag(tagname, taglevel, description,
                                   self.filename, self.linenum))
        except ValueError:
          self.AddError("Unknown tag level \"%s\" at line %d" % (taglevel, self.linenum))
          
    except (IOError, OSError), e:
      self.AddError(str(e))

def WriteOutput(output_file, data):
  """Write 'data' to the given output filename.
  Emit an error message and die on any failure.
  'data' may be a string or a StringIO object."""
  if not isinstance(data, str):
    data = data.getvalue()
  try:
    if output_file is not None:
        out = open(output_file, "wb")
        out.write(data)
        out.close()
  except (IOError, OSError), e:
    print >> sys.stderr, "failed to write %s: %s" % (output_file, e)
    sys.exit(1)

def TransformTag(tag_level):
  if tag_level == "default":
    return None
  if tag_level == "off":
    return "off"
  if tag_level == "on":
    return None

  tag_level_index = tag_level_enum.index(tag_level)

  if default_level != None:
    default_level_index = tag_level_enum.index(default_level)
    if default_level_index >= tag_level_index:
      return None

  return tag_level

infos = []
errors = []
warnings = []

default_level = None
default_filter_file = None
default_tag_list_file = None

try:
  opts, args = getopt.getopt(sys.argv[1:], "hf:l:t:")
except getopt.GetoptError, err:
  print str(err)
  print __doc__
  sys.exit(2)

for o, a in opts:
  if o == "-h":
    print __doc__
    sys.exit(2)
  elif o == "-f":
    default_filter_file = a
  elif o == "-l":
    default_level = a
  elif o == "-t":
    default_tag_list_file = a
  else:
    print >> sys.stderr, "unhandled option %s" % (o,)
    sys.exit(1)

if (default_filter_file is None) and (default_tag_list_file is None):
  print >> sys.stderr, "No output file set"
  sys.exit(1)

if default_level != None:
  try:
    tag_level_enum.index(default_level)
  except ValueError:
    print >> sys.stderr, "Invalid tag level %s" % default_level
    sys.exit(1)

# Restrictions on tags:
#
#   Tag names must be unique.
#

by_tagname = {}

for fn in args:
  tagfile = XlogTagFile(fn)

  for t in tagfile.tags:
    tagname = t.tagname
    taglevel = t.taglevel
    description = t.description

    if t.tagname in by_tagname:
      orig = by_tagname[t.tagname]

      tagfile.AddInfo(
        "tag name \"%s\" override tag %s from %s:%d" %
        (t.tagname, orig.taglevel, orig.filename, orig.linenum),
        linenum=t.linenum)

    by_tagname[t.tagname] = t

  infos.extend(tagfile.infos)
  errors.extend(tagfile.errors)
  warnings.extend(tagfile.warnings)

if errors:
  for fn, ln, msg in errors:
    print >> sys.stderr, "%s:%d: error: %s" % (fn, ln, msg)
  sys.exit(1)

if warnings:
  for fn, ln, msg in warnings:
    print >> sys.stderr, "%s:%d: warning: %s" % (fn, ln, msg)

if infos:
  for fn, ln, msg in infos:
    print >> sys.stderr, "%s:%d: info: %s" % (fn, ln, msg)

tag_list_buffer = cStringIO.StringIO()

default_filter_buffer = cStringIO.StringIO()

olevel = "verbose/debug+verbose/verbose"
if default_level != None:
  olevel = default_level
default_filter_buffer.write("XLOG-FILTER-V2\nALL\t%s\n" % (olevel))

for k in sorted(by_tagname.keys(), key=str.lower):
  t = by_tagname[k]
  if t.description:
    tag_list_buffer.write("%s\t%s\n" % (t.tagname, t.description))
  else:
    tag_list_buffer.write("%s\n" % (t.tagname))
  transform_tag = TransformTag(t.taglevel)
  if transform_tag != None:
    default_filter_buffer.write("TAG\t%s\t%s\n" % (t.tagname, transform_tag))

WriteOutput(default_tag_list_file, tag_list_buffer)
WriteOutput(default_filter_file, default_filter_buffer)
