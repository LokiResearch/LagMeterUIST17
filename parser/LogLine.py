# -*- coding: iso-8859-1 -*-
#
# LogLine.py --
#
# Authors: Nicolas Roussel
#
# Copyright © Inria

import datetime

class ANSI:
    # From http://stackoverflow.com/questions/287871/print-in-terminal-with-colors-using-python
    BLACK = '\033[30m'
    BLUE = '\033[94m'
    RED = '\033[91m'
    GREEN = '\033[92m'
    LIGHTGREY = '\033[37m'    
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    ENDC = '\033[0m'

def timestamp2datetime(timestamp, unit=1e-9): # nanoseconds
    return datetime.datetime.fromtimestamp(timestamp*unit)
    
class LogLine(object):
    
    obspoints = [
        "piezo",
        "libpointing",
        "system",
        "toolkit",
        "repaint",
        "screen",        
        "photodiode"
    ]
   
    def __init__(self, linenumber, line):
        self.linenumber = linenumber
        self.block = None
        words = line.split()
        self.timestamp, self.obspoint, args = int(words[0]), words[1], words[2:]
        self.xtra = []
        if self.obspoint=="leonardo":
            # args: nbbytes 0 0 src t0 t1 t2 t3
            nbbytes = int(args[0])
            msglen = len(args)-1
            if msglen!=nbbytes:
                raise ValueError("Bad leonardo message length (%d) at time %s"%(msglen, words[0]))
            self.obspoint = {"D":"photodiode", "P":"piezo"}.get(chr(int(args[3])), "leonardo")
            self.event_timestamp = ((int(args[4]) << 24) + (int(args[5]) << 16) + (int(args[6]) << 8) + int(args[7])) * 1e3 # microseconds to nanoseconds
            self.xtra = args[1+nbbytes:]
        elif self.obspoint=="libpointing":
            # args: event_timestamp
            self.event_timestamp = int(args[0])
            if len(args)>1: self.xtra = args[3:] # Obsolete format: event_timestamp dx,dy buttons
        elif self.obspoint=="system":
            # args: x,y event_timestamp
            self.x, self.y = map(float, args[0].split(","))
            self.event_timestamp = int(args[1])            
            self.xtra = args[2:]
        elif self.obspoint=="toolkit":
            # args: x,y
            #self.x, self.y = map(float, args[0].split(","))
            self.x = 0
            self.y = 0
            self.xtra = args[1:]
        elif self.obspoint=="repaint":
            pass
        elif self.obspoint=="screen":
            pass
        # -----------------------------------------------------------------------
        # OBSOLETE STUFF TRANSLATED TO NEW STUFF
        #
        elif self.obspoint=="sysPointer":
            # args: timestamp x,y            
            self.obspoint = "system"
            self.event_timestamp = int(args[0])
            self.x, self.y = map(float, args[1].split(","))
            self.xtra = args[2:]            
        elif self.obspoint=="tkMouseButtonPress":
            # args: button x,y
            self.obspoint = "toolkit"
            if args:
                self.x, self.y = map(float, args[1].split(","))
                self.xtra = args[2:]
            else:
                self.x = self.y = float("nan")
        # -----------------------------------------------------------------------                
        else:
            raise ValueError("Unknown observation point '%s' at line %d"%(words[1], linenumber))
    
    def __str__(self):
        timestamp = timestamp2datetime(self.timestamp).isoformat()
        if self.block:
            prefix = ANSI.BLACK
        else:
            prefix = ANSI.LIGHTGREY
        suffix = ANSI.ENDC
        if self.obspoint=="piezo":
            prefix = ANSI.RED
            seconds = self.event_timestamp/1e9
            return "%s%s [%s %20s %s]%s"%(prefix, self.linenumber, timestamp, self.obspoint, seconds, suffix)
        elif self.obspoint=="libpointing":
            event_timestamp = timestamp2datetime(self.event_timestamp).isoformat()
            return "%s%s [%s %20s %s]%s"%(prefix, self.linenumber, timestamp, self.obspoint, event_timestamp, suffix)
        elif self.obspoint=="system":
            event_timestamp = timestamp2datetime(self.event_timestamp).isoformat()
            return "%s%s [%s %20s x=%s y=%s %s]%s"%(prefix, self.linenumber, timestamp, self.obspoint, self.x, self.y, event_timestamp, suffix)            
        elif self.obspoint=="toolkit":
            return "%s%s [%s %20s x=%s y=%s]%s"%(prefix, self.linenumber, timestamp, self.obspoint, self.x, self.y, suffix)
        elif self.obspoint=="repaint":
            return "%s%s [%s %20s]%s"%(prefix, self.linenumber, timestamp, self.obspoint, suffix)
        elif self.obspoint=="screen":
            return "%s%s [%s %20s]%s"%(prefix, self.linenumber, timestamp, self.obspoint, suffix)
        elif self.obspoint=="photodiode":
            prefix = ANSI.BLUE
            seconds = self.event_timestamp/1e9
            return "%s%s [%s %20s %s]%s"%(prefix, self.linenumber, timestamp, self.obspoint, seconds, suffix)
        else:
            return "%s%s [??? %20s %s ???]%s"%(prefix, self.linenumber, timestamp, self.obspoint, suffix)
