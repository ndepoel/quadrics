#!/usr/bin/python
import os
import re
files = os.listdir("data");
files = [f for f in files if f[-4:] == ".sql"]
#files=  files[0:1]
sphere_re = re.compile("Loaded (\d*) spheres")
frames_re = re.compile(".*avg. fps = (\d*\.\d{2}).*")
fout = open("benchmark_results.csv", "w")
for f in files:
    res = []
    spheres = None
    for mode in ["","d","d o"]:
        out = os.popen("./benchmark data/%s %s" % (f, mode))
        txt = out.read()
        res.append(frames_re.search(txt).groups()[0])
        spheres = sphere_re.search(txt).groups()[0]
    fout.write(",".join([f,spheres]+res)+"\n")
fout.close()
