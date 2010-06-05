#!/usr/bin/python

import cvxopt as co
import cvxopt.solvers
import disambig_db as dd
import itertools
import pickle
import functools
import rpy2.robjects as ro
import numpy
import operator
from py_quadproc_include import *

cvxopt.solvers.options['show_progress'] = False 
cvxopt.solvers.options['maxiters'] = 1000
cvxopt.solvers.options['feastol'] = 1e-8
cvxopt.solvers.options['reltol'] = 1e-10

#supports = ["fname", "dist", "other", "coauths"]
#prof_maxes = {'fname':(5,), 'dist':(7,1), 'other':(5, 5, 4), 'coauths':(11,)}
#training_sets = {'fname': (("tset01_F_yes","tset05_F_yes"), ("xset01_F_no",)),
#                 'dist': (("tset02_F_yes", "tset03_F_yes", "tset05_F_yes"), ("xset03_F_no",)),
#                 'coauths': (("tset02_F_yes", "tset03_F_yes"), ("xset01_F_no", "xset03_F_no")),
#                 'other': (("tset02_F_yes", "tset03_F_yes", "tset05_F_yes"), ("xset01_F_no", "xset03_F_no"))}
   
count = 0

def product(*args, **kwds):
    #From python 2.6 documentation
    #Not very efficient, but fine for our purposes
    pools = map(tuple, args) * kwds.get('repeat', 1)
    result = [[]]
    for pool in pools:
        result = [x+[y] for x in result for y in pool]
    for prod in result:
        yield tuple(prod)

def flatten(l, ltypes=(list, tuple)):
    ltype = type(l)
    l = list(l)
    i = 0
    while i < len(l):
        while isinstance(l[i], ltypes):
            if not l[i]:
                l.pop(i)
                i -= 1
                break
            else:
                l[i:i + 1] = l[i]
        i += 1
    return ltype(l)


def prof_tuples(prof_maxes):
    return list(product(*map(lambda x: range(x,-1,-1), prof_maxes)))

def raw_counts(tset, index, func_num, prof_maxes):
    tuples = prof_tuples(prof_maxes)
    print "Calling:\n\ttset: %s\n\tindex: %s\n\tfunc_num: %d"%(tset,index,func_num)
    tup_count = dd.rawCountTable(tset, index, func_num) 
    for t in tuples:
        if t not in tup_count:
            tup_count[t] = 0
    return tup_count

def raw_ratios(support):
    """
    Collects all of the necessary counts to create the initial values for the quadratic
    program.
    """
    tuples = prof_tuples(prof_maxes[support])
    #print(len(tuples))
    tsets,xsets = training_sets[support]
    #print tsets
    t_idx = map(lambda x: x + "_" + support, tsets)
    #print t_idx
    x_idx = map(lambda x: x + "_" + support, xsets)
    #print x_idx
    tset_dicts = []
    xset_dicts = []
    t_dict = {}
    x_dict = {}

    for t in range(len(tsets)):
        print t
        tset_dicts.append(raw_counts(tsets[t], t_idx[t], supports.index(support), prof_maxes[support]))
        #print len(tset_dicts[t].keys())
        s1 = set(tuples)
        s2 = set(tset_dicts[t].keys())
        #print s1.difference(s2)
        #print sum([v for k,v in tset_dicts[t].iteritems()])
        #seen.extend(tset_dicts[t].keys())

        #print tset_dicts[t]
#    print tuples
    #print tset_dicts
    total = 0
    seen = []
    #print range(len(tset_dicts))
    i = 0
    for td in tset_dicts:
        top_counts = [(k,v) for k,v in td.iteritems() if v > 0]
        top_counts.sort(reverse=True)
        #print top_counts[0:100]
        for k in td.keys():#tuples:
            if t_dict.has_key(k):
                t_dict[k] += td[k]
            else:
                t_dict[k] = td[k]
            #if t_dict[k] == 0:
                #t_dict[k] = 1
    #        total += td[k]
        #if (1,4,1,1) in td:
        #    print "1411_T: %d in test %s"%(td[(1,4,1,1)], tsets[i])
        i += 1

    #print len(seen)

    #print total

    s = set()
    s = s.union(*[t.keys() for t in tset_dicts])
    t = set(tuples)
    #print s.difference(t)
    
    #print sum([v for k,v in t_dict.iteritems()])
    #print t_dict
    

    del tset_dicts

    for x in range(len(xsets)):
        xset_dicts.append(raw_counts(xsets[x], x_idx[x], supports.index(support), prof_maxes[support]))

    for t in tuples:
        for d in range(len(xset_dicts)):
            if t in x_dict:
                x_dict[t] += xset_dicts[d][t]
            else:
                x_dict[t] = xset_dicts[d][t] 
            if x_dict[t] == 0:
                #Avoid divide by zero
                x_dict[t] = 1

    del xset_dicts

    debug_keys = t_dict.keys()
    debug_keys.sort()
#    print [(x, t_dict[x]) for x in debug_keys] 
    t_T = sum(t_dict.values())     
    #t_T = t_T and 1 or t_T
    print "t_T: %d"%t_T
    x_T = sum(x_dict.values())
    #x_T = x_T and 1 or x_T
    print "x_T: %d"%x_T

    ratios = [(t_dict[t]/float(t_T))/(x_dict[t]/float(x_T)) for t in tuples]
    weights = [x_dict[t] + t_dict[t] for t in tuples]
#    print tuples
    return (weights, ratios)

def G_mat(tuples):
    """
    Constructs the constraint matrix enforcing monotonicity, and constraining all of the ratios to be
    greater than zero.
    """
    #tuples = prof_tuples(prof_maxes[support])

    #tc1 = prof_maxes
    #tc2 = tc1
    #I = range(len(tc1))
    #J = range(len(tc1))
    #m = [-1,]*len(tc1)
    
    I=[]
    J=[]
    m=[]

    i=0
    #j=len(tc1)
    j=0

    for tc1 in tuples:
        for e in range(len(tc1)):
            tc2 = list(tc1)
            good = True
            while 1:
                if tc2[e] > 0:
                    tc2[e] -= 1
                else:
                    good = False
                    break
                if tuple(tc2) in tuples:
                    break
            if not good:
                continue
            J.extend([j, j])
            I.extend([i, tuples.index(tuple(tc2))])
            m.extend([-1,1])
            j+=1
        i+=1

    I.extend(range(0,len(tuples)))
    J.extend(range(j+1,j+len(tuples)+1))
    m.extend([-1,]*len(tuples))
        
    spm = co.spmatrix(m,I,J,
                      #size=(i,j+1),
                      tc='d')
    print spm.T.size
    return spm.T

def objective_factory(dim, init, w, P, q):
    return functools.partial(objective, dim, init,  w, P, q)

def objective(dim, init, w, P, q, x = None, z = None):
    if x is None and z is None:
        return (0, init)
    f0 = (x.T * P * x) - (2 * q.T * x) + (init.T * P * init)
    df0 = 2*P*(x-init)
    if z is None:
        #w = co.matrix(weights, tc='d')
        #P = co.spmatrix(weights, range(len(weights)), range(len(weights)))
        #q = co.matrix(-2 * w.T * init, tc='d')
        return (f0, df0.T)
    else:
        d2f0 = 2 * P
        return (f0, df0.T, z[0,0] * d2f0)

def in_cvx_hull(point, A):
    #print point
    n = A.size[1]
    #A[0, idx] = 1
    b = co.matrix([1,] + list(point), tc='d')
    G = co.spmatrix(-1, range(n), range(n))
    h = co.matrix([0,]*n, tc='d')
    res = co.solvers.lp(h, G, h, A, b)
    if res['status'] != 'primal infeasible':
        return True
    else: #if res['status'] == 'primal infeasible':
        return False
    #else:
    #    raise Exception('Problem with CVX LP: %s'%res['status'])
    

def smooth_ratios(support=supports):
    tuple_lists = []
    ratio_dicts = []
    for s in support:
        print "*"*50 + s
        tuples = prof_tuples(prof_maxes[s])
        weights, r_bar = raw_ratios(s)

        if 0: #len(prof_maxes[s]) > 1:
            wdict = dict(zip(tuples, weights))
            rdict = dict(zip(tuples, r_bar))
            obs = [tuples[i] for i in range(len(tuples)) if r_bar[i] > 0]
            print obs
            pre_R = flatten(obs)
            #obs = [tuples[i] for i in range(len(tuples)) if r_bar[i] > 0]
            #print "rdict: " + str(rdict[(5,5,4,2)])
            pre_R_vec  = ro.RVector(pre_R)
            ro.r['assign']("obs", pre_R_vec)
            obs_mat = ro.r("obs_mat <- matrix(as.numeric(obs), nc=%d, byrow=T)"%len(prof_maxes[s]))
            ro.r("library(geometry)")
            hull_idx = [int(k) for k in ro.r("print(unique(as.vector(convhulln(obs_mat, \"QJ\"))))")]
            hull = [obs[i-1] for i in hull_idx]
            print hull
            
            constr = [[1,]+list(i) for i in hull]
            print constr

            constr_mat = co.matrix(constr, tc='d')
            print constr_mat
            #in_hull = filter(lambda t: in_cvx_hull(t, constr_mat), tuples)
            #out_hull = list(set(tuples).difference(in_hull))
            #print "Difference: " + str(len(in_hull)-len(tuples))
            #in_hull = obs
            in_hull=tuples
            in_weights = [wdict[t] for t in in_hull]
            in_rbar = [rdict[t] for t in in_hull]
            print set(tuples).difference(in_hull)
            #print in_cvx_hull((5,5,4,2), obs_mat)

            
        else:
            in_weights = weights
            in_rbar = r_bar
            in_hull = tuples

        init = co.matrix(in_rbar, tc='d')
        w = co.matrix(in_weights, tc='d')
        P = co.spmatrix(w, range(len(in_weights)), range(len(in_weights)), tc='d')
        q = co.matrix(-P * init, tc='d')

        #objfun = objective_factory(len(in_hull), init, w, P, q)
        #print objfun.args
        if s == "dist":
            r = r_bar
            t = in_hull 
            #print q
        G = G_mat(in_hull)
        f = open("G_file.p", 'w')
        pickle.dump(G, f)
        f.close()
        #print G
        h = co.matrix((0,)*G.size[0], tc='d')
        sol = co.solvers.coneqp(P, q, G, h, dims={'l':G.size[0],'q':[],'s':[]})
        #sol = co.solvers.cp(objfun, G, h)
        res = dict(zip(in_hull, sol['x']))
        #print [(k,v) for k,v in rdict.iteritems() if v == max(rdict.values())]
        #print [(k,v) for k,v in res.iteritems() if v == max(res.values())]
        #print [(k,res[k],rdict[k]) for k in [in_hull[x] for x in investigate(0, G)]] 
        #print [(k,res[k],rdict[k]) for k in [in_hull[x] for x in investigate(1, G)]]
        #print [(k,res[k],rdict[k]) for k in [in_hull[x] for x in investigate(11, G)]]
        #print [(k,res[k],rdict[k]) for k in [in_hull[x] for x in investigate(19, G)]]
        #print [(k,res[k],rdict[k]) for k in [in_hull[x] for x in investigate(66, G)]]
        #print [(k,res[k],rdict[k]) for k in [in_hull[x] for x in investigate(143, G)]]
        tuple_lists.append(tuples)
        ratio_dicts.append(dict(zip(in_hull, list(sol['x']))))

#    print w
    #print r
#    r2 = [2929666.7390237185, 0.70787580866602073, 0.1314312027126355, 0.015006581860636961, 0.005, 0.0045703943247474149]
    #print r2
#    print ratio_dicts[0]
    #d = [(tup, ratio_dicts[0][tup]) for tup in t]
    #print d

    full_tuples = [x for x in product(*tuple_lists)]
    return dict([(flatten(t),
                    reduce(operator.mul, [ratio_dicts[i][t[i]] for i in range(len(support))], 1))
                    for t in full_tuples])

#def dominates(orig, G):
#    a = numpy.array(co.matrix(G))
#    res = [orig]
#    for r in a:
#        if r[orig] == -1:
#            i = numpy.where(r==1)
#            if i[0].size:
#                res.append(i[0][0])
#    print res
#    return res

def dominates(sp1, sp2):
    return all(map(lambda x,y: x >= y, sp1, sp2))

def round_to_hull(point, hull):
    done = False
    hull = filter(lambda x: dominates(point, x), hull)
    current = hull.pop(0)
    done = False
    while not done:
        print hull
        i = 0
        popped = 0
        iters = len(hull)
        for i in range(iters):
            if dominates(current, hull[i-popped]):
                #print str(current) + " dominates " + str(hull[i-popped])
                hull.pop(i-popped)
                popped += 1
            elif dominates(hull[i-popped], current):
                #print str(current) + " dominated by " + str(hull[i-popped])
                #leftovers.append(current)
                current = hull.pop(i-popped) 
                popped += 1
                #print "switch"
                break
        if iters ==0 or i == iters-1:
            done = True
    hull.append(current)
    return hull 

if __name__=="__main__":
    #print raw_ratios("fname")
    ratio_dict = smooth_ratios()
    #print ratio_dict
    f = open("ratios.p", 'w')
    pickle.dump(ratio_dict, f)
    f.close()
    dd.writeRatioDB(ratio_dict)
