function [x,y,typ] = ilc2(job,arg1,arg2)

  x=[];y=[];typ=[];
  select job
  case 'plot' then
    exprs=arg1.graphics.exprs;
    standard_draw(arg1)
  case 'getinputs' then
    [x,y,typ]=standard_inputs(arg1)
  case 'getoutputs' then
    [x,y,typ]=standard_outputs(arg1)
  case 'getorigin' then
    [x,y]=standard_origin(arg1)
  case 'set' then
    x=arg1
    model=arg1.model;graphics=arg1.graphics;
    exprs=graphics.exprs;
    while %t do
      [ok,u0,L,Q,Nch,hold_type,N_samples,filtfilt,exprs]=..
      getvalue('Iterative learning control',..
         ['initial u :';
         'impulse Matrix L :';
         'impulse Matrix Q :';
         'Nr channels :';
         'output after sampling: zero 0; hold last value 1  :';
         'Nr samples';
          'filtfiltmode: on 1; off 0 :'],..
      list('row',-1,'row',-1,'row',-1,'vec',1,'vec',1,'vec',1,'vec',1),exprs)
     if ~ok then break,end
      in=[[Nch; ones(Nch,1)*N_samples]]
      out=[[Nch;Nch; ones(Nch,1) * N_samples * 2]]
      evtin=[[1;1]]
      evtout=[]
      [model,graphics,ok]=check_io(model,graphics,in,out,evtin,evtout);
      if ok then
        graphics.exprs=exprs;
        model.ipar=[length(u0);
length(L(:));
length(Q(:));
Nch;
hold_type;
N_samples;
filtfilt
];
        model.rpar=[u0(:);
L(:);
Q(:)
];
	  model.dstate=[];
        x.graphics=graphics;x.model=model
        break
      end
    end
  case 'define' then
     u0=[0,0,0, 0,0,0];
     L=[0];
     Q=[0];
     Nch=2;
     hold_type=0;
     N_samples=6;
     filtfilt=1;
   model=scicos_model()
   model.sim=list('rt_ilc',4)
   model.in=[[Nch; ones(Nch,1)*N_samples]]
   model.out=[[Nch;Nch; ones(Nch,1) * N_samples * 2]]
   model.evtin=[[1;1]]
   model.evtout=[]
   model.ipar=[length(u0);
length(L(:));
length(Q(:));
Nch;
hold_type;
N_samples;
filtfilt
];
   model.rpar=[u0(:);
L(:);
Q(:)
];
	model.dstate=[];
	model.blocktype='d';
	model.dep_ut=[%f %f];
    exprs=[sci2exp(u0),sci2exp(L),sci2exp(Q),sci2exp(Nch),sci2exp(hold_type),sci2exp(N_samples),sci2exp(filtfilt)]
    gr_i=['xstringb(orig(1),orig(2),[''MIMO SQ ILC'' ],sz(1),sz(2),''fill'');'];
    x=standard_define([3 2],model,exprs,gr_i)
case 'readout' then
      BLOCK.version=020;
      BLOCK.name='ilc2';
      BLOCK.comp_name='rt_ilc';
      BLOCK.desr_short='Iterative learning control';
      BLOCK.dep_u=%f;
      BLOCK.dep_t=%f;
      BLOCK.blocktype='d';
      BLOCK.dstate='';
      BLOCK.IOmatrix=%f;
      BLOCK.inset=%t;
      BLOCK.in='[Nch; ones(Nch,1)*N_samples]';
      BLOCK.outset=%t;
      BLOCK.out='[Nch;Nch; ones(Nch,1) * N_samples * 2]';
      BLOCK.evtin='[1;1]';
      BLOCK.evtout='';
      BLOCK.size='3 2';
      BLOCK.completelabel=%f;
      BLOCK.label=[39,39,77,73,77,79,32,83,81,32,73,76,67,39,39,10];
      BLOCK.ipar=[108,101,110,103,116,104,40,117,48,41,59,10,108,101,110,103,116,104,40,76,40,58,41,41,59,..
         10,108,101,110,103,116,104,40,81,40,58,41,41,59,10,78,99,104,59,10,104,111,108,100,95,..
         116,121,112,101,59,10,78,95,115,97,109,112,108,101,115,59,10,102,105,108,116,102,105,108,..
         116,10];
      BLOCK.rpar=[117,48,40,58,41,59,10,76,40,58,41,59,10,81,40,58,41,10];
      BLOCK.opar=[];
      BLOCK.parameter=list();
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(1).name='u0';
      BLOCK.parameter(1).text='initial u :';
      BLOCK.parameter(1).type='row';
      BLOCK.parameter(1).size='-1';
      BLOCK.parameter(1).init='[0,0,0, 0,0,0]';
      BLOCK.parameter(1).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(2).name='L';
      BLOCK.parameter(2).text='impulse Matrix L :';
      BLOCK.parameter(2).type='row';
      BLOCK.parameter(2).size='-1';
      BLOCK.parameter(2).init='[0]';
      BLOCK.parameter(2).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(3).name='Q';
      BLOCK.parameter(3).text='impulse Matrix Q :';
      BLOCK.parameter(3).type='row';
      BLOCK.parameter(3).size='-1';
      BLOCK.parameter(3).init='[0]';
      BLOCK.parameter(3).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(4).name='Nch';
      BLOCK.parameter(4).text='Nr channels :';
      BLOCK.parameter(4).type='vec';
      BLOCK.parameter(4).size='1';
      BLOCK.parameter(4).init='2';
      BLOCK.parameter(4).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(5).name='hold_type';
      BLOCK.parameter(5).text='output after sampling: zero 0; hold last value 1  :';
      BLOCK.parameter(5).type='vec';
      BLOCK.parameter(5).size='1';
      BLOCK.parameter(5).init='0';
      BLOCK.parameter(5).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(6).name='N_samples';
      BLOCK.parameter(6).text='Nr samples';
      BLOCK.parameter(6).type='vec';
      BLOCK.parameter(6).size='1';
      BLOCK.parameter(6).init='6';
      BLOCK.parameter(6).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(7).name='filtfilt';
      BLOCK.parameter(7).text='filtfiltmode: on 1; off 0 :';
      BLOCK.parameter(7).type='vec';
      BLOCK.parameter(7).size='1';
      BLOCK.parameter(7).init='1';
      BLOCK.parameter(7).visible_plot=%f;
      x=BLOCK;
  end
endfunction
