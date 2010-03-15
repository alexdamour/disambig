import pickle

f = open('ratios.p')
r = pickle.load(f)
f.close()

Pr_M = 0.1

#print '\n'.join([str((k,v)) for k,v in r.iteritems() if v > 0])

l = [(k,1/(1+(1-Pr_M)/(Pr_M*v))) for k,v in r.iteritems() if 1/(1+(1-Pr_M)/(Pr_M*v)) > 0.3]
l.sort(key = lambda i: i[1])
print '\n'.join(["%s: %g"%x for x in l])
