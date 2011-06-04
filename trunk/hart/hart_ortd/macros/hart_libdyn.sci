function [x,y,typ] = hart_libdyn(job,arg1,arg2)

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
  try
  getversion('scilab');
      [ok,insizes,outsizes,usemaster,mastertcp,fname,nEvents,shematic_id,exprs]=..
      scicos_getvalue('ORTD Scicos Interface Block (libdyn)',..
         ['Insizes:';
         'Outsizes:';
         'use master:';
         'master tcp port:';
         'filename:';
         'num events:';
          'shematic id:'],..
      list('vec',-1,'vec',-1,'vec',1,'vec',1,'str',-1,'vec',1,'vec',1),exprs)
catch
      [ok,insizes,outsizes,usemaster,mastertcp,fname,nEvents,shematic_id,exprs]=..
      getvalue('ORTD Scicos Interface Block (libdyn)',..
         ['Insizes:';
         'Outsizes:';
         'use master:';
         'master tcp port:';
         'filename:';
         'num events:';
          'shematic id:'],..
      list('vec',-1,'vec',-1,'vec',1,'vec',1,'str',-1,'vec',1,'vec',1),exprs)
end;
     if ~ok then break,end
      in=[insizes]
      out=[outsizes]
      evtin=[ones(nEvents,1)]
      evtout=[]
      [model,graphics,ok]=check_io(model,graphics,in,out,evtin,evtout);
      if ok then
        graphics.exprs=exprs;
        model.ipar=[1;
usemaster;
mastertcp;
nEvents;
shematic_id;
0;0;0;0;0;
length(ascii(fname));
ascii(fname)'




];
        model.rpar=[];
   model.dstate=[0];
        x.graphics=graphics;x.model=model
        break
      end
    end
  case 'define' then
     insizes=[1;1];
     outsizes=[1];
     usemaster=0;
     mastertcp=12345;
     fname='irpar_file';
     nEvents=1;
     shematic_id=900;
   model=scicos_model()
   model.sim=list('rt_hart_libdyn',4)
   model.in=[insizes]
   model.out=[outsizes]
   model.evtin=[ones(nEvents,1)]
   model.evtout=[]
   model.ipar=[1;
usemaster;
mastertcp;
nEvents;
shematic_id;
0;0;0;0;0;
length(ascii(fname));
ascii(fname)'




];
   model.rpar=[];
 model.dstate=[0];
 model.blocktype='d';
 model.dep_ut=[%t %f];
    exprs=[sci2exp(insizes);sci2exp(outsizes);sci2exp(usemaster);sci2exp(mastertcp);fname;sci2exp(nEvents);sci2exp(shematic_id)]
    gr_i=['xstringb(orig(1),orig(2),[''generic libdyn'' ],sz(1),sz(2),''fill'');'];
    x=standard_define([5 2],model,exprs,gr_i)
case 'readout' then
      BLOCK.version=020;
      BLOCK.name='hart_libdyn';
      BLOCK.comp_name='rt_hart_libdyn';
      BLOCK.desr_short='ORTD Scicos Interface Block (libdyn)';
      BLOCK.dep_u=%t;
      BLOCK.dep_t=%f;
      BLOCK.blocktype='d';
      BLOCK.dstate='0';
      BLOCK.IOmatrix=%f;
      BLOCK.inset=%t;
      BLOCK.in='insizes';
      BLOCK.outset=%t;
      BLOCK.out='outsizes';
      BLOCK.evtin='ones(nEvents,1)';
      BLOCK.evtout='';
      BLOCK.size='5 2';
      BLOCK.completelabel=%f;
      BLOCK.label=[39,39,103,101,110,101,114,105,99,32,108,105,98,100,121,110,39,39,10];
      BLOCK.ipar=[49,59,10,117,115,101,109,97,115,116,101,114,59,10,109,97,115,116,101,114,116,99,112,59,..
         10,110,69,118,101,110,116,115,59,10,115,104,101,109,97,116,105,99,95,105,100,59,10,48,59,..
         48,59,48,59,48,59,48,59,10,108,101,110,103,116,104,40,97,115,99,105,105,40,102,110,97,..
         109,101,41,41,59,10,97,115,99,105,105,40,102,110,97,109,101,41,39,10,10,10,10,10];
      BLOCK.rpar=[];
      BLOCK.opar=[];
      BLOCK.parameter=list();
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(1).name='insizes';
      BLOCK.parameter(1).text='Insizes:';
      BLOCK.parameter(1).type='vec';
      BLOCK.parameter(1).size='-1';
      BLOCK.parameter(1).init='[1;1]';
      BLOCK.parameter(1).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(2).name='outsizes';
      BLOCK.parameter(2).text='Outsizes:';
      BLOCK.parameter(2).type='vec';
      BLOCK.parameter(2).size='-1';
      BLOCK.parameter(2).init='[1]';
      BLOCK.parameter(2).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(3).name='usemaster';
      BLOCK.parameter(3).text='use master:';
      BLOCK.parameter(3).type='vec';
      BLOCK.parameter(3).size='1';
      BLOCK.parameter(3).init='0';
      BLOCK.parameter(3).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(4).name='mastertcp';
      BLOCK.parameter(4).text='master tcp port:';
      BLOCK.parameter(4).type='vec';
      BLOCK.parameter(4).size='1';
      BLOCK.parameter(4).init='12345';
      BLOCK.parameter(4).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(5).name='fname';
      BLOCK.parameter(5).text='filename:';
      BLOCK.parameter(5).type='str';
      BLOCK.parameter(5).size='-1';
      BLOCK.parameter(5).init='irpar_file';
      BLOCK.parameter(5).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(6).name='nEvents';
      BLOCK.parameter(6).text='num events:';
      BLOCK.parameter(6).type='vec';
      BLOCK.parameter(6).size='1';
      BLOCK.parameter(6).init='1';
      BLOCK.parameter(6).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(7).name='shematic_id';
      BLOCK.parameter(7).text='shematic id:';
      BLOCK.parameter(7).type='vec';
      BLOCK.parameter(7).size='1';
      BLOCK.parameter(7).init='900';
      BLOCK.parameter(7).visible_plot=%f;
      x=BLOCK;
  end
endfunction
