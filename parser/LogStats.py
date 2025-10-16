# -*- coding: iso-8859-1 -*-
#
# LogFile.py --
#
# Authors: Nicolas Roussel
#
# Copyright © Inria

import numpy

def mad(arr):
    """ Median Absolute Deviation: a "Robust" version of standard deviation.
        Indices variabililty of the sample.
        https://en.wikipedia.org/wiki/Median_absolute_deviation 
    """
    arr = numpy.ma.array(arr).compressed() # should be faster to not use masked arrays.
    arr = arr[~numpy.isnan(arr)]
    med = numpy.median(arr)
    return numpy.median(numpy.abs(arr - med))

class LogStats(object):
    
    def __init__(self, blocks, measures):
        self.blocks = blocks
        self.measures = []
        self.minvalue, self.maxvalue = 1e4, -1
        self.values, self.minvalues, self.maxvalues = {}, {}, {}
        self.median, self.mean, self.mad, self.std = {}, {}, {}, {}
        for m in measures: self.__computeStatsFor(m, blocks)

    def __computeStatsFor(self, measure, blocks):
        data = [block.lag.get(measure, numpy.nan) for block in blocks]
        if numpy.isnan(data).all(): return
        self.measures.append(measure)
        self.values[measure] = data            
        self.minvalues[measure] = minvalue = min(data)
        self.maxvalues[measure] = maxvalue = max(data)
        self.median[measure] = numpy.nanmedian(data)            
        self.mean[measure] = numpy.nanmean(data)
        self.std[measure] = numpy.nanstd(data)
        self.mad[measure] = mad(data)
        self.minvalue = min(self.minvalue, minvalue)
        self.maxvalue = max(self.maxvalue, maxvalue)                        

    def getTitle(self, measure):
        return "min=%3.3f max=%3.3f median=%3.3f mean=%3.3f mad=%02.3f std=%3.3f"%(self.minvalues[measure],
                                                                                   self.maxvalues[measure],
                                                                                   self.median[measure],
                                                                                   self.mean[measure],
                                                                                   self.mad[measure],
                                                                                   self.std[measure])
