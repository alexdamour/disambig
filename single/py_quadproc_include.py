import re

f = open("sp.desc")
lines = f.read()
f.close()

simprof_block = re.findall(r"^simprof{.*?^}", lines, re.S | re.M)[0]
print simprof_block
sim_splitfields = [s.strip().split() for s in simprof_block.split('\n')[1:-1] if s.strip()[0] != '%']
sim_groups = {}
for s in sim_splitfields:
    if s[4] in sim_groups:
        sim_groups[s[4]].append(s[0])
    else:
        sim_groups[s[4]] = [s[0],]

res_space_block = re.findall(r"^res_space{.*?^}", lines, re.S | re.M)[0]
res_splitfields = [r.strip().split() for r in res_space_block.split('\n')[1:-1] if r.strip()[0] != '%']
res_levels = dict([(r[0], r[1].split(',')) for r in res_splitfields])

training_block = re.findall(r"^training{.*?^}", lines, re.S | re.M)[0]
train_splitfields = [t.strip().split() for t in training_block.split('\n')[1:-1] if t.strip()[0] != '%']

supports = [t[0] for t in train_splitfields]
print supports
prof_maxes = dict(zip(supports, [map(lambda y: len(res_levels[y])-1, sim_groups[x]) for x in supports]))
print prof_maxes
training_sets = dict([(t[0], (t[1].split(','), t[2].split(','))) for t in train_splitfields])
print training_sets
