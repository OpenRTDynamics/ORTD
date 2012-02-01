  expr = 'sin(u1+1)*c2+u2';
  float_param=[3.1, 21];
  [sim,out] = ld_muparser(sim, defaultevents, list(u,x), expr, float_param);

  [sim] = ld_printf(sim, defaultevents, out, "muout = ", 1);

  
  
  parNames = ['par1', 'par2']; par = [ 0.2, 0.4 ];
  inNames = ['x', 'y' ]; in = list(x,y);
  expr = 'sin(x)*par1 + par2*y';
  [sim,mytest] = ld_muparser_subst(sim, defaultevents, in, expr, par, inNames, parNames);
  
  [sim] = ld_printf(sim, defaultevents, mytest, "mytest__ = ", 1);

