function [x,y,typ] = generic_libdyn(job,arg1,arg2)

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
      [ok,Nin,Nout,gipar,grpar,events,exprs]=..
      getvalue('Generic libdyn block DF',..
         ['Nin:';
         'Nout:';
         'ipar:';
         'rpar:';
          'events:'],..
      list('vec',-1,'vec',-1,'vec',-1,'vec',-1,'vec',-1),exprs)
     if ~ok then break,end
      in=[[Nin]]
      out=[[Nout]]
      evtin=[[events]]
      evtout=[]
      [model,graphics,ok]=check_io(model,graphics,in,out,evtin,evtout);
      if ok then
        graphics.exprs=exprs;
        model.ipar=[gipar(:)
];
        model.rpar=[grpar(:)
];
	  model.dstate=[1];
        x.graphics=graphics;x.model=model
        break
      end
    end
  case 'define' then
     Nin=3;
     Nout=2;
     gipar=[];
     grpar=[];
     events=[1;1];
   model=scicos_model()
   model.sim=list('rt_generic_libdyn',4)
   model.in=[[Nin]]
   model.out=[[Nout]]
   model.evtin=[[events]]
   model.evtout=[]
   model.ipar=[gipar(:)
];
   model.rpar=[grpar(:)
];
	model.dstate=[1];
	model.blocktype='d';
	model.dep_ut=[%t %f];
    exprs=[sci2exp(Nin),sci2exp(Nout),sci2exp(gipar),sci2exp(grpar),sci2exp(events)]
    gr_i=['xstringb(orig(1),orig(2),[''GENERIC LIBDYN DF''    ],sz(1),sz(2),''fill'');'];
    x=standard_define([5 2],model,exprs,gr_i)
case 'readout' then
      BLOCK.version=020;
      BLOCK.name='generic_libdyn';
      BLOCK.comp_name='rt_generic_libdyn';
      BLOCK.desr_short='Generic libdyn block DF';
      BLOCK.dep_u=%t;
      BLOCK.dep_t=%f;
      BLOCK.blocktype='d';
      BLOCK.dstate='';
      BLOCK.IOmatrix=%f;
      BLOCK.inset=%t;
      BLOCK.in='[Nin]';
      BLOCK.outset=%t;
      BLOCK.out='[Nout]';
      BLOCK.evtin='[events]';
      BLOCK.evtout='';
      BLOCK.size='5 2';
      BLOCK.completelabel=%f;
      BLOCK.label=[39,39,71,69,78,69,82,73,67,32,76,73,66,68,89,78,32,68,70,39,39,10,10,10,10];
      BLOCK.ipar=[103,105,112,97,114,40,58,41,10];
      BLOCK.rpar=[103,114,112,97,114,40,58,41,10];
      BLOCK.opar=[];
      BLOCK.parameter=list();
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(1).name='Nin';
      BLOCK.parameter(1).text='Nin:';
      BLOCK.parameter(1).type='vec';
      BLOCK.parameter(1).size='-1';
      BLOCK.parameter(1).init='3';
      BLOCK.parameter(1).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(2).name='Nout';
      BLOCK.parameter(2).text='Nout:';
      BLOCK.parameter(2).type='vec';
      BLOCK.parameter(2).size='-1';
      BLOCK.parameter(2).init='2';
      BLOCK.parameter(2).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(3).name='gipar';
      BLOCK.parameter(3).text='ipar:';
      BLOCK.parameter(3).type='vec';
      BLOCK.parameter(3).size='-1';
      BLOCK.parameter(3).init='';
      BLOCK.parameter(3).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(4).name='grpar';
      BLOCK.parameter(4).text='rpar:';
      BLOCK.parameter(4).type='vec';
      BLOCK.parameter(4).size='-1';
      BLOCK.parameter(4).init='';
      BLOCK.parameter(4).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(5).name='events';
      BLOCK.parameter(5).text='events:';
      BLOCK.parameter(5).type='vec';
      BLOCK.parameter(5).size='-1';
      BLOCK.parameter(5).init='[1;1]';
      BLOCK.parameter(5).visible_plot=%f;
      x=BLOCK;
  end
endfunction
