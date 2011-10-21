exec('irpar.sci');

parlist = new_irparam_set();
//parlist = new_irparam_elemet(parlist, 200, 1, [3], [1,2,3]);
//parlist = new_irparam_elemet(parlist, 210, 1, [2], [11,12]);
parlist = new_irparam_elemet_rvec(parlist, [90,100,110], 220); // new vektor connected to id=220

// A transfer function
z=poly(0,'z');
G = (5*z^2+3*z+9)/(2*z^3 + 7*z);
parlist = new_irparam_elemet_tf(parlist, G, 230);

// A matrix
A = [0.1,0.2; 0.3,0.4];
//parlist = new_irparam_elemet_rmat(parlist, A, 240);

blockparam2 = combine_irparam(parlist);

save_irparam(blockparam2, 'demo.ipar', 'demo.rpar');

