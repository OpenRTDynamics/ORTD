exec('libdyn.sci');
z=poly(0,'z');

sim = libdyn_new_simulation();
defaultevents = [0];

[sim,sum1] = libdyn_new_blk_sum(sim, defaultevents, 1.2, -0.1);

H = (3*z^2 + 4*z+ 9) / (2*z^3 + 7*z^2 + 8*z + 4);
H = (1) / (z-0.5);
I = 0.01*(z+1)/(z-1);
[sim,tf_H] = libdyn_new_blk_zTF(sim, defaultevents, H);
[sim,tf_H2] = libdyn_new_blk_zTF(sim, defaultevents, H);
[sim,tf_I] = libdyn_new_blk_zTF(sim, defaultevents, I);
[sim,sum2] = libdyn_new_blk_sum(sim, defaultevents, 2, 3);
[sim,const1] = libdyn_new_blk_const(sim, defaultevents, 3.5);
[sim,gain1] = libdyn_new_blk_gain(sim, defaultevents, 3);

//sim = libdyn_connect_extern_in(sim, 0, sum1, 0);
//sim = libdyn_connect_block(sim, sum1, 0, sum2, 0);
//sim = libdyn_connect_block(sim, sum2, 0, tf_H, 0);
//sim = libdyn_connect_block(sim, tf_H, 0, sum1, 1);
//sim = libdyn_connect_block(sim, tf_H, 0, sum2, 1);
//sim = libdyn_connect_extern_ou(sim, sum2, 0, 0);


//sim = libdyn_connect_extern_in(sim, 0, tf_H, 0);
//sim = libdyn_connect_block(sim, tf_H, 0, tf_H2, 0);
//sim = libdyn_connect_extern_ou(sim, tf_H2, 0, 0);


sim = libdyn_connect_extern_in(sim, 0, tf_H, 0);

sim = libdyn_connect_block(sim, tf_H, 0, sum1, 0);
sim = libdyn_connect_block(sim, sum1, 0, tf_H2, 0);
sim = libdyn_connect_block(sim, tf_H2, 0, sum1, 1);
sim = libdyn_connect_block(sim, tf_H2, 0, tf_I, 0);

sim = libdyn_connect_block(sim, tf_H2, 0, gain1, 0);

//sim = libdyn_connect_extern_ou(sim, tf_I, 0, 0);
sim = libdyn_connect_extern_ou(sim, const1, 0, 0);
sim = libdyn_connect_extern_ou(sim, tf_H, 0, 1);



sim = libdyn_build_cl(sim);
irpar = combine_irparam(sim.parlist);

//clean cfg;
cfg.ipar = irpar.ipar;
cfg.rpar = irpar.rpar;
cfg.Nin = 1;
cfg.Nout = 1;
